/* hyscan-fix-project.c
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

#include "hyscan-fix-project.h"
#include "hyscan-fix-common.h"
#include "hyscan-fix-track.h"

#define PROJECT_FILE_MAGIC     0x52505348      /* HSPR в виде строки. */
#define PROJECT_FILE_VERSION   0x31303731      /* 1701 в виде строки. */

/* Функция возвращает значение контрольной суммы для версии схемы. */
static const gchar *
hyscan_fix_project_get_hash (HyScanFixProjectVersion version)
{
  switch (version)
    {
    case HYSCAN_FIX_PROJECT_3E65462D:
      return "3e65462db44e1dc9317f38a063d60ef1";
    case HYSCAN_FIX_PROJECT_6190124D:
      return "6190124dbc946f010e07a5ffb86f68ee";
    case HYSCAN_FIX_PROJECT_2C71F69B:
      return "2c71f69b31e8d610da88edcb8a165fe0";
    case HYSCAN_FIX_PROJECT_E38FABCF:
      return "e38fabcf1b95fced8c87447c1ffa0f32";
    case HYSCAN_FIX_PROJECT_3C282D25:
      return "3c282d259e3db686fef4c3b26dde5edc";
    case HYSCAN_FIX_PROJECT_7F9EB90C:
      return "7f9eb90c7f852d6457d7a1bdc5d9b012";
    case HYSCAN_FIX_PROJECT_FD8E8922:
      return "fd8e8922e61b89e749e7b076444c039e";
    case HYSCAN_FIX_PROJECT_DE7491C1:
      return "de7491c1298d4368ad87ded88e617fea";
    case HYSCAN_FIX_PROJECT_AD1F40A3:
      return "ad1f40a3292926b566fe8ed6465a6e64";
    case HYSCAN_FIX_PROJECT_B288BA04:
      return "b288ba043bea8a11886a458a4bfab4a8";
    case HYSCAN_FIX_PROJECT_C95A6F48:
      return "c95a6f48d4f0f2784dfd45351ab83f8a";
    case HYSCAN_FIX_PROJECT_8C1D17C8:
      return "8c1d17c827ebbc76fca7548e4ca06226";
    default:
      break;
    }

  return NULL;
}

/* Функция преобразовывает идентификаторы источников данных.
 *
 * SIDE_SCAN_STARBOARD     101, 201 -> 2
 * SIDE_SCAN_PORT          102, 202 -> 5
 * SIDE_SCAN_STARBOARD_HI  103, 203 -> 4
 * SIDE_SCAN_PORT_HI       104, 204 -> 7
 * ECHOSOUNDER             107, 205 -> 8
 * PROFILER                108, 209 -> 13
 */
static gint
hyscan_fix_project_update_source_enum_6190124d (gint source)
{
  switch (source)
    {
    case 101:
    case 201:
      return 2;
    case 102:
    case 202:
      return 5;
    case 103:
    case 203:
      return 4;
    case 104:
    case 204:
      return 7;
    case 107:
    case 205:
      return 8;
    case 108:
    case 209:
      return 13;
    }

  return -1;
}

/* Функция преобразовывает численное значение идентификатора источника
 * данных в строковое название.*/
static const gchar *
hyscan_fix_project_get_source_by_id_7f9eb90c (gint source)
{
  switch (source)
    {
    case 2:
      return "ss-starboard";
    case 3:
      return "ss-starboard-low";
    case 4:
      return "ss-starboard-hi";
    case 5:
      return "ss-port";
    case 6:
      return "ss-port-low";
    case 7:
      return "ss-port-hi";
    case 8:
      return "echosounder";
    case 9:
      return "echosounder-low";
    case 10:
      return "echosounder-hi";
    case 13:
      return "profiler";
    case 14:
      return "profiler-echo";
    }

  return NULL;
}

/* Функция записывает схему параметров проекта для указанной версии. */
static gboolean
hyscan_fix_project_set_schema (const gchar             *db_path,
                               const gchar             *project_path,
                               HyScanFixProjectVersion  version)
{
  gboolean status = FALSE;
  gchar *sch_file = NULL;

  sch_file = g_build_filename (project_path, "project.prm", "project.sch", NULL);
  if (hyscan_fix_file_backup (db_path, sch_file, TRUE))
    status = hyscan_fix_file_schema (db_path, sch_file, hyscan_fix_project_get_hash (version));
  g_free (sch_file);

  return status;
}

/* Функция обновляет формат данных параметров проекта с версии 3e65462d
 * до 6190124d.
 *
 * Версия 3e65462d внутренняя разработка.
 *
 * При обновлении до 6190124d добавлена схема для водопадных меток.
 */
static gboolean
hyscan_fix_project_3e65462d (const gchar *db_path,
                             const gchar *project_path)
{
  gboolean status = FALSE;

  /* Обновление схемы параметров проекта. */
  if (!hyscan_fix_project_set_schema (db_path, project_path, HYSCAN_FIX_PROJECT_6190124D))
    goto exit;

  /* Уборка. */
  status = hyscan_fix_cleanup (db_path);

exit:
  return status;
}

/* Функция обновляет формат данных параметров проекта с версии 6190124d
 * до e38fabcf.
 *
 * Версия 6190124d записывалась версией ПО для АМЭ на испытаниях.
 *
 * При обновлении до e38fabcf необходимо создать параметры с информацией
 * о проекте и галсах. Группа водопадных меток переименована из
 * waterfall-marks.prm в waterfall-mark.prm. Изменились идентификаторы
 * источников данных меток.
 */
static gboolean
hyscan_fix_project_6190124d (const gchar *db_path,
                             const gchar *project_path)
{
  gboolean status = FALSE;
  HyScanFixFileIDType id;
  gchar *id_file = NULL;
  gchar *project_ids = NULL;
  gchar *track_ids = NULL;
  gchar *project_info_file = NULL;
  gchar *track_info_file = NULL;
  gchar *old_mark_file = NULL;
  gchar *new_mark_file = NULL;
  gchar *tracks_path = NULL;
  gchar **tracks = NULL;
  GKeyFile *project_info = NULL;
  GKeyFile *track_info = NULL;
  GKeyFile *params_in = NULL;
  GKeyFile *params_out = NULL;
  gchar **groups = NULL;
  gchar **keys = NULL;
  guint i,j;

  /* Дата и время создания проекта. */
  id_file = g_build_filename (project_path, "project.id", NULL);
  id = hyscan_fix_file_db_id (db_path, id_file);
  if ((GUINT32_FROM_LE (id.magic) != PROJECT_FILE_MAGIC) ||
      (GUINT32_FROM_LE (id.version) != PROJECT_FILE_VERSION))
    {
      goto exit;
    }

  /* Идентификатор проекта. */
  project_ids = hyscan_fix_id_create ();

  /* Группа параметров с информацией. */
  project_info = g_key_file_new ();

  /* Информация о проекте. */
  g_key_file_set_string (project_info, "project", "schema-id", "project-info");
  g_key_file_set_int64 (project_info, "project", "/ctime", 1000000 * GINT64_FROM_LE (id.dt));
  g_key_file_set_int64 (project_info, "project", "/mtime", g_get_real_time ());
  g_key_file_set_string (project_info, "project", "/id", project_ids);

  /* Информация о галсах. */
  tracks_path = g_build_filename (db_path, project_path, NULL);
  tracks = hyscan_fix_dir_list (tracks_path);
  for (i = 0; (tracks != NULL) && (tracks[i] != NULL); i++)
    {
      HyScanFixTrackVersion version;

      version = hyscan_fix_track_get_version (tracks_path, tracks[i]);
      if (version == HYSCAN_FIX_TRACK_NOT_TRACK)
        continue;
      if (version != HYSCAN_FIX_TRACK_LATEST)
        goto exit;

      track_info = g_key_file_new ();
      track_info_file = g_build_filename (tracks_path, tracks[i], "track.prm", NULL);
      if (!g_key_file_load_from_file (track_info, track_info_file, G_KEY_FILE_NONE, NULL))
        goto exit;

      track_ids = g_key_file_get_string (track_info, "track", "/id", NULL);
      if (track_ids == NULL)
        goto exit;

      g_key_file_set_string (project_info, track_ids, "schema-id", "track-info");
      g_key_file_set_int64 (project_info, track_ids, "/mtime", g_get_real_time ());

      g_clear_pointer (&track_info, g_key_file_unref);
      g_clear_pointer (&track_info_file, g_free);
      g_clear_pointer (&track_ids, g_free);
    }

  /* Сохраняем группу с информацией. */
  project_info_file = g_build_filename (db_path, project_path, "project.prm", "info.prm", NULL);
  if (!g_key_file_save_to_file (project_info, project_info_file, NULL))
    goto exit;

  /* Копируем водопадные метки. */
  old_mark_file = g_build_filename (project_path, "project.prm", "waterfall-marks.prm", NULL);
  new_mark_file = g_build_filename (project_path, "project.prm", "waterfall-mark.prm", NULL);
  if (!hyscan_fix_file_copy (db_path, old_mark_file, new_mark_file, FALSE))
    goto exit;

  /* Преобразование параметров меток. */
  g_free (new_mark_file);
  new_mark_file = g_build_filename (db_path, project_path, "project.prm", "waterfall-mark.prm", NULL);

  params_in = g_key_file_new ();
  g_key_file_load_from_file (params_in, new_mark_file, G_KEY_FILE_NONE, NULL);

  params_out = g_key_file_new ();
  groups = g_key_file_get_groups (params_in, NULL);

  for (i = 0; groups != NULL && groups[i] != NULL; i++)
    {
      keys = g_key_file_get_keys (params_in, groups[i], NULL, NULL);

      for (j = 0; keys != NULL && keys[j] != NULL; j++)
        {
          if (g_strcmp0 (keys[j], "/coordinates/source0") == 0)
            {
              gint source_old = g_key_file_get_integer (params_in, groups[i], keys[j], NULL);
              gint source_new = hyscan_fix_project_update_source_enum_6190124d (source_old);
              g_key_file_set_integer (params_out, groups[i], keys[j], source_new);
            }
          else
            {
              gchar *value = g_key_file_get_string (params_in, groups[i], keys[j], NULL);
              g_key_file_set_string (params_out, groups[i], keys[j], value);
              g_free (value);
              continue;
            }
        }

      g_strfreev (keys);
    }

  /* Записываем изменённые параметры. */
  if (!g_key_file_save_to_file (params_out, new_mark_file, NULL))
    goto exit;

  g_strfreev (groups);

  /* Обновление схемы параметров проекта. */
  if (!hyscan_fix_project_set_schema (db_path, project_path, HYSCAN_FIX_PROJECT_E38FABCF))
    goto exit;

  /* Уборка. */
  status = hyscan_fix_cleanup (db_path);

exit:
  g_clear_pointer (&project_info, g_key_file_unref);
  g_clear_pointer (&track_info, g_key_file_unref);
  g_strfreev (tracks);
  g_free (id_file);
  g_free (tracks_path);
  g_free (project_ids);
  g_free (track_ids);
  g_free (project_info_file);
  g_free (track_info_file);
  g_free (old_mark_file);
  g_free (new_mark_file);

  return status;
}

/* Функция обновляет формат данных параметров проекта с версий
 * e38fabcf и 2c71f69b до 3c282d25.
 *
 * Версия e38fabcf записывается текущей версией ПО для АМЭ.
 * Версия 2c71f69b из "master'а".
 *
 * При обновлении до 3c282d25 добавлена схема для геометок.
 */
static gboolean
hyscan_fix_project_e38fabcf (const gchar *db_path,
                             const gchar *project_path)
{
  gboolean status = FALSE;

  /* Обновление схемы параметров проекта. */
  if (!hyscan_fix_project_set_schema (db_path, project_path, HYSCAN_FIX_PROJECT_3C282D25))
    goto exit;

  /* Уборка. */
  status = hyscan_fix_cleanup (db_path);

exit:
  return status;
}

/* Функция обновляет формат данных параметров проекта с версии 3c282d25
 * до 7f9eb90c.
 *
 * Версия 3c282d25 внутренняя разработка.
 *
 * При обновлении до 06ada1af изменились названия параметров меток:
 *
 * /time/creation       -> /ctime
 * /time/modification   -> /mtime
 * /coordinates/source0 -> /source
 * /coordinates/index0  -> /index
 * /coordinates/count0  -> /count
 * /coordinates/lat     -> /lat
 * /coordinates/lon     -> /lon
 * /coordinates/width   -> /width + преобразование в метры из мм
 * /coordinates/height  -> /height + преобразование в метры из мм
 */
static gboolean
hyscan_fix_project_3c282d25 (const gchar *db_path,
                             const gchar *project_path)
{
  gboolean status = FALSE;
  gchar *prm_file = NULL;

  GKeyFile *params_in = NULL;
  GKeyFile *params_out = NULL;
  gchar **groups = NULL;
  guint i, j;

  /* Бэкап меток. */
  prm_file = g_build_filename (project_path, "project.prm", "waterfall-mark.prm", NULL);
  if (!hyscan_fix_file_backup (db_path, prm_file, FALSE))
    goto exit;

  /* Преобразование параметров меток. */
  g_free (prm_file);
  prm_file = g_build_filename (db_path, project_path, "project.prm", "waterfall-mark.prm", NULL);

  params_in = g_key_file_new ();
  params_out = g_key_file_new ();

  g_key_file_load_from_file (params_in, prm_file, G_KEY_FILE_NONE, NULL);
  groups = g_key_file_get_groups (params_in, NULL);

  for (i = 0; groups != NULL && groups[i] != NULL; i++)
    {
      gchar **keys = g_key_file_get_keys (params_in, groups[i], NULL, NULL);

      for (j = 0; keys != NULL && keys[j] != NULL; j++)
        {
          if (g_strcmp0 (keys[j], "/time/creation") == 0)
            {
              gint64 ctime = g_key_file_get_int64 (params_in, groups[i], keys[j], NULL);
              g_key_file_set_int64 (params_out, groups[i], "/ctime", ctime);
            }
          else if (g_strcmp0 (keys[j], "/time/modification") == 0)
            {
              gint64 mtime = g_key_file_get_int64 (params_in, groups[i], keys[j], NULL);
              g_key_file_set_int64 (params_out, groups[i], "/mtime", mtime);
            }
          else if (g_strcmp0 (keys[j], "/coordinates/source0") == 0)
            {
              gint source = g_key_file_get_integer (params_in, groups[i], keys[j], NULL);
              g_key_file_set_integer (params_out, groups[i], "/source", source);
            }
          else if (g_strcmp0 (keys[j], "/coordinates/index0") == 0)
            {
              gint index = g_key_file_get_integer (params_in, groups[i], keys[j], NULL);
              g_key_file_set_integer (params_out, groups[i], "/index", index);
            }
          else if (g_strcmp0 (keys[j], "/coordinates/count0") == 0)
            {
              gint count = g_key_file_get_integer (params_in, groups[i], keys[j], NULL);
              g_key_file_set_integer (params_out, groups[i], "/count", count);
            }
          else if (g_strcmp0 (keys[j], "/coordinates/lat") == 0)
            {
              gdouble lat = g_key_file_get_double (params_in, groups[i], keys[j], NULL);
              g_key_file_set_double (params_out, groups[i], "/lat", lat);
            }
          else if (g_strcmp0 (keys[j], "/coordinates/lon") == 0)
            {
              gdouble lon = g_key_file_get_double (params_in, groups[i], keys[j], NULL);
              g_key_file_set_double (params_out, groups[i], "/lon", lon);
            }
          else if (g_strcmp0 (keys[j], "/coordinates/width") == 0)
            {
              gdouble width = g_key_file_get_double (params_in, groups[i], keys[j], NULL);
              width /= 1000.0;
              g_key_file_set_double (params_out, groups[i], "/width", width);
            }
          else if (g_strcmp0 (keys[j], "/coordinates/height") == 0)
            {
              gdouble height = g_key_file_get_double (params_in, groups[i], keys[j], NULL);
              height /= 1000.0;
              g_key_file_set_double (params_out, groups[i], "/height", height);
            }
          else
            {
              gchar *value = g_key_file_get_string (params_in, groups[i], keys[j], NULL);
              g_key_file_set_string (params_out, groups[i], keys[j], value);
              g_free (value);
            }
        }

      g_strfreev (keys);
    }

  g_strfreev (groups);

  /* Записываем изменённые параметры. */
  if (!g_key_file_save_to_file (params_out, prm_file, NULL))
    goto exit;

  /* Обновление схемы параметров галса. */
  if (!hyscan_fix_project_set_schema (db_path, project_path, HYSCAN_FIX_PROJECT_7F9EB90C))
    goto exit;

  /* Уборка. */
  status = hyscan_fix_cleanup (db_path);

exit:
  g_clear_pointer (&params_in, g_key_file_unref);
  g_clear_pointer (&params_out, g_key_file_unref);
  g_free (prm_file);

  return status;
}

/* Функция обновляет формат данных параметров проекта с версии 7f9eb90c
 * до fd8e8922.
 *
 * Версия 7f9eb90c записывалась sonobot-19, сборка от 18 июля 2019 года.
 *
 * При обновлении до fd8e8922 изменился тип источника данных в параметрах метки с
 * integer на string.
 */
static gboolean
hyscan_fix_project_7f9eb90c (const gchar *db_path,
                             const gchar *project_path)
{
  gboolean status = FALSE;
  gchar *prm_file = NULL;

  GKeyFile *params_in = NULL;
  GKeyFile *params_out = NULL;
  gchar **groups = NULL;
  gchar **keys = NULL;
  guint i,j;

  /* Бэкап меток. */
  prm_file = g_build_filename (project_path, "project.prm", "waterfall-mark.prm", NULL);
  if (!hyscan_fix_file_backup (db_path, prm_file, FALSE))
    goto exit;

  /* Преобразование параметров меток. */
  g_free (prm_file);
  prm_file = g_build_filename (db_path, project_path, "project.prm", "waterfall-mark.prm", NULL);

  params_in = g_key_file_new ();
  g_key_file_load_from_file (params_in, prm_file, G_KEY_FILE_NONE, NULL);

  params_out = g_key_file_new ();
  groups = g_key_file_get_groups (params_in, NULL);

  for (i = 0; groups != NULL && groups[i] != NULL; i++)
    {
      keys = g_key_file_get_keys (params_in, groups[i], NULL, NULL);

      for (j = 0; keys != NULL && keys[j] != NULL; j++)
        {
          if (g_strcmp0 (keys[j], "/source") == 0)
            {
              gint source_id = g_key_file_get_integer (params_in, groups[i], keys[j], NULL);
              const gchar *source = hyscan_fix_project_get_source_by_id_7f9eb90c (source_id);
              g_key_file_set_string (params_out, groups[i], keys[j], source);
            }
          else
            {
              gchar *value = g_key_file_get_string (params_in, groups[i], keys[j], NULL);
              g_key_file_set_string (params_out, groups[i], keys[j], value);
              g_free (value);
              continue;
            }
        }

      g_strfreev (keys);
    }

  g_strfreev (groups);

  /* Записываем изменённые параметры. */
  if (!g_key_file_save_to_file (params_out, prm_file, NULL))
    goto exit;

  /* Обновление схемы параметров галса. */
  if (!hyscan_fix_project_set_schema (db_path, project_path, HYSCAN_FIX_PROJECT_FD8E8922))
    goto exit;

  /* Уборка. */
  status = hyscan_fix_cleanup (db_path);

exit:
  g_clear_pointer (&params_in, g_key_file_unref);
  g_clear_pointer (&params_out, g_key_file_unref);
  g_free (prm_file);

  return status;
}

/* Функция обновляет формат данных параметров проекта с версии fd8e8922
 * до de7491c1.
 *
 * Версия fd8e8922 записывалась ревизией 1 online инсталятора HyScan 5.
 *
 * При обновлении до de7491c1 изменилась версия схемы водопадных меток.
 * Данные не изменяются.
 */
static gboolean
hyscan_fix_project_fd8e8922 (const gchar *db_path,
                             const gchar *project_path)
{
  gboolean status = FALSE;

  /* Обновление схемы параметров проекта. */
  if (!hyscan_fix_project_set_schema (db_path, project_path, HYSCAN_FIX_PROJECT_DE7491C1))
    goto exit;

  /* Уборка. */
  status = hyscan_fix_cleanup (db_path);

exit:
  return status;
}

/* Функция обновляет формат данных параметров проекта с версии de7491c1
 * до ad1f40a3.
 *
 * Версия de7491c1 записывалась ревизиями со 2-й по 4-ю online инсталятора
 * HyScan 5.
 *
 * При обновлении до ad1f40a3 добавлены схемы для планировщика, но
 * реального использования этих схем нет.
 */
static gboolean
hyscan_fix_project_de7491c1 (const gchar *db_path,
                             const gchar *project_path)
{
  gboolean status = FALSE;

  /* Обновление схемы параметров проекта. */
  if (!hyscan_fix_project_set_schema (db_path, project_path, HYSCAN_FIX_PROJECT_AD1F40A3))
    goto exit;

  /* Уборка. */
  status = hyscan_fix_cleanup (db_path);

exit:
  return status;
}

/* Функция обновляет формат данных параметров проекта с версии ad1f40a3
 * до b288ba04.
 *
 * Версия ad1f40a3 являлась внутренней версией.
 *
 * При обновлении до b288ba04 изменены схемы для планировщика, но
 * реального использования этих схем нет.
 */
static gboolean
hyscan_fix_project_ad1f40a3 (const gchar *db_path,
                             const gchar *project_path)
{
  gboolean status = FALSE;

  /* Обновление схемы параметров проекта. */
  if (!hyscan_fix_project_set_schema (db_path, project_path, HYSCAN_FIX_PROJECT_B288BA04))
    goto exit;

  /* Уборка. */
  status = hyscan_fix_cleanup (db_path);

exit:
  return status;
}

/* Функция обновляет формат данных параметров проекта с версии b288ba04
 * до c95a6f48.
 *
 * Версия b288ba04 записывалась ревизией HyScan для "корейцев" от 04.04.2020.
 *
 * При обновлении до c95a6f48 изменились названия параметров планируемых галсов:
 *
 * /start-lat -> /start/lat
 * /start-lon -> /start/lon
 * /end-lat   -> /end/lat
 * /end-lon   -> /end/lon
 */
static gboolean
hyscan_fix_project_b288ba04 (const gchar *db_path,
                             const gchar *project_path)
{
  gboolean status = FALSE;
  gchar *prm_file = NULL;

  GKeyFile *params_in = NULL;
  GKeyFile *params_out = NULL;
  gchar **groups = NULL;
  guint i, j;

  /* Бэкап плана съёмки. */
  prm_file = g_build_filename (project_path, "project.prm", "planner.prm", NULL);
  if (!hyscan_fix_file_backup (db_path, prm_file, FALSE))
    goto exit;

  /* Преобразование параметров плана съёмки. */
  g_free (prm_file);
  prm_file = g_build_filename (db_path, project_path, "project.prm", "planner.prm", NULL);

  params_in = g_key_file_new ();
  params_out = g_key_file_new ();

  g_key_file_load_from_file (params_in, prm_file, G_KEY_FILE_NONE, NULL);
  groups = g_key_file_get_groups (params_in, NULL);

  for (i = 0; groups != NULL && groups[i] != NULL; i++)
    {
      gchar **keys = g_key_file_get_keys (params_in, groups[i], NULL, NULL);

      for (j = 0; keys != NULL && keys[j] != NULL; j++)
        {
          if (g_strcmp0 (keys[j], "/start-lat") == 0)
            {
              gdouble start_lat = g_key_file_get_double (params_in, groups[i], keys[j], NULL);
              g_key_file_set_double (params_out, groups[i], "/start/lat", start_lat);
            }
          else if (g_strcmp0 (keys[j], "/start-lon") == 0)
            {
              gdouble start_lon = g_key_file_get_double (params_in, groups[i], keys[j], NULL);
              g_key_file_set_double (params_out, groups[i], "/start/lon", start_lon);
            }
          else if (g_strcmp0 (keys[j], "/end-lat") == 0)
            {
              gdouble end_lat = g_key_file_get_double (params_in, groups[i], keys[j], NULL);
              g_key_file_set_double (params_out, groups[i], "/end/lat", end_lat);
            }
          else if (g_strcmp0 (keys[j], "/end-lon") == 0)
            {
              gdouble end_lon = g_key_file_get_double (params_in, groups[i], keys[j], NULL);
              g_key_file_set_double (params_out, groups[i], "/end/lon", end_lon);
            }
          else
            {
              gchar *value = g_key_file_get_string (params_in, groups[i], keys[j], NULL);
              g_key_file_set_string (params_out, groups[i], keys[j], value);
              g_free (value);
            }
        }

      g_strfreev (keys);
    }

  g_strfreev (groups);

  /* Записываем изменённые параметры. */
  if (!g_key_file_save_to_file (params_out, prm_file, NULL))
    goto exit;

  /* Обновление схемы параметров галса. */
  if (!hyscan_fix_project_set_schema (db_path, project_path, HYSCAN_FIX_PROJECT_C95A6F48))
    goto exit;

  /* Уборка. */
  status = hyscan_fix_cleanup (db_path);

exit:
  g_clear_pointer (&params_in, g_key_file_unref);
  g_clear_pointer (&params_out, g_key_file_unref);
  g_free (prm_file);

  return status;
}

/* Функция обновляет формат данных параметров проекта с версии b288ba04
 * до 8c1d17c8.
 *
 * Версия b288ba04 являлась внутренней версией.
 *
 * При обновлении до 8c1d17c8 внесены следующие изменения:
 *
 * - добавлена схема данных группы объектов "label",
 * - добавлено поле /labels в схему информации о галсе "track-info",
 * - в схемах меток переименовано поле /label -> /labels,
 * - добавлена схема параметров отображения галсов на планшете "map-track".
 */
static gboolean
hyscan_fix_project_c95a6f48 (const gchar *db_path,
                             const gchar *project_path)
{
  gboolean status = FALSE;
  const gchar *files[] = {"waterfall-mark.prm", "geo-mark.prm"};
  gchar *prm_file = NULL;
  guint k;

  for (k = 0; k < G_N_ELEMENTS (files); k++)
    {
      gboolean file_status;
      GKeyFile *params_in;
      GKeyFile *params_out;
      gchar **groups;
      guint i, j;

      /* Бэкап меток. */
      prm_file = g_build_filename (project_path, "project.prm", files[k], NULL);
      if (!hyscan_fix_file_backup (db_path, prm_file, FALSE))
        goto exit;

      /* Преобразование параметров меток. */
      g_free (prm_file);
      prm_file = g_build_filename (db_path, project_path, "project.prm", files[k], NULL);

      params_in = g_key_file_new ();
      params_out = g_key_file_new ();

      g_key_file_load_from_file (params_in, prm_file, G_KEY_FILE_NONE, NULL);
      groups = g_key_file_get_groups (params_in, NULL);

      for (i = 0; groups != NULL && groups[i] != NULL; i++)
        {
          gchar **keys = g_key_file_get_keys (params_in, groups[i], NULL, NULL);

          for (j = 0; keys != NULL && keys[j] != NULL; j++)
            {
              if (g_strcmp0 (keys[j], "/label") == 0)
                {
                  gint64 labels = g_key_file_get_int64 (params_in, groups[i], keys[j], NULL);
                  g_key_file_set_int64 (params_out, groups[i], "/labels", labels);
                }
              else
                {
                  gchar *value = g_key_file_get_string (params_in, groups[i], keys[j], NULL);
                  g_key_file_set_string (params_out, groups[i], keys[j], value);
                  g_free (value);
                }
            }

          g_strfreev (keys);
        }

      g_strfreev (groups);

      /* Записываем изменённые параметры. */
      file_status = g_key_file_save_to_file (params_out, prm_file, NULL);
      g_key_file_unref (params_in);
      g_key_file_unref (params_out);
      g_clear_pointer (&prm_file, g_free);

      if (!file_status)
        goto exit;
    }

  /* Обновление схемы параметров проекта. */
  if (!hyscan_fix_project_set_schema (db_path, project_path, HYSCAN_FIX_PROJECT_8C1D17C8))
    goto exit;

  /* Уборка. */
  status = hyscan_fix_cleanup (db_path);

exit:
  g_free (prm_file);

  return status;
}

/**
 * hyscan_fix_project_get_version:
 * @db_path: путь к базе данных (каталог с проектами)
 * @project_path: путь к проекту относительно db_path
 *
 * Функция определяет версию формата данных параметров проекта.
 *
 * Returns: Версия формата данных параметров проекта.
 */
HyScanFixProjectVersion
hyscan_fix_project_get_version (const gchar *db_path,
                                const gchar *project_path)
{
  HyScanFixProjectVersion version = HYSCAN_FIX_PROJECT_NOT_PROJECT;
  HyScanFixFileIDType id;

  gchar *id_file = NULL;
  gchar *sch_file = NULL;
  gchar *sch_md5 = NULL;

  id_file = g_build_filename (project_path, "project.id", NULL);
  id = hyscan_fix_file_db_id (db_path, id_file);
  if ((GUINT32_FROM_LE (id.magic) != PROJECT_FILE_MAGIC) ||
      (GUINT32_FROM_LE (id.version) != PROJECT_FILE_VERSION))
    {
      goto exit;
    }

  version = HYSCAN_FIX_PROJECT_UNKNOWN;

  sch_file = g_build_filename (project_path, "project.prm", "project.sch", NULL);
  sch_md5 = hyscan_fix_file_md5 (db_path, sch_file);
  if (sch_md5 == NULL)
    goto exit;

  while (version < HYSCAN_FIX_PROJECT_LAST)
    {
      if (g_strcmp0 (sch_md5, hyscan_fix_project_get_hash (version)) == 0)
        break;
      version += 1;
    }

  if (version == HYSCAN_FIX_PROJECT_LAST)
    version = HYSCAN_FIX_PROJECT_UNKNOWN;

  if (version == HYSCAN_FIX_PROJECT_UNKNOWN)
    hyscan_fix_log (db_path, "unknown project version %s - %s", project_path, sch_md5);

exit:
  g_free (sch_md5);
  g_free (sch_file);
  g_free (id_file);

  return version;
}

/**
 * hyscan_fix_project:
 * @db_path: путь к базе данных (каталог с проектами)
 * @project_path: путь к проекту относительно db_path
 *
 * Функция обновляет формат данных параметров проекта. При этом
 * происходит последовательное обновление формата данных от одной
 * версии к другой, до текущей используемой в HyScan.
 *
 * Returns: %TRUE если обновление успешно завершено, иначе %FALSE.
 */
gboolean
hyscan_fix_project (const gchar *db_path,
                    const gchar *project_path)
{
  HyScanFixProjectVersion version;
  gboolean status = TRUE;

  /* Проверяем состояние базы данных и откатываем изменения
   * в случае ошибки при предыдущем обновлении. */
  if (!hyscan_fix_revert (db_path))
    return FALSE;

  version = hyscan_fix_project_get_version (db_path, project_path);
  switch (version)
    {
    case HYSCAN_FIX_PROJECT_NOT_PROJECT:
      status = TRUE;
      break;

    case HYSCAN_FIX_PROJECT_UNKNOWN:
      status = FALSE;
      break;

    case HYSCAN_FIX_PROJECT_3E65462D:
      if (status)
        status = hyscan_fix_project_3e65462d (db_path, project_path);

    case HYSCAN_FIX_PROJECT_6190124D:
      if (status)
        status = hyscan_fix_project_6190124d (db_path, project_path);

    case HYSCAN_FIX_PROJECT_2C71F69B:
    case HYSCAN_FIX_PROJECT_E38FABCF:
      if (status)
        status = hyscan_fix_project_e38fabcf (db_path, project_path);

    case HYSCAN_FIX_PROJECT_3C282D25:
      if (status)
        status = hyscan_fix_project_3c282d25 (db_path, project_path);

    case HYSCAN_FIX_PROJECT_7F9EB90C:
      if (status)
        status = hyscan_fix_project_7f9eb90c (db_path, project_path);

    case HYSCAN_FIX_PROJECT_FD8E8922:
      if (status)
        status = hyscan_fix_project_fd8e8922 (db_path, project_path);

    case HYSCAN_FIX_PROJECT_DE7491C1:
      if (status)
        status = hyscan_fix_project_de7491c1 (db_path, project_path);

    case HYSCAN_FIX_PROJECT_AD1F40A3:
      if (status)
        status = hyscan_fix_project_ad1f40a3 (db_path, project_path);

    case HYSCAN_FIX_PROJECT_B288BA04:
      if (status)
        status = hyscan_fix_project_b288ba04 (db_path, project_path);

    case HYSCAN_FIX_PROJECT_C95A6F48:
      if (status)
        status = hyscan_fix_project_c95a6f48 (db_path, project_path);

    case HYSCAN_FIX_PROJECT_8C1D17C8:
      break;

    case HYSCAN_FIX_PROJECT_LAST:
      status = FALSE;
      break;
    }

  return status;
}
