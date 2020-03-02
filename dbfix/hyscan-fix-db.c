/* hyscan-fix-db.c
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
 * SECTION: hyscan-fix-db
 * @Short_description: класс обновления базы данных HyScan
 * @Title: HyScanFixDB
 *
 * Класс предназначен для обновления формата проектов и галсов до текущего
 * используемого в HyScan. Обновление производится в фоновом режиме, во
 * время которого пользователю доступна информация о прогрессе обновления и
 * текущем выполняемом шаге.
 *
 * Запуск обновления производится с помощью функции #hyscan_fix_db_upgrade.
 * После получения сигнала о завершении обновления, необходимо вызвать функцию
 * #hyscan_fix_db_complete.
 */

#include <glib/gi18n.h>

#include "hyscan-fix-db.h"
#include "hyscan-fix-common.h"
#include "hyscan-fix-project.h"
#include "hyscan-fix-track.h"

#include <hyscan-db.h>

enum
{
  SIGNAL_LOG,
  SIGNAL_COMPLETED,
  SIGNAL_LAST
};

typedef struct _HyScanFixDBArgs
{
} HyScanFixDBArgs;

struct _HyScanFixDBPrivate
{
  GMutex               lock;
  GThread             *upgrader;
  gchar               *db_path;
  HyScanCancellable   *cancellable;
  gboolean             status;
};

static void            hyscan_fix_db_object_constructed      (GObject            *object);
static void            hyscan_fix_db_object_finalize         (GObject            *object);

static gboolean        hyscan_fix_db_project_upgrade         (HyScanFixDB        *fix,
                                                              const gchar        *project_name);

static gpointer        hyscan_fix_db_upgrader                (gpointer            data);

static guint           hyscan_fix_db_signals[SIGNAL_LAST] = { 0 };

G_DEFINE_TYPE_WITH_PRIVATE (HyScanFixDB, hyscan_fix_db, G_TYPE_OBJECT)

static void
hyscan_fix_db_class_init (HyScanFixDBClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = hyscan_fix_db_object_constructed;
  object_class->finalize = hyscan_fix_db_object_finalize;

  /**
   * HyScanFixDB::log:
   * @fix: указатель на #HyScanFixDB
   * @message: информационное сообщение
   *
   * Сигнал посылается для информирование польщователя о
   * текущем выполняемом действии.
   */
  hyscan_fix_db_signals[SIGNAL_LOG] =
      g_signal_new ("log", HYSCAN_TYPE_FIX_DB, G_SIGNAL_RUN_LAST, 0,
                    NULL, NULL,
                    g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);

  /**
   * HyScanFixDB::completed:
   * @fix: указатель на #HyScanFixDB
   * @status: статус обновления
   *
   * Сигнал посылается при завершении обновления.
   */
  hyscan_fix_db_signals[SIGNAL_COMPLETED] =
      g_signal_new ("completed", HYSCAN_TYPE_FIX_DB, G_SIGNAL_RUN_LAST, 0,
                    NULL, NULL,
                    g_cclosure_marshal_VOID__BOOLEAN, G_TYPE_NONE, 1, G_TYPE_BOOLEAN);
}

static void
hyscan_fix_db_init (HyScanFixDB *fix)
{
  fix->priv = hyscan_fix_db_get_instance_private (fix);
}

static void
hyscan_fix_db_object_constructed (GObject *object)
{
  HyScanFixDB *fix = HYSCAN_FIX_DB (object);
  HyScanFixDBPrivate *priv = fix->priv;

  g_mutex_init (&priv->lock);
}

static void
hyscan_fix_db_object_finalize (GObject *object)
{
  HyScanFixDB *fix = HYSCAN_FIX_DB (object);
  HyScanFixDBPrivate *priv = fix->priv;

  hyscan_fix_db_complete (fix);
  g_mutex_clear (&priv->lock);

  G_OBJECT_CLASS (hyscan_fix_db_parent_class)->finalize (object);
}

/* Функция обновляет проект и все галсы в нём. */
static gboolean
hyscan_fix_db_project_upgrade (HyScanFixDB *fix,
                               const gchar *project_name)
{
  HyScanFixDBPrivate *priv = fix->priv;
  gboolean status = TRUE;
  gchar *log_message;
  gchar *project_path;
  gchar **tracks;
  guint n_tracks;
  gint version;
  guint i;

  version = hyscan_fix_project_get_version (priv->db_path, project_name);
  if (version == HYSCAN_FIX_PROJECT_NOT_PROJECT)
    return TRUE;

  log_message = g_strdup_printf (_("Update project %s"), project_name);
  g_signal_emit (fix, hyscan_fix_db_signals[SIGNAL_LOG], 0, log_message);
  g_free (log_message);

  project_path = g_build_filename (priv->db_path, project_name, NULL);
  tracks = hyscan_fix_dir_list (project_path);
  g_free (project_path);

  if (tracks == NULL)
    return FALSE;

  n_tracks = g_strv_length (tracks);
  hyscan_cancellable_push (priv->cancellable);

  for (i = 0; i < n_tracks; i++)
    {
      gchar *track_path;

      hyscan_cancellable_set_total (priv->cancellable, i, 0, n_tracks);

      if (g_cancellable_is_cancelled (G_CANCELLABLE (priv->cancellable)))
        break;

      track_path = g_build_filename (project_name, tracks[i], NULL);
      version = hyscan_fix_track_get_version (priv->db_path, track_path);
      if ((version != HYSCAN_FIX_TRACK_NOT_TRACK) &&
          (version != HYSCAN_FIX_TRACK_LATEST))
        {
          log_message = g_strdup_printf (_("Update track %s.%s"), project_name, tracks[i]);
          g_signal_emit (fix, hyscan_fix_db_signals[SIGNAL_LOG], 0, log_message);
          g_free (log_message);

          status = hyscan_fix_track (priv->db_path, track_path, priv->cancellable);
        }
      g_free (track_path);

      if (!status)
        {
          log_message = g_strdup_printf (_("Failed to update %s.%s"), project_name, tracks[i]);
          g_signal_emit (fix, hyscan_fix_db_signals[SIGNAL_LOG], 0, log_message);
          g_free (log_message);

          break;
        }
    }

  if (status)
    {
      log_message = g_strdup_printf (_("Update parameters %s"), project_name);
      g_signal_emit (fix, hyscan_fix_db_signals[SIGNAL_LOG], 0, log_message);
      g_free (log_message);

      status = hyscan_fix_project (priv->db_path, project_name);
      if (!status)
        {
          log_message = g_strdup_printf (_("Failed to update parameters %s"), project_name);
          g_signal_emit (fix, hyscan_fix_db_signals[SIGNAL_LOG], 0, log_message);
          g_free (log_message);
        }
    }

  hyscan_cancellable_pop (priv->cancellable);
  g_strfreev (tracks);

  return status;
}

/* Поток обновления базы данных. */
static gpointer
hyscan_fix_db_upgrader (gpointer data)
{
  HyScanFixDB *fix  = data;
  HyScanFixDBPrivate *priv = fix->priv;
  gboolean status = FALSE;

  HyScanDB *db_lock;
  gchar *db_uri;

  gchar **projects;
  guint n_projects;
  guint i;

  db_uri = g_strdup_printf ("file://%s", priv->db_path);
  db_lock = hyscan_db_new (db_uri);
  g_free (db_uri);

  if (db_lock == NULL)
    goto exit;

  projects = hyscan_fix_dir_list (priv->db_path);
  if (projects == NULL)
    goto exit;

  status = TRUE;
  n_projects = g_strv_length (projects);
  hyscan_cancellable_push (priv->cancellable);

  for (i = 0; i < n_projects; i++)
    {
      hyscan_cancellable_set_total (priv->cancellable, i, 0, n_projects);

      if (g_cancellable_is_cancelled (G_CANCELLABLE (priv->cancellable)))
        break;

      status = hyscan_fix_db_project_upgrade (fix, projects[i]);
      if (!status)
        break;
    }

  hyscan_cancellable_pop (priv->cancellable);
  g_strfreev (projects);

exit:
  g_clear_object (&db_lock);
  g_clear_object (&priv->cancellable);
  g_clear_pointer (&priv->db_path, g_free);

  priv->status = status;

  g_signal_emit (fix, hyscan_fix_db_signals[SIGNAL_COMPLETED], 0, status);

  return NULL;
}

/**
 * hyscan_fix_db_new:
 *
 * Функция создаёт новый объект #HyScanFixDB.
 *
 * Returns: #HyScanFixDB. Для удаления #g_object_unref.
 */
HyScanFixDB *
hyscan_fix_db_new (void)
{
  return g_object_new (HYSCAN_TYPE_FIX_DB, NULL);
}

/**
 * hyscan_fix_db_upgrade:
 * @fix: указатель на #HyScanFixDB
 * @db_path: путь к базе данных
 * @cancellable: указатель на #HyScanCancellable
 *
 * Функция запускает поток обновления базы данных.
 */
void
hyscan_fix_db_upgrade (HyScanFixDB       *fix,
                       const gchar       *db_path,
                       HyScanCancellable *cancellable)
{
  HyScanFixDBPrivate *priv;

  g_return_if_fail (HYSCAN_IS_FIX_DB (fix));

  priv = fix->priv;

  g_mutex_lock (&priv->lock);

  if (priv->upgrader == NULL)
    {
      priv->db_path = g_strdup (db_path);
      priv->cancellable = g_object_ref (cancellable);
      priv->upgrader = g_thread_new ("db-upgrader", hyscan_fix_db_upgrader, fix);
    }

  g_mutex_unlock (&priv->lock);
}

/**
 * hyscan_fix_db_upgrade:
 * @fix: указатель на #HyScanFixDB
 *
 * Функция ожидает завершения обновления базы данных и возвращает его статус.
 *
 * Returns: %TRUE если обновление успешно завершено, иначе %FALSE.
 */
gboolean
hyscan_fix_db_complete (HyScanFixDB *fix)
{
  HyScanFixDBPrivate *priv;
  gboolean status = FALSE;

  g_return_val_if_fail (HYSCAN_IS_FIX_DB (fix), FALSE);

  priv = fix->priv;

  g_mutex_lock (&priv->lock);
  if (priv->upgrader != NULL)
    {
      g_thread_join (priv->upgrader);
      priv->upgrader = NULL;
      status = priv->status;
    }
  g_mutex_unlock (&priv->lock);

  return status;
}
