/* hyscan-fix-common.c
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

#include "hyscan-fix-common.h"

#include <glib/gstdio.h>
#include <gio/gio.h>
#include <string.h>

#define BACKUP_INDEX   "update.backup"
#define CLEANUP_INDEX  "update.cleanup"
#define UPDATE_LOG     "update.log"

/**
 * hyscan_fix_id_create:
 *
 * Функция создаёт строку с уникальным идентификатором.
 *
 * Returns: (transfer full): Строка с уникальным идентификатором.
 * Для удаления #g_free.
 */
gchar *
hyscan_fix_id_create (void)
{
  GRand *rand = g_rand_new ();
  gchar buffer[33] = {0};
  guint i;

  for (i = 0; i < 32; i++)
    {
      gint rnd = g_rand_int_range (rand, 0, 62);
      if (rnd < 10)
        buffer[i] = '0' + rnd;
      else if (rnd < 36)
        buffer[i] = 'a' + rnd - 10;
      else
        buffer[i] = 'A' + rnd - 36;
    }

  g_rand_free (rand);

  return g_strdup (buffer);
}

/**
 * hyscan_fix_dir_list:
 * @path: рабочий путь
 *
 * Функция возвращает список каталогов для указанного пути.
 *
 * Returns: (transfer full): %NULL-терминированный список каталогов.
 * Для удаления #g_strfreev.
 */
gchar **
hyscan_fix_dir_list (const gchar *path)
{
  GDir *dir;
  const gchar *name;
  GArray *names;

  dir = g_dir_open (path, 0, NULL);
  if (dir == NULL)
    return NULL;

  names = g_array_new (TRUE, TRUE, sizeof (gchar *));

  while ((name = g_dir_read_name (dir)) != NULL)
    {
      gchar *full = g_build_filename (path, name, NULL);

      if (g_file_test (full, G_FILE_TEST_IS_DIR))
        {
          gchar *copy = g_strdup (name);
          g_array_append_val (names, copy);
        }

      g_free (full);
    }

  g_dir_close (dir);

  return (gchar**)g_array_free (names, FALSE);
}

/**
 * hyscan_fix_file_exist:
 * @db_path: путь к базе данных (каталог с проектами)
 * @file_path: путь к файлу относительно db_path
 *
 * Функция проверяет существует файл или нет. Функция выполняет проверку
 * пытаясь открыть файл на чтение.
 *
 * Returns: %TRUE если файл существует, иначе %FALSE.
 */
gboolean
hyscan_fix_file_exist (const gchar *db_path,
                       const gchar *file_path)
{
  gchar *path;
  GFile *file;
  gboolean exist;

  path = g_build_filename (db_path, file_path, NULL);
  file = g_file_new_for_path  (path);
  exist = g_file_query_exists (file, NULL);
  g_object_unref (file);
  g_free (path);

  return exist;
}

/**
 * hyscan_fix_file_md5:
 * @db_path: путь к базе данных (каталог с проектами)
 * @file_path: путь к файлу относительно db_path
 *
 * Функция считает MD5 сумму для данных в файле.
 *
 * Returns: (transfer full): Строка md5 хэшем содержимого файла.
 * Для удаления #g_free.
 */
gchar *
hyscan_fix_file_md5 (const gchar *db_path,
                     const gchar *file_path)
{
  gchar *file;
  gchar *data;
  gchar *md5;
  gsize size;

  file = g_build_filename (db_path, file_path, NULL);

  if (!g_file_get_contents (file, &data, &size, NULL))
    return NULL;

  md5 = g_compute_checksum_for_string (G_CHECKSUM_MD5, data, size);

  g_free (data);
  g_free (file);

  return md5;
}

/**
 * hyscan_fix_file_db_id:
 * @db_path: путь к базе данных (каталог с проектами)
 * @file_path: путь к файлу относительно db_path
 *
 * Функция считывает ID файл базы данных.
 *
 * Returns: Содержимое ID файла базы данных.
 */
HyScanFixFileIDType
hyscan_fix_file_db_id (const gchar *db_path,
                       const gchar *file_path)
{
  gchar *file;
  GFile *fin;
  GFileInputStream *sin;
  HyScanFixFileIDType id;

  file = g_build_filename (db_path, file_path, NULL);
  fin = g_file_new_for_path (file);
  sin = g_file_read (fin, NULL, NULL);

  if (sin == NULL)
    goto exit;

  if (g_input_stream_read (G_INPUT_STREAM (sin), &id, sizeof (id), NULL, NULL) != sizeof (id))
    memset (&id, 0, sizeof (id));

  g_object_unref (sin);

exit:
  g_object_unref (fin);
  g_free (file);

  return id;
}

/**
 * hyscan_fix_file_append:
 * @db_path: путь к базе данных (каталог с проектами)
 * @file_path: путь к файлу относительно db_path
 * @str: строка для добавления к файлу
 *
 * Функция  добавляет строку к файлу.
 *
 * Returns: %TRUE если строка добавлена, иначе %FALSE.
 */
gboolean
hyscan_fix_file_append (const gchar *db_path,
                        const gchar *file_path,
                        const gchar *str)
{
  gboolean status;
  gchar *file;
  gchar *data;
  gsize size;
  gsize len;

  len = strlen (str);
  file = g_build_filename (db_path, file_path, NULL);

  if (!g_file_get_contents (file, &data, &size, NULL))
    {
      data = NULL;
      size = 0;
    }

  data = g_realloc (data, size + len + 1);
  g_snprintf (data + size, len + 1, "%s", str);

  size += len;
  status = g_file_set_contents (file, data, size, NULL);

  g_free (file);
  g_free (data);

  return status;
}

/**
 * hyscan_fix_file_backup:
 * @db_path: путь к базе данных (каталог с проектами)
 * @file_path: путь к файлу относительно db_path
 * @exist: TRUE если файл должен существовать
 *
 * Функция создаёт резервную копию файла и сохраняет информацию о ней
 * в файле с откатом изменений.
 *
 * Returns: %TRUE если резервная копия создана, иначе %FALSE.
 */
gboolean
hyscan_fix_file_backup (const gchar *db_path,
                        const gchar *file_path,
                        gboolean     exist)
{
  gboolean status = FALSE;
  gchar *backup_index = NULL;
  gchar *cleanup_index = NULL;
  gchar *from = NULL;
  gchar *to = NULL;
  gchar *data = NULL;
  gchar *md5 = NULL;
  gsize size;

  from = g_build_filename (db_path, file_path, NULL);
  to = g_strdup_printf ("%s.bak", from);

  if (!g_file_get_contents (from, &data, &size, NULL))
    {
      status = !exist;
      goto exit;
    }

  if (!g_file_set_contents (to, data, size, NULL))
    goto exit;

  md5 = g_compute_checksum_for_string (G_CHECKSUM_MD5, data, size);
  backup_index = g_strdup_printf ("%s: %s\n", file_path, md5);
  if (!hyscan_fix_file_append (db_path, BACKUP_INDEX, backup_index))
    goto exit;

  cleanup_index = g_strdup_printf ("%s.bak\n", file_path);
  if (!hyscan_fix_file_append (db_path, CLEANUP_INDEX, cleanup_index))
    goto exit;

  status = hyscan_fix_log (db_path, "backup file %s\n", file_path);

exit:
  g_free (cleanup_index);
  g_free (backup_index);
  g_free (from);
  g_free (to);
  g_free (data);
  g_free (md5);

  return status;
}

/**
 * hyscan_fix_file_copy:
 * @db_path: путь к базе данных (каталог с проектами)
 * @src_path: исходный путь к файлу относительно db_path
 * @dst_path: целевой путь к файлу относительно db_path
 * @exist: TRUE если файл должен существовать
 *
 * Функция копирует файл и сохраняет информацию о необходимости удаления
 * оригинального файла.
 *
 * Returns: %TRUE если копия создана, иначе %FALSE.
 */
gboolean
hyscan_fix_file_copy (const gchar *db_path,
                      const gchar *src_path,
                      const gchar *dst_path,
                      gboolean     exist)
{
  gboolean status = FALSE;
  gchar *cleanup_index = NULL;
  gchar *src_file = NULL;
  gchar *dst_file = NULL;
  GFile *src = NULL;
  GFile *dst = NULL;
  GError *error = NULL;

  src_file = g_build_filename (db_path, src_path, NULL);
  dst_file = g_build_filename (db_path, dst_path, NULL);

  src = g_file_new_for_path (src_file);
  dst = g_file_new_for_path (dst_file);

  if (!g_file_copy (src, dst, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, &error))
    {
      if (error->code == G_IO_ERROR_NOT_FOUND)
        status = !exist;
      goto exit;
    }

  cleanup_index = g_strdup_printf ("%s\n", src_path);
  if (!hyscan_fix_file_append (db_path, CLEANUP_INDEX, cleanup_index))
    goto exit;

  status = hyscan_fix_log (db_path, "copy file %s\n", src_path);

exit:
  g_clear_object (&src);
  g_clear_object (&dst);
  g_free (src_file);
  g_free (dst_file);

  return status;
}

/**
 * hyscan_fix_file_mark_remove:
 * @db_path: путь к базе данных (каталог с проектами)
 * @file_path: путь к файлу относительно db_path
 *
 * Функция помечает файл для удаления.
 *
 * Returns: %TRUE если файл отмечен, иначе %FALSE.
 */
gboolean
hyscan_fix_file_mark_remove (const gchar *db_path,
                             const gchar *file_path)
{
  gchar *cleanup_index;
  gboolean status;

  cleanup_index = g_strdup_printf ("%s\n", file_path);
  status = hyscan_fix_file_append (db_path, CLEANUP_INDEX, cleanup_index);
  g_free (cleanup_index);

  return status;
}

/**
 * hyscan_fix_file_schema:
 * @db_path: путь к базе данных (каталог с проектами)
 * @file_path: путь к файлу относительно db_path
 * @schema_version: версия схемы данных
 *
 * Функция записывает в файл новую схему данных с указанной версией.
 *
 * Returns: %TRUE если схема записана, иначе %FALSE.
 */
gboolean
hyscan_fix_file_schema (const gchar *db_path,
                        const gchar *file_path,
                        const gchar *schema_version)
{
  gboolean status = FALSE;
  gchar *schema_id = NULL;
  gchar *schema_file = NULL;
  GBytes *schema = NULL;
  const gchar *data;
  gsize size;

  schema_id = g_strdup_printf ("/org/hyscan/schemas/%s", schema_version);
  schema_file = g_build_filename (db_path, file_path, NULL);

  schema = g_resources_lookup_data (schema_id, 0, NULL);
  if (schema == NULL)
    goto exit;

  data = g_bytes_get_data (schema, &size);
  if (data == NULL)
    goto exit;

  status = g_file_set_contents (schema_file, data, size, NULL);

exit:
  g_free (schema_id);
  g_free (schema_file);
  g_clear_pointer (&schema, g_bytes_unref);

  return status;
}

/**
 * hyscan_fix_log:
 * @db_path: путь к базе данных (каталог с проектами)
 * @format: формат сообщения
 * @...: NULL терминированный список параметров сообщения
 *
 * Функция записывает сообщение в лог файл.
 *
 * Returns: %TRUE если сообщение записано, иначе %FALSE.
 */
gboolean
hyscan_fix_log (const gchar *db_path,
                const gchar *format,
                ...)
{
  va_list list;
  GString *message;
  gboolean status;

  va_start (list, format);
  message = g_string_new (NULL);

  g_string_vprintf (message, format, list);
  status = hyscan_fix_file_append (db_path, UPDATE_LOG, message->str);

  g_string_free (message, TRUE);
  va_end (list);

  return status;
}

/**
 * hyscan_fix_cleanup:
 * @db_path: путь к базе данных (каталог с проектами)
 *
 * Функция удаляет вспомогательные файлы, созданные при обновлении.
 *
 * Returns: %TRUE если фйлы удалены, иначе %FALSE.
 */
gboolean
hyscan_fix_cleanup (const gchar *db_path)
{
  gboolean status = FALSE;
  gchar *backup_index = NULL;
  gchar *update_log = NULL;
  gchar *cleanup_index = NULL;
  gchar **list = NULL;
  gchar *data = NULL;
  gsize size;
  guint i;

  backup_index = g_build_filename (db_path, BACKUP_INDEX, NULL);
  update_log = g_build_filename (db_path, UPDATE_LOG, NULL);
  cleanup_index = g_build_filename (db_path, CLEANUP_INDEX, NULL);

  g_unlink (backup_index);
  g_unlink (update_log);

  if (g_file_get_contents (cleanup_index, &data, &size, NULL))
    {
      list = g_strsplit (data, "\n", -1);
      if (list == NULL)
        goto exit;

      for (i = 0; list[i] != NULL && list[i][0] != 0; i++)
        {
          gchar *file;

          file = g_build_filename (db_path, list[i], NULL);
          status = (g_unlink (file) == 0);
          g_free (file);

          if (!status)
            goto exit;
        }

      g_unlink (cleanup_index);
    }

  status = TRUE;

exit:
  g_strfreev (list);
  g_free (backup_index);
  g_free (update_log);
  g_free (cleanup_index);
  g_free (data);

  return status;
}

/**
 * hyscan_fix_revert:
 * @db_path: путь к базе данных (каталог с проектами)
 *
 * функция отменяет изменения в проекте или галсе из бэкапа.
 *
 * Returns: %TRUE если изменения отменены, иначе %FALSE.
 */
gboolean
hyscan_fix_revert (const gchar *db_path)
{
  gboolean status = FALSE;
  gchar *backup_index = NULL;
  gchar **list = NULL;
  gchar **info = NULL;
  gchar *file = NULL;
  gchar *from = NULL;
  gchar *md5 = NULL;
  gchar *data = NULL;
  gsize size;
  guint i;

  backup_index = g_build_filename (db_path, BACKUP_INDEX, NULL);
  if (g_file_get_contents (backup_index, &data, &size, NULL))
    {
      list = g_strsplit (data, "\n", -1);
      if (list == NULL)
        goto exit;

      for (i = 0; list[i] != NULL; i++)
        {
          info = g_strsplit (list[i], ": ", -1);
          if (info != NULL && info[0] != NULL && info[1] != NULL)
            {
              file = g_build_filename (db_path, info[0], NULL);
              from = g_strdup_printf ("%s.bak", file);

              g_free (data);
              if (!g_file_get_contents (from, &data, &size, NULL))
                goto exit;

              md5 = g_compute_checksum_for_string (G_CHECKSUM_MD5, data, size);
              if (g_strcmp0 (md5, info[1]) != 0)
                goto exit;

              if (!g_file_set_contents (file, data, size, NULL))
                goto exit;

              g_clear_pointer (&data, g_free);
              g_clear_pointer (&file, g_free);
              g_clear_pointer (&from, g_free);
              g_clear_pointer (&md5, g_free);
            }
        }
    }

  status = hyscan_fix_cleanup (db_path);

exit:
  g_strfreev (list);
  g_strfreev (info);
  g_free (backup_index);
  g_free (data);
  g_free (file);
  g_free (from);
  g_free (md5);

  return status;
}
