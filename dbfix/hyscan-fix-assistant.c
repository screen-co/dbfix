/* hyscan-fix-assistant.c
 *
 * Copyright 2020 Screen LLC, Andrei Fadeev <andrei@webcontrol.ru>
 *
 * This file is part of HyScan DBFix.
 *
 * HyScan DBFix is dual-licensed: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HyScan DBFix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Alternatively, you can license this code under a commercial license.
 * Contact the Screen LLC in this case - <info@screen-co.ru>.
 */

/* HyScan DBFix имеет двойную лицензию.
 *
 * Во-первых, вы можете распространять HyScan DBFix на условиях Стандартной
 * Общественной Лицензии GNU версии 3, либо по любой более поздней версии
 * лицензии (по вашему выбору). Полные положения лицензии GNU приведены в
 * <http://www.gnu.org/licenses/>.
 *
 * Во-вторых, этот программный код можно использовать по коммерческой
 * лицензии. Для этого свяжитесь с ООО Экран - <info@screen-co.ru>.
 */

/**
 * SECTION: hyscan-fix-assistant
 * @Short_description: мастер обновления баз данных HyScan
 * @Title: HyScanFixAssistant
 *
 * Класс представляет собой законченный мастер обновления баз данных HyScan.
 * Класс реализует toplevel #GtkWindow и позволяет построить на базе него
 * полноценное приложение.
 */

#include <glib/gi18n.h>

#include "hyscan-fix-assistant.h"
#include "hyscan-fix-db.h"

#include <hyscan-gtk-profile-db.h>

enum
{
  PROP_O,
  PROP_SYSTEM_PROFILES,
  PROP_USER_PROFILES
};

enum
{
  PAGE_GREETING,
  PAGE_SELECTOR,
  PAGE_CONFIRM,
  PAGE_PROGRESS
};

struct _HyScanFixAssistantPrivate
{
  gchar               *system_profiles;    /* Путь к системным профилям. */
  gchar               *user_profiles;      /* Путь к пользовательским профилям. */
  gchar               *db_path;            /* Каталог с базой данных. */

  HyScanFixDB         *db_fix;             /* Объект обновления базы данных. */
  HyScanCancellable   *db_fix_cancel;      /* Управление обновленим. */
  gboolean             updating;           /* Признак выполнения обновления. */
};

static void            hyscan_fix_assistant_set_property       (GObject               *object,
                                                                guint                  prop_id,
                                                                const GValue          *value,
                                                                GParamSpec            *pspec);
static void            hyscan_fix_assistant_object_constructed (GObject               *object);
static void            hyscan_fix_assistant_object_finalize    (GObject               *object);

static void            hyscan_fix_assistant_decorate_widget    (GtkWidget             *widget,
                                                                gint                   spacing,
                                                                gboolean               expand);

static gint            hyscan_fix_assistant_dialog             (GtkWindow             *parent,
                                                                const gchar           *header,
                                                                const gchar           *message,
                                                                const gchar           *accept,
                                                                const gchar           *reject);

static GtkWidget *     hyscan_fix_assistant_page_welcome       (void);
static GtkWidget *     hyscan_fix_assistant_page_select_db     (HyScanFixAssistant    *fix);
static GtkWidget *     hyscan_fix_assistant_page_confirm       (void);
static GtkWidget *     hyscan_fix_assistant_page_progress      (HyScanFixAssistant    *fix);
static GtkWidget *     hyscan_fix_assistant_page_summary       (HyScanFixAssistant    *fix);
static void            hyscan_fix_assistant_create_ui          (HyScanFixAssistant    *fix);

static void            hyscan_fix_assistant_cancel             (HyScanFixAssistant    *fix);
static void            hyscan_fix_assistant_select_db          (HyScanFixAssistant    *fix,
                                                                HyScanProfileDB       *profile);
static void            hyscan_fix_assistant_start_update       (HyScanFixAssistant    *fix,
                                                                GtkWidget             *cur_page,
                                                                GtkWidget             *progress_page);
static void            hyscan_fix_assistant_fix_log            (HyScanFixAssistant    *fix,
                                                                const gchar           *message);
static void            hyscan_fix_assistant_fix_completed      (HyScanFixAssistant    *fix,
                                                                gboolean               status);
static void            hyscan_fix_assistant_start_again        (HyScanFixAssistant    *fix);

G_DEFINE_TYPE_WITH_PRIVATE (HyScanFixAssistant, hyscan_fix_assistant, GTK_TYPE_ASSISTANT)

static void
hyscan_fix_assistant_class_init (HyScanFixAssistantClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = hyscan_fix_assistant_set_property;

  object_class->constructed = hyscan_fix_assistant_object_constructed;
  object_class->finalize = hyscan_fix_assistant_object_finalize;

  g_object_class_install_property (object_class, PROP_SYSTEM_PROFILES,
    g_param_spec_string ("system-profiles", "SystemProfiles", "Path to system profiles", NULL,
                          G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (object_class, PROP_USER_PROFILES,
    g_param_spec_string ("user-profiles", "UserProfiles", "Path to user profiles", NULL,
                          G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
}

static void
hyscan_fix_assistant_init (HyScanFixAssistant *fix)
{
  fix->priv = hyscan_fix_assistant_get_instance_private (fix);
}

static void
hyscan_fix_assistant_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  HyScanFixAssistant *fix = HYSCAN_FIX_ASSISTANT (object);
  HyScanFixAssistantPrivate *priv = fix->priv;

  switch (prop_id)
    {
    case PROP_SYSTEM_PROFILES:
      priv->system_profiles = g_value_dup_string (value);
      break;

    case PROP_USER_PROFILES:
      priv->user_profiles = g_value_dup_string (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
hyscan_fix_assistant_object_constructed (GObject *object)
{
  HyScanFixAssistant *fix = HYSCAN_FIX_ASSISTANT (object);
  HyScanFixAssistantPrivate *priv = fix->priv;

  G_OBJECT_CLASS (hyscan_fix_assistant_parent_class)->constructed (object);

  hyscan_fix_assistant_create_ui (fix);

  priv->db_fix = hyscan_fix_db_new ();
  g_signal_connect_swapped (priv->db_fix, "log",
                            G_CALLBACK (hyscan_fix_assistant_fix_log), fix);
  g_signal_connect_swapped (priv->db_fix, "completed",
                            G_CALLBACK (hyscan_fix_assistant_fix_completed), fix);
}

static void
hyscan_fix_assistant_object_finalize (GObject *object)
{
  HyScanFixAssistant *fix = HYSCAN_FIX_ASSISTANT (object);
  HyScanFixAssistantPrivate *priv = fix->priv;

  g_object_unref (priv->db_fix);

  g_free (priv->system_profiles);
  g_free (priv->user_profiles);
  g_free (priv->db_path);

  G_OBJECT_CLASS (hyscan_fix_assistant_parent_class)->finalize (object);
}

/* Функция задаёт выравнивание виджета. */
static void
hyscan_fix_assistant_decorate_widget (GtkWidget *widget,
                                      gint       spacing,
                                      gboolean   expand)
{
  gtk_widget_set_margin_start  (widget, spacing);
  gtk_widget_set_margin_end    (widget, spacing);
  gtk_widget_set_margin_top    (widget, spacing);
  gtk_widget_set_margin_bottom (widget, spacing);
  gtk_widget_set_halign        (widget, expand ? GTK_ALIGN_FILL : GTK_ALIGN_CENTER);
  gtk_widget_set_valign        (widget, expand ? GTK_ALIGN_FILL : GTK_ALIGN_CENTER);
  gtk_widget_set_hexpand       (widget, expand);
  gtk_widget_set_vexpand       (widget, expand);
}

/* Функция запрашивает у пользователя подтверждение действия. */
static gint
hyscan_fix_assistant_dialog (GtkWindow   *parent,
                             const gchar *header,
                             const gchar *message,
                             const gchar *accept,
                             const gchar *reject)
{
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *content;
  GtkDialogFlags flags;
  gint response;

  flags = GTK_DIALOG_MODAL;
  dialog = gtk_dialog_new_with_buttons (header, parent, flags,
                                        accept, GTK_RESPONSE_ACCEPT,
                                        reject, GTK_RESPONSE_REJECT,
                                        NULL);

  label = gtk_label_new (message);
  gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
  hyscan_fix_assistant_decorate_widget (label, 18, TRUE);

  content = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  gtk_container_add (GTK_CONTAINER (content), label);
  gtk_widget_show_all (dialog);

  response = gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);

  return response;
}

/* Страница приветствия. */
static GtkWidget *
hyscan_fix_assistant_page_welcome (void)
{
  GtkWidget *label;

  label = gtk_label_new (_("<b>Welcome to HyScan DB fix program!</b>\n\n"
                           "HyScan DB fix is a database migration tool.\n"
                           "It is used to update HyScan projects to provide\n"
                           "compatibility with latest versions."));
  gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
  hyscan_fix_assistant_decorate_widget (label, 6, TRUE);

  return label;
}

/* Страница выбора базы данных. */
static GtkWidget *
hyscan_fix_assistant_page_select_db (HyScanFixAssistant *fix)
{
  HyScanFixAssistantPrivate *priv = fix->priv;
  gchar *profiles[3];
  GtkWidget *selector;

  profiles[0] = priv->system_profiles;
  if (g_strcmp0 (priv->system_profiles, priv->user_profiles) != 0)
    profiles[1] = priv->user_profiles;
  else
    profiles[1] = NULL;
  profiles[2] = NULL;

  selector = hyscan_gtk_profile_db_new (profiles, TRUE);
  hyscan_fix_assistant_decorate_widget (selector, 6, TRUE);

  g_signal_connect_swapped (selector, "selected", G_CALLBACK (hyscan_fix_assistant_select_db), fix);

  return selector;
}

/* Страница подтверждения выбора. */
static GtkWidget *
hyscan_fix_assistant_page_confirm (void)
{
  GtkWidget *label;

  label = gtk_label_new ("");
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
  hyscan_fix_assistant_decorate_widget (label, 6, TRUE);

  return label;
}

/* Страница процесса обновления. */
static GtkWidget *
hyscan_fix_assistant_page_progress (HyScanFixAssistant *fix)
{
  GtkWidget *grid;
  GtkWidget *label;
  GtkWidget *progress;
  GtkWidget *info;

  label = gtk_label_new ("");
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
  hyscan_fix_assistant_decorate_widget (label, 6, TRUE);

  progress = gtk_progress_bar_new ();
  hyscan_fix_assistant_decorate_widget (progress, 6, TRUE);

  info = gtk_label_new ("");
  gtk_label_set_justify (GTK_LABEL (info), GTK_JUSTIFY_CENTER);
  hyscan_fix_assistant_decorate_widget (info, 6, TRUE);

  grid = gtk_grid_new ();
  gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), progress, 0, 1, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), info, 0, 2, 1, 1);

  return grid;
}

/* Финальная страница. */
static GtkWidget *
hyscan_fix_assistant_page_summary (HyScanFixAssistant *fix)
{
  GtkWidget *grid;
  GtkWidget *label;
  GtkWidget *start;

  label = gtk_label_new (_("Update another DB?"));
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
  hyscan_fix_assistant_decorate_widget (label, 6, FALSE);

  start = gtk_button_new_with_label (_("OK"));
  hyscan_fix_assistant_decorate_widget (start, 6, FALSE);

  grid = gtk_grid_new ();
  gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), start, 0, 1, 1, 1);
  hyscan_fix_assistant_decorate_widget (grid, 6, FALSE);

  g_signal_connect_swapped (start, "clicked",
                            G_CALLBACK (hyscan_fix_assistant_start_again), fix);

  return grid;
}

/* Функция создаёт графический интерфейс пользователя. */
static void
hyscan_fix_assistant_create_ui (HyScanFixAssistant *fix)
{
  GtkAssistant *assistant = GTK_ASSISTANT (fix);
  GtkWidget *welcome;
  GtkWidget *selector;
  GtkWidget *confirm;
  GtkWidget *progress;
  GtkWidget *summary;

  welcome = hyscan_fix_assistant_page_welcome ();
  gtk_assistant_append_page (assistant, welcome);
  gtk_assistant_set_page_type (assistant, welcome, GTK_ASSISTANT_PAGE_INTRO);
  gtk_assistant_set_page_title (assistant, welcome, _("Welcome"));
  gtk_assistant_set_page_complete (assistant, welcome, TRUE);

  selector = hyscan_fix_assistant_page_select_db (fix);
  gtk_assistant_append_page (assistant, selector);
  gtk_assistant_set_page_type (assistant, selector, GTK_ASSISTANT_PAGE_INTRO);
  gtk_assistant_set_page_title (assistant, selector, _("Choose DB"));

  confirm = hyscan_fix_assistant_page_confirm ();
  gtk_assistant_append_page (assistant, confirm);
  gtk_assistant_set_page_type (assistant, confirm, GTK_ASSISTANT_PAGE_CONFIRM);
  gtk_assistant_set_page_title (assistant, confirm, _("Confirm"));

  progress = hyscan_fix_assistant_page_progress (fix);
  gtk_assistant_append_page (assistant, progress);
  gtk_assistant_set_page_type (assistant, progress, GTK_ASSISTANT_PAGE_INTRO);
  gtk_assistant_set_page_title (assistant, progress, _("Update"));

  summary = hyscan_fix_assistant_page_summary (fix);
  gtk_assistant_append_page (assistant, summary);
  gtk_assistant_set_page_type (assistant, summary, GTK_ASSISTANT_PAGE_SUMMARY);
  gtk_assistant_set_page_title (assistant, summary, _("Update"));

  g_signal_connect (fix, "cancel", G_CALLBACK (hyscan_fix_assistant_cancel), NULL);
  g_signal_connect (fix, "prepare", G_CALLBACK (hyscan_fix_assistant_start_update), progress);
  g_signal_connect (fix, "close", G_CALLBACK (gtk_main_quit), NULL);
}

/* Обработчик нажатия кнопки прерывания/завершения обновления. */
static void
hyscan_fix_assistant_cancel (HyScanFixAssistant *fix)
{
  HyScanFixAssistantPrivate *priv = fix->priv;

  if (priv->updating)
    {
      const gchar *header = _("Cancel DB update");
      const gchar *message = _("<b>Do you really want to cancel DB update?</b>\n"
                               "It can render your DB invalid!");
      const gchar *accept = _("Yes");
      const gchar *reject = _("No");
      gint response;

      response = hyscan_fix_assistant_dialog (GTK_WINDOW (fix),
                                              header, message,
                                              accept, reject);

      if (response == GTK_RESPONSE_ACCEPT)
        g_cancellable_cancel (G_CANCELLABLE (priv->db_fix_cancel));
    }
  else
    {
      const gchar *header = _("Quit DB fix");
      const gchar *message = _("<b>Do you really want to quit DB fix?</b>");
      const gchar *accept = _("Yes");
      const gchar *reject = _("No");
      gint response;

      response = hyscan_fix_assistant_dialog (GTK_WINDOW (fix),
                                              header, message,
                                              accept, reject);

      if (response == GTK_RESPONSE_ACCEPT)
        gtk_main_quit ();
    }
}

/* Обработчик сигнала выбора базы данных для обновления. */
static void
hyscan_fix_assistant_select_db (HyScanFixAssistant *fix,
                                HyScanProfileDB    *profile)
{
  HyScanFixAssistantPrivate *priv = fix->priv;
  GtkAssistant *assistant = GTK_ASSISTANT (fix);
  GtkWidget *selector;
  GtkWidget *confirm;
  GtkWidget *progress;
  GtkWidget *label;

  const gchar *db_name;
  const gchar *db_uri;
  gchar *text;

  selector = gtk_assistant_get_nth_page (assistant, PAGE_SELECTOR);
  confirm = gtk_assistant_get_nth_page (assistant, PAGE_CONFIRM);
  progress = gtk_assistant_get_nth_page (assistant, PAGE_PROGRESS);
  label = gtk_grid_get_child_at (GTK_GRID (progress), 0, 0);

  gtk_assistant_set_page_complete (assistant, selector, FALSE);
  gtk_assistant_set_page_complete (assistant, confirm, FALSE);
  g_clear_pointer (&priv->db_path, g_free);
  if (profile == NULL)
    return;

  db_name = hyscan_profile_get_name (HYSCAN_PROFILE (profile));
  db_uri = hyscan_profile_db_get_uri (profile);
  if (!g_str_has_prefix (db_uri, "file://"))
    return;

  priv->db_path = g_strdup (db_uri + strlen ("file://"));

  text = g_strdup_printf (_("HyScan DB fix is ready to update projects\n"
                            "%s - %s"),
                            db_name, priv->db_path);
  gtk_label_set_markup (GTK_LABEL (confirm), text);
  g_free (text);

  text = g_strdup_printf (_("Update %s"), db_name);
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);

  gtk_assistant_set_page_complete (assistant, selector, TRUE);
  gtk_assistant_set_page_complete (assistant, confirm, TRUE);
}

/* Функция запускает обновление базы данных. */
static void
hyscan_fix_assistant_start_update (HyScanFixAssistant *fix,
                                   GtkWidget          *cur_page,
                                   GtkWidget          *progress_page)
{
  HyScanFixAssistantPrivate *priv = fix->priv;

  /* Срабатываем при переключении на страницу обновления. */
  if (cur_page != progress_page)
    return;

  priv->db_fix_cancel = hyscan_cancellable_new ();
  hyscan_fix_db_upgrade (priv->db_fix, priv->db_path, priv->db_fix_cancel);
  priv->updating = TRUE;
}

/* Функция информирует о прогрессе обновления. */
static void
hyscan_fix_assistant_fix_log (HyScanFixAssistant *fix,
                          const gchar        *message)
{
  HyScanFixAssistantPrivate *priv = fix->priv;
  GtkAssistant *assistant = GTK_ASSISTANT (fix);
  GtkWidget *progress;
  GtkWidget *progress_bar;
  GtkWidget *info;

  progress = gtk_assistant_get_nth_page (assistant, PAGE_PROGRESS);
  progress_bar = gtk_grid_get_child_at (GTK_GRID (progress), 0, 1);
  info = gtk_grid_get_child_at (GTK_GRID (progress), 0, 2);

  gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress_bar),
                                 hyscan_cancellable_get (priv->db_fix_cancel));

  gtk_label_set_text (GTK_LABEL (info), message);
}

/* Функция завершает процесс обновления. */
static void
hyscan_fix_assistant_fix_completed (HyScanFixAssistant *fix,
                                    gboolean            status)
{
  HyScanFixAssistantPrivate *priv = fix->priv;
  GtkAssistant *assistant = GTK_ASSISTANT (fix);
  GtkWidget *progress;
  GtkWidget *progress_bar;
  GtkWidget *info;
  gboolean cancelled;

  cancelled = g_cancellable_is_cancelled (G_CANCELLABLE (priv->db_fix_cancel));

  hyscan_fix_db_complete (priv->db_fix);
  g_clear_object (&priv->db_fix_cancel);
  priv->updating = FALSE;

  progress = gtk_assistant_get_nth_page (assistant, PAGE_PROGRESS);
  progress_bar = gtk_grid_get_child_at (GTK_GRID (progress), 0, 1);
  info = gtk_grid_get_child_at (GTK_GRID (progress), 0, 2);

  if (status)
    {
      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress_bar), 1.0);
      gtk_label_set_text (GTK_LABEL (info), cancelled ? _("Cancelled") : _("Completed"));
    }

  gtk_assistant_set_page_complete (assistant, progress, TRUE);
}

/* Функция начинает процесс обновления снова. */
static void
hyscan_fix_assistant_start_again (HyScanFixAssistant *fix)
{
  GtkAssistant *assistant = GTK_ASSISTANT (fix);
  GtkWidget *selector;
  GtkWidget *confirm;
  GtkWidget *progress;

  selector = gtk_assistant_get_nth_page (assistant, PAGE_SELECTOR);
  confirm = gtk_assistant_get_nth_page (assistant, PAGE_CONFIRM);
  progress = gtk_assistant_get_nth_page (assistant, PAGE_PROGRESS);

  gtk_assistant_set_page_complete (assistant, selector, FALSE);
  gtk_assistant_set_page_complete (assistant, confirm, FALSE);
  gtk_assistant_set_page_complete (assistant, progress, FALSE);

  gtk_assistant_set_current_page (assistant, PAGE_SELECTOR);
}

/**
 * hyscan_fix_assistant_new:
 * @system_profiles: путь к системным профилям
 * @user_profiles: путь к пользовательским профилям
 *
 * Функция создаёт новый объект #HyScanFixAssistant.
 *
 * Returns: #HyScanFixAssistant.
 */

GtkWidget *
hyscan_fix_assistant_new (const gchar *system_profiles,
                          const gchar *user_profiles)
{
  return g_object_new (HYSCAN_TYPE_FIX_ASSISTANT,
                       "use-header-bar", TRUE,
                       "system-profiles", system_profiles,
                       "user-profiles", user_profiles,
                       NULL);
}
