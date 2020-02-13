/* hyscan-fix-track.c
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

#include <hyscan-data-schema-builder.h>

#include "hyscan-fix-track.h"
#include "hyscan-fix-common.h"

#define TRACK_FILE_MAGIC       0x52545348      /* HSTR в виде строки. */
#define TRACK_FILE_VERSION     0x31303731      /* 1701 в виде строки. */

/* Функция возвращает значение контрольной суммы для версии схемы. */
static const gchar *
hyscan_fix_track_get_hash (HyScanFixTrackVersion version)
{
  switch (version)
    {
    case HYSCAN_FIX_TRACK_2F9C8A44:
      return "2f9c8a4405c52774c22fbd1e00c45ab2";
    case HYSCAN_FIX_TRACK_19A285F3:
      return "19a285f398b930c936df0567abf356b3";
    case HYSCAN_FIX_TRACK_9726336A:
      return "9726336af32731e31dd3752276981192";
    default:
      break;
    }
  return NULL;
}

/* Функция копирует данные канала. */
static gboolean
hyscan_fix_track_copy_channel (const gchar *db_path,
                               const gchar *track_path,
                               const gchar *src_channel,
                               const gchar *dst_channel)
{
  gboolean status = FALSE;
  gchar *src_file = NULL;
  gchar *dst_file = NULL;
  gchar *src_path = NULL;
  gchar *dst_path = NULL;
  guint32 n_segments = 0;
  guint32 i;

  if (dst_channel == NULL)
    return FALSE;

  if (g_strcmp0 (src_channel, dst_channel) == 0)
    return TRUE;

  /* Считаем число сегментов данных. */
  while (TRUE)
    {
      gboolean has_index;
      gboolean has_data;

      src_file = g_strdup_printf ("%s.%06d.i", src_channel, n_segments);
      src_path = g_build_filename (track_path, src_file, NULL);
      has_index = hyscan_fix_file_exist (db_path, src_path);
      g_clear_pointer (&src_file, g_free);
      g_clear_pointer (&src_path, g_free);

      src_file = g_strdup_printf ("%s.%06d.d", src_channel, n_segments);
      src_path = g_build_filename (track_path, src_file, NULL);
      has_data = hyscan_fix_file_exist (db_path, src_path);
      g_clear_pointer (&src_file, g_free);
      g_clear_pointer (&src_path, g_free);

      if (has_index && has_data)
        {
          n_segments += 1;
          continue;
        }

      if ((has_index && !has_data) || (!has_index && has_data))
        goto exit;

      break;
    }

  /* Копируем файлы канала. */
  for (i = 0; i < n_segments; i++)
    {
      src_file = g_strdup_printf ("%s.%06d.i", src_channel, i);
      dst_file = g_strdup_printf ("%s.%06d.i", dst_channel, i);
      src_path = g_build_filename (track_path, src_file, NULL);
      dst_path = g_build_filename (track_path, dst_file, NULL);

      if (!hyscan_fix_file_copy (db_path, src_path, dst_path, TRUE))
        goto exit;

      g_clear_pointer (&src_file, g_free);
      g_clear_pointer (&dst_file, g_free);
      g_clear_pointer (&src_path, g_free);
      g_clear_pointer (&dst_path, g_free);

      src_file = g_strdup_printf ("%s.%06d.d", src_channel, i);
      dst_file = g_strdup_printf ("%s.%06d.d", dst_channel, i);
      src_path = g_build_filename (track_path, src_file, NULL);
      dst_path = g_build_filename (track_path, dst_file, NULL);

      if (!hyscan_fix_file_copy (db_path, src_path, dst_path, TRUE))
        goto exit;

      g_clear_pointer (&src_file, g_free);
      g_clear_pointer (&dst_file, g_free);
      g_clear_pointer (&src_path, g_free);
      g_clear_pointer (&dst_path, g_free);
    }

  status = TRUE;

exit:
  g_free (src_file);
  g_free (dst_file);
  g_free (src_path);
  g_free (dst_path);

  return status;
}

/* Функия отмечает файлы канала данных для удаления. */
static gboolean
hyscan_fix_track_mark_channel_remove (const gchar *db_path,
                                      const gchar *track_path,
                                      const gchar *channel)
{
  gboolean status = FALSE;
  gchar *file = NULL;
  gchar *path = NULL;
  guint32 n_segments = 0;

  while (TRUE)
    {
      gboolean has_index;
      gboolean has_data;

      file = g_strdup_printf ("%s.%06d.i", channel, n_segments);
      path = g_build_filename (track_path, file, NULL);
      has_index = hyscan_fix_file_exist (db_path, path);
      if (has_index)
        hyscan_fix_file_mark_remove (db_path, path);
      g_clear_pointer (&file, g_free);
      g_clear_pointer (&path, g_free);

      file = g_strdup_printf ("%s.%06d.d", channel, n_segments);
      path = g_build_filename (track_path, file, NULL);
      has_data = hyscan_fix_file_exist (db_path, path);
      if (has_data)
        hyscan_fix_file_mark_remove (db_path, path);
      g_clear_pointer (&file, g_free);
      g_clear_pointer (&path, g_free);

      if (has_index && has_data)
        {
          n_segments += 1;
          continue;
        }

      if ((has_index && !has_data) || (!has_index && has_data))
        goto exit;

      break;
    }

  status = TRUE;

exit:
  g_free (file);
  g_free (path);

  return status;
}

/* Функция преобразовывает названия каналов данных. */
static const gchar *
hyscan_fix_track_update_channel_name_2f9c8a44 (const gchar *name)
{
  if (g_strcmp0 (name, "ss-port-raw") == 0)
    return "ss-port";
  if (g_strcmp0 (name, "ss-port-raw-noise") == 0)
    return "ss-port-noise";
  if (g_strcmp0 (name, "ss-port-raw-signal") == 0)
    return "ss-port-signal";
  if (g_strcmp0 (name, "ss-port-raw-tvg") == 0)
    return "ss-port-tvg";

  if (g_strcmp0 (name, "ss-starboard-raw") == 0)
    return "ss-starboard";
  if (g_strcmp0 (name, "ss-starboard-raw-noise") == 0)
    return "ss-starboard-noise";
  if (g_strcmp0 (name, "ss-starboard-raw-signal") == 0)
    return "ss-starboard-signal";
  if (g_strcmp0 (name, "ss-starboard-raw-tvg") == 0)
    return "ss-starboard-tvg";

  if (g_strcmp0 (name, "echosounder-raw") == 0)
    return "echosounder";
  if (g_strcmp0 (name, "echosounder-raw-noise") == 0)
    return "echosounder-noise";
  if (g_strcmp0 (name, "echosounder-raw-signal") == 0)
    return "echosounder-signal";
  if (g_strcmp0 (name, "echosounder-raw-tvg") == 0)
    return "echosounder-tvg";

  if (g_strcmp0 (name, "forward-look-raw-1") == 0)
    return "forward-look";
  if (g_strcmp0 (name, "forward-look-raw-1-noise") == 0)
    return "forward-look-noise";
  if (g_strcmp0 (name, "forward-look-raw-1-signal") == 0)
    return "forward-look-signal";
  if (g_strcmp0 (name, "forward-look-raw-1-tvg") == 0)
    return "forward-look-tvg";

  if (g_strcmp0 (name, "forward-look-raw-2") == 0)
    return "forward-look-2";
  if (g_strcmp0 (name, "forward-look-raw-2-noise") == 0)
    return "forward-look-noise-2";
  if (g_strcmp0 (name, "forward-look-raw-2-signal") == 0)
    return "forward-look-signal-2";
  if (g_strcmp0 (name, "forward-look-raw-2-tvg") == 0)
    return "forward-look-tvg-2";

  if (g_strcmp0 (name, "profiler") == 0)
    return "profiler";

  if (g_strcmp0 (name, "nmea") == 0)
    return "nmea";
  if (g_strcmp0 (name, "nmea-2") == 0)
    return "nmea-2";

  if (g_strcmp0 (name, "track") == 0)
    return "track";

  return NULL;
}

/* Функция обновляет схему с информацией о гидролокаторе. */
static gchar *
hyscan_fix_track_update_sonar_2f9c8a44 (GKeyFile *src_params)
{
  gchar *sonar_data = NULL;
  gchar *tmp_data = NULL;
  gchar *info = NULL;

  gchar **channels = NULL;
  const gchar *channel;
  guint i;

  HyScanDataSchemaBuilder *builder = NULL;
  HyScanDataSchema *schema = NULL;
  GRegex *regex;

  /* Старая информация о гидролокаторе. */
  sonar_data = g_key_file_get_string (src_params, "track", "/sonar", NULL);
  if (sonar_data == NULL)
    goto exit;

  /* Схема имеет старый формат, её необходимо преобразовать.
   * Заменить "readonly" на "r". */
  regex = g_regex_new ("readonly", 0, 0, NULL);
  tmp_data = g_regex_replace (regex, sonar_data, -1, 0, "r", 0, NULL);
  g_regex_unref (regex);

  schema = hyscan_data_schema_new_from_string (tmp_data, "info");
  if (schema == NULL)
    goto exit;

  /* Старую информацию переносим в ветку /info/hydra. */
  builder = hyscan_data_schema_builder_new ("info");
  if (!hyscan_data_schema_builder_schema_join (builder, "/info/hydra", schema, "/"))
    goto exit;

  /* Дополняем информацией по драйверу. */
  hyscan_data_schema_builder_key_string_create (builder, "/info/hydra/drv",
                                                "Driver", NULL, "Hydra4");
  hyscan_data_schema_builder_key_set_access (builder, "/info/hydra/drv",
                                             HYSCAN_DATA_SCHEMA_ACCESS_READ);

  hyscan_data_schema_builder_key_string_create (builder, "/info/hydra/drv-version",
                                                "Driver version", NULL, "4a");
  hyscan_data_schema_builder_key_set_access (builder, "/info/hydra/drv-version",
                                             HYSCAN_DATA_SCHEMA_ACCESS_READ);

  /* Добавляем информацию по источникам данных. */
  channels = g_key_file_get_groups (src_params, NULL);
  if (channels == NULL)
    goto exit;

  for (i = 0; channels[i] != NULL; i++)
    {
      gchar *key_id;

      channel = hyscan_fix_track_update_channel_name_2f9c8a44 (channels[i]);

      /* Добавляем информацию по источникам данных. */
      if ((g_strcmp0 (channel, "ss-port") == 0) ||
          (g_strcmp0 (channel, "ss-starboard") == 0) ||
          (g_strcmp0 (channel, "echosounder") == 0) ||
          (g_strcmp0 (channel, "forward-look") == 0) ||
          (g_strcmp0 (channel, "profiler") == 0))
        {
          key_id = g_strdup_printf ("/info/sources/%s/dev-id", channel);
          hyscan_data_schema_builder_key_string_create (builder, key_id, "dev-id", NULL, "hydra");
          hyscan_data_schema_builder_key_set_access (builder, key_id, HYSCAN_DATA_SCHEMA_ACCESS_READ);
          g_free (key_id);

          key_id = g_strdup_printf ("/info/sources/%s/description", channel);
          hyscan_data_schema_builder_key_string_create (builder, key_id, "description", NULL, channel);
          hyscan_data_schema_builder_key_set_access (builder, key_id, HYSCAN_DATA_SCHEMA_ACCESS_READ);
          g_free (key_id);
        }
      else if ((g_strcmp0 (channel, "nmea") == 0) ||
               (g_strcmp0 (channel, "nmea-2") == 0))
        {
          key_id = g_strdup_printf ("/info/sensors/%s/dev-id", channel);
          hyscan_data_schema_builder_key_string_create (builder, key_id, "dev-id", NULL, "hydra");
          hyscan_data_schema_builder_key_set_access (builder, key_id, HYSCAN_DATA_SCHEMA_ACCESS_READ);
          g_free (key_id);

          key_id = g_strdup_printf ("/info/sensors/%s/description", channel);
          hyscan_data_schema_builder_key_string_create (builder, key_id, "description", NULL, "NMEA sensor");
          hyscan_data_schema_builder_key_set_access (builder, key_id, HYSCAN_DATA_SCHEMA_ACCESS_READ);
          g_free (key_id);
        }
    }

  info = hyscan_data_schema_builder_get_data (builder);

exit:
  g_clear_object (&builder);
  g_clear_object (&schema);
  g_free (sonar_data);
  g_free (tmp_data);
  g_strfreev (channels);

  return info;
}

/* Функция возвращает значение рабочей частоты канала. */
static gdouble
hyscan_fix_track_get_signal_frequency (const gchar *source,
                                       gdouble      data_rate,
                                       gdouble      signal_frequency)
{
  gint irate = data_rate;

  switch (irate)
    {
    /* ППФ Гидра 4. */
    case 61276:
      if (g_str_has_prefix (source, "profiler"))
        return 12000.0;
      break;

    /* ППФ Гидра 5. */
    case 58640:
      if (g_str_has_prefix (source, "profiler"))
        return 12000.0;
      break;

    /* Эхолот ППФ Гидра 5. */
    case 78125:
      if (g_str_has_prefix (source, "echosounder"))
        return 315657.0;
      break;

    /* ГБО 300 Гидра 4 левый борт. */
    case 52083:
      if (g_str_has_prefix (source, "ss-port"))
        return 240000.0;
      break;

    /* ГБО 300 Гидра 4 правый борт. */
    case 68681:
      if (g_str_has_prefix (source, "ss-starboard"))
        return 250000.0;
      break;

    /* ГБОЭ 700 Гидра 5 эхолот. */
    case 208333:
      if (g_str_has_prefix (source, "echosounder"))
        return 1041670.0;
      break;

    /* ГБОЭ 700 Гидра 5 левый и правый борта. */
    case 223214:
      if (g_str_has_prefix (source, "ss-port"))
        return 538793;
      if (g_str_has_prefix (source, "ss-starboard"))
        return 664894;
      break;

    /* ВСЛ Гидра 5. */
    case 111607:
      if (g_str_has_prefix (source, "forward-look"))
       return 434027.0;
      break;

    /* ВСЛ Гидра 5 левый и правый борта. */
    case 156250:
      if (g_str_has_prefix (source, "ss-port"))
        return 679348;
      if (g_str_has_prefix (source, "ss-starboard"))
        return 679348;
      break;
    }

  if (signal_frequency > 1e3)
    return signal_frequency;

  return -1.0;
}



/* Функция преобразовывает параметры каналов данных. */
static gboolean
hyscan_fix_track_update_params_2f9c8a44 (GKeyFile    *src_params,
                                         GKeyFile    *dst_params,
                                         const gchar *src_group,
                                         const gchar *dst_group,
                                         gint64       ctime)
{
  gboolean status = FALSE;
  gchar *schema_id = NULL;
  gchar **keys = NULL;

  schema_id = g_key_file_get_string (src_params, src_group, "schema-id", NULL);
  if (schema_id == NULL)
    goto exit;

  keys = g_key_file_get_keys (src_params, src_group, NULL, NULL);
  if (keys == NULL)
    goto exit;

  /* Информация о галсе. Здесь необходимо добавить поле ctime с датой и временем
   * создания галса, а также преобразовать схему с информацией о гидролокаторе. */
  if (g_strcmp0 (schema_id, "track") == 0)
    {
      gchar *track_id;
      gchar *track_type;
      gchar *sonar_data;

      track_id = g_key_file_get_string (src_params, src_group, "/id", NULL);
      track_type = g_key_file_get_string (src_params, src_group, "/type", NULL);
      sonar_data = hyscan_fix_track_update_sonar_2f9c8a44 (src_params);
      ctime *= G_USEC_PER_SEC;

      if (sonar_data == NULL)
        goto exit;

      g_key_file_set_string (dst_params, dst_group, "schema-id", "track");
      g_key_file_set_string (dst_params, dst_group, "/id", track_id);
      g_key_file_set_string (dst_params, dst_group, "/type", track_type);
      g_key_file_set_string (dst_params, dst_group, "/sonar", sonar_data);
      g_key_file_set_int64 (dst_params, dst_group, "/ctime", ctime);

      g_free (sonar_data);
      g_free (track_type);
      g_free (track_id);
    }

  else if (g_strcmp0 (schema_id, "raw") == 0)
    {
      gchar   *data_type;
      gdouble  data_rate;
      gdouble  antenna_voffset;
      gdouble  antenna_hoffset;
      gdouble  antenna_frequency;
      gdouble  antenna_bandwidth;
      gdouble  signal_frequency;
      gdouble  signal_bandwidth;
      gdouble  position_x;
      gdouble  position_y;
      gdouble  position_z;
      gdouble  position_psi;
      gdouble  position_gamma;
      gdouble  position_theta;

      data_type = g_key_file_get_string (src_params, src_group, "/data/type", NULL);
      data_rate = g_key_file_get_double (src_params, src_group, "/data/rate", NULL);
      antenna_voffset = g_key_file_get_double (src_params, src_group, "/antenna/offset/vertical", NULL);
      antenna_hoffset = g_key_file_get_double (src_params, src_group, "/antenna/offset/horizontal", NULL);
      antenna_frequency = g_key_file_get_double (src_params, src_group, "/antenna/frequency", NULL);
      antenna_bandwidth = g_key_file_get_double (src_params, src_group, "/antenna/bandwidth", NULL);
      position_x = g_key_file_get_double (src_params, src_group, "/position/x", NULL);
      position_y = g_key_file_get_double (src_params, src_group, "/position/y", NULL);
      position_z = g_key_file_get_double (src_params, src_group, "/position/z", NULL);
      position_psi = g_key_file_get_double (src_params, src_group, "/position/psi", NULL);
      position_gamma = g_key_file_get_double (src_params, src_group, "/position/gamma", NULL);
      position_theta = g_key_file_get_double (src_params, src_group, "/position/theta", NULL);

      signal_frequency = hyscan_fix_track_get_signal_frequency (dst_group, data_rate, antenna_frequency);
      signal_bandwidth = 0.18 * signal_frequency;

      if ((g_strcmp0 (data_type, "complex-adc14le") != 0) || (signal_frequency < 0))
        {
          goto exit;
        }

      g_key_file_set_string (dst_params, dst_group, "schema-id", "acoustic");
      g_key_file_set_string (dst_params, dst_group, "/data/type", data_type);
      g_key_file_set_double (dst_params, dst_group, "/data/rate", data_rate);
      g_key_file_set_double (dst_params, dst_group, "/antenna/offset/vertical", antenna_voffset);
      g_key_file_set_double (dst_params, dst_group, "/antenna/offset/horizontal", antenna_hoffset);
      g_key_file_set_double (dst_params, dst_group, "/antenna/frequency", antenna_frequency);
      g_key_file_set_double (dst_params, dst_group, "/antenna/bandwidth", antenna_bandwidth);
      g_key_file_set_double (dst_params, dst_group, "/signal/frequency", signal_frequency);
      g_key_file_set_double (dst_params, dst_group, "/signal/bandwidth", signal_bandwidth);
      g_key_file_set_double (dst_params, dst_group, "/offset/x", position_x);
      g_key_file_set_double (dst_params, dst_group, "/offset/y", position_y);
      g_key_file_set_double (dst_params, dst_group, "/offset/z", position_z);
      g_key_file_set_double (dst_params, dst_group, "/offset/psi", position_psi);
      g_key_file_set_double (dst_params, dst_group, "/offset/gamma", position_gamma);
      g_key_file_set_double (dst_params, dst_group, "/offset/theta", position_theta);

      g_free (data_type);
    }

  else if (g_strcmp0 (schema_id, "signal") == 0)
    {
      gchar   *data_type;
      gdouble  data_rate;

      data_type = g_key_file_get_string (src_params, src_group, "/data/type", NULL);
      data_rate = g_key_file_get_double (src_params, src_group, "/data/rate", NULL);

      if (g_strcmp0 (data_type, "complex-float") != 0)
        goto exit;

      g_key_file_set_string (dst_params, dst_group, "schema-id", "signal");
      g_key_file_set_string (dst_params, dst_group, "/data/type", "complex-float32le");
      g_key_file_set_double (dst_params, dst_group, "/data/rate", data_rate);

      g_free (data_type);
    }

  else if (g_strcmp0 (schema_id, "tvg") == 0)
    {
      gchar   *data_type;
      gdouble  data_rate;

      data_type = g_key_file_get_string (src_params, src_group, "/data/type", NULL);
      data_rate = g_key_file_get_double (src_params, src_group, "/data/rate", NULL);

      if (g_strcmp0 (data_type, "float") != 0)
        goto exit;

      g_key_file_set_string (dst_params, dst_group, "schema-id", "tvg");
      g_key_file_set_string (dst_params, dst_group, "/data/type", "float32le");
      g_key_file_set_double (dst_params, dst_group, "/data/rate", data_rate);

      g_free (data_type);
    }

  else if (g_strcmp0 (schema_id, "acoustic") == 0)
    {
      gchar   *data_type;
      gdouble  data_rate;
      gdouble  position_x;
      gdouble  position_y;
      gdouble  position_z;
      gdouble  position_psi;
      gdouble  position_gamma;
      gdouble  position_theta;

      data_type = g_key_file_get_string (src_params, src_group, "/data/type", NULL);
      data_rate = g_key_file_get_double (src_params, src_group, "/data/rate", NULL);
      position_x = g_key_file_get_double (src_params, src_group, "/position/x", NULL);
      position_y = g_key_file_get_double (src_params, src_group, "/position/y", NULL);
      position_z = g_key_file_get_double (src_params, src_group, "/position/z", NULL);
      position_psi = g_key_file_get_double (src_params, src_group, "/position/psi", NULL);
      position_gamma = g_key_file_get_double (src_params, src_group, "/position/gamma", NULL);
      position_theta = g_key_file_get_double (src_params, src_group, "/position/theta", NULL);

      if (g_strcmp0 (data_type, "float") != 0)
        goto exit;

      g_key_file_set_string (dst_params, dst_group, "schema-id", "acoustic");
      g_key_file_set_string (dst_params, dst_group, "/data/type", "float32le");
      g_key_file_set_double (dst_params, dst_group, "/data/rate", data_rate);
      g_key_file_set_double (dst_params, dst_group, "/offset/x", position_x);
      g_key_file_set_double (dst_params, dst_group, "/offset/y", position_y);
      g_key_file_set_double (dst_params, dst_group, "/offset/z", position_z);
      g_key_file_set_double (dst_params, dst_group, "/offset/psi", position_psi);
      g_key_file_set_double (dst_params, dst_group, "/offset/gamma", position_gamma);
      g_key_file_set_double (dst_params, dst_group, "/offset/theta", position_theta);

      g_free (data_type);
    }

  else if (g_strcmp0 (schema_id, "sensor") == 0)
    {
      gdouble  position_x;
      gdouble  position_y;
      gdouble  position_z;
      gdouble  position_psi;
      gdouble  position_gamma;
      gdouble  position_theta;

      position_x = g_key_file_get_double (src_params, src_group, "/position/x", NULL);
      position_y = g_key_file_get_double (src_params, src_group, "/position/y", NULL);
      position_z = g_key_file_get_double (src_params, src_group, "/position/z", NULL);
      position_psi = g_key_file_get_double (src_params, src_group, "/position/psi", NULL);
      position_gamma = g_key_file_get_double (src_params, src_group, "/position/gamma", NULL);
      position_theta = g_key_file_get_double (src_params, src_group, "/position/theta", NULL);

      g_key_file_set_string (dst_params, dst_group, "schema-id", "sensor");
      g_key_file_set_string (dst_params, dst_group, "/sensor-name", dst_group);
      g_key_file_set_double (dst_params, dst_group, "/offset/x", position_x);
      g_key_file_set_double (dst_params, dst_group, "/offset/y", position_y);
      g_key_file_set_double (dst_params, dst_group, "/offset/z", position_z);
      g_key_file_set_double (dst_params, dst_group, "/offset/psi", position_psi);
      g_key_file_set_double (dst_params, dst_group, "/offset/gamma", position_gamma);
      g_key_file_set_double (dst_params, dst_group, "/offset/theta", position_theta);
    }

  status = TRUE;

exit:
  g_free (schema_id);
  g_strfreev (keys);

  return status;
}

/* Функция записывает схему параметров проекта для указанной версии. */
static gboolean
hyscan_fix_track_set_schema (const gchar           *db_path,
                             const gchar           *track_path,
                             HyScanFixTrackVersion  version)
{
  gboolean status = FALSE;
  gchar *sch_file = NULL;

  sch_file = g_build_filename (track_path, "track.sch", NULL);
  if (hyscan_fix_file_backup (db_path, sch_file, TRUE))
    status = hyscan_fix_file_schema (db_path, sch_file, hyscan_fix_track_get_hash (version));
  g_free (sch_file);

  return status;
}

/* Функция обновляет формат данных галса с версии 2f9c8a44 до 19a285f3. */
static gboolean
hyscan_fix_track_2f9c8a44 (const gchar       *db_path,
                           const gchar       *track_path,
                           HyScanCancellable *cancellable)
{
  gboolean status = FALSE;
  HyScanFixFileIDType id;
  gchar *id_path = NULL;
  gchar *prm_file = NULL;
  GKeyFile *src_params = NULL;
  GKeyFile *dst_params = NULL;
  gchar **groups = NULL;
  guint i;

  /* Время создания галса. */
  id_path = g_build_filename (track_path, "track.id", NULL);
  id = hyscan_fix_file_db_id (db_path, id_path);
  g_free (id_path);

  /* Бэкап параметров галса. */
  prm_file = g_build_filename (track_path, "track.prm", NULL);
  if (!hyscan_fix_file_backup (db_path, prm_file, TRUE))
    goto exit;

  /* Преобразование параметров и данных галса. */
  g_free (prm_file);
  prm_file = g_build_filename (db_path, track_path, "track.prm", NULL);

  src_params = g_key_file_new ();
  if (!g_key_file_load_from_file (src_params, prm_file, G_KEY_FILE_NONE, NULL))
    goto exit;

  dst_params = g_key_file_new ();
  hyscan_cancellable_push (cancellable);
  groups = g_key_file_get_groups (src_params, NULL);
  for (i = 0; groups != NULL && groups[i] != NULL; i++)
    {
      const gchar *channel;

      hyscan_cancellable_set_total (cancellable, i, 0, g_strv_length (groups));

      /* Каналы галса. */
      channel = hyscan_fix_track_update_channel_name_2f9c8a44 (groups[i]);
      if (channel == NULL)
        {
          hyscan_fix_track_mark_channel_remove (db_path, track_path, groups[i]);
          continue;
        }

      /* Копируем данные канала. */
      if (!hyscan_fix_track_copy_channel (db_path, track_path, groups[i], channel))
        goto exit;

      /* Преобразовываем параметры канала. */
      if (!hyscan_fix_track_update_params_2f9c8a44 (src_params, dst_params, groups[i], channel, id.dt))
        goto exit;
    }
  hyscan_cancellable_pop (cancellable);

  /* Записываем изменённые параметры. */
  if (!g_key_file_save_to_file (dst_params, prm_file, NULL))
    goto exit;

  /* Обновление схемы параметров галса. */
  if (!hyscan_fix_track_set_schema (db_path, track_path, HYSCAN_FIX_TRACK_9726336A))
    goto exit;

  /* Уборка. */
  status = hyscan_fix_cleanup (db_path);

exit:
  g_clear_pointer (&src_params, g_key_file_unref);
  g_clear_pointer (&dst_params, g_key_file_unref);
  g_clear_pointer (&groups, g_strfreev);

  g_free (prm_file);

  return status;
}

/* Функция обновляет формат данных галса с версии 19a285f3 до 9726336a.
 *
 * Версия 19a285f3 записывалась ревизиями с 1-й по 4-ю online
 * инсталятора HyScan 5, сонобот-19 и версией для АМЭ от 2019 года.
 *
 * Изменяются названия параметров каналов данных:
 * /offset/x     -> /offset/forward
 * /offset/y     -> /offset/starboard
 * /offset/z     -> /offset/vertical
 * /offset/psi   -> /offset/yaw + инвертирование знака
 * /offset/gamma -> /offset/roll + инвертирование знака
 * /offset/theta -> /offset/pitch + инвертирование знака
 */
static gboolean
hyscan_fix_track_19a285f3 (const gchar *db_path,
                           const gchar *track_path)
{
  gboolean status = FALSE;
  gchar *prm_file = NULL;

  GKeyFile *params_in = NULL;
  GKeyFile *params_out = NULL;
  gchar **groups = NULL;
  gchar **keys = NULL;
  guint i,j;

  /* Бэкап параметров галса. */
  prm_file = g_build_filename (track_path, "track.prm", NULL);
  if (!hyscan_fix_file_backup (db_path, prm_file, TRUE))
    goto exit;

  /* Преобразование параметров галса. */
  g_free (prm_file);
  prm_file = g_build_filename (db_path, track_path, "track.prm", NULL);

  params_in = g_key_file_new ();
  if (!g_key_file_load_from_file (params_in, prm_file, G_KEY_FILE_NONE, NULL))
    goto exit;

  params_out = g_key_file_new ();
  groups = g_key_file_get_groups (params_in, NULL);

  for (i = 0; groups != NULL && groups[i] != NULL; i++)
    {
      keys = g_key_file_get_keys (params_in, groups[i], NULL, NULL);

      for (j = 0; keys != NULL && keys[j] != NULL; j++)
        {
          const gchar *key_out;
          gdouble dvalue;
          gchar *svalue;

          dvalue = g_key_file_get_double (params_in, groups[i], keys[j], NULL);
          svalue = g_key_file_get_string (params_in, groups[i], keys[j], NULL);

          if (g_strcmp0 (keys[j], "/offset/x") == 0)
            {
              key_out = "/offset/forward";
            }
          else if (g_strcmp0 (keys[j], "/offset/y") == 0)
            {
              key_out = "/offset/starboard";
            }
          else if (g_strcmp0 (keys[j], "/offset/z") == 0)
            {
              key_out = "/offset/vertical";
            }
          else if (g_strcmp0 (keys[j], "/offset/psi") == 0)
            {
              key_out = "/offset/yaw";
              dvalue = -dvalue;
            }
          else if (g_strcmp0 (keys[j], "/offset/gamma") == 0)
            {
              key_out = "/offset/roll";
              dvalue = -dvalue;
            }
          else if (g_strcmp0 (keys[j], "/offset/theta") == 0)
            {
              key_out = "/offset/pitch";
              dvalue = -dvalue;
            }
          else
            {
              key_out = keys[j];
            }

          if (key_out == keys[j])
            {
              g_key_file_set_string (params_out, groups[i], key_out, svalue);
            }
          else
            {
              g_key_file_set_double (params_out, groups[i], key_out, dvalue);

              hyscan_fix_log (db_path, "rename track parameter in %s: %s -> %s\n",
                              groups[i], keys[j], key_out);
            }

          g_free (svalue);
        }

      g_clear_pointer (&keys, g_strfreev);
    }

  /* Записываем изменённые параметры. */
  if (!g_key_file_save_to_file (params_out, prm_file, NULL))
    goto exit;

  /* Обновление схемы параметров галса. */
  if (!hyscan_fix_track_set_schema (db_path, track_path, HYSCAN_FIX_TRACK_9726336A))
    goto exit;

  /* Уборка. */
  status = hyscan_fix_cleanup (db_path);

exit:
  g_clear_pointer (&params_in, g_key_file_unref);
  g_clear_pointer (&params_out, g_key_file_unref);
  g_clear_pointer (&groups, g_strfreev);
  g_clear_pointer (&keys, g_strfreev);
  g_free (prm_file);

  return status;
}

/**
 * hyscan_fix_track_get_version:
 * @db_path: путь к базе данных (каталог с проектами)
 * @track_path: путь к проекту относительно db_path
 *
 * Функция определяет версию формата данных галса.
 *
 * Returns: Версия формата данных галса.
 */
HyScanFixTrackVersion
hyscan_fix_track_get_version (const gchar *db_path,
                              const gchar *track_path)
{
  HyScanFixTrackVersion version = HYSCAN_FIX_TRACK_NOT_TRACK;
  HyScanFixFileIDType id;

  gchar *id_file = NULL;
  gchar *sch_file = NULL;
  gchar *sch_md5 = NULL;

  id_file = g_build_filename (track_path, "track.id", NULL);
  id = hyscan_fix_file_db_id (db_path, id_file);
  if ((GUINT32_FROM_LE (id.magic) != TRACK_FILE_MAGIC) ||
      (GUINT32_FROM_LE (id.version) != TRACK_FILE_VERSION))
    {
      goto exit;
    }

  version = HYSCAN_FIX_TRACK_UNKNOWN;

  sch_file = g_build_filename (track_path, "track.sch", NULL);
  sch_md5 = hyscan_fix_file_md5 (db_path, sch_file);
  if (sch_md5 == NULL)
    goto exit;

  while (version < HYSCAN_FIX_TRACK_LAST)
    {
      if (g_strcmp0 (sch_md5, hyscan_fix_track_get_hash (version)) == 0)
        break;
      version += 1;
    }

  if (version == HYSCAN_FIX_TRACK_LAST)
    version = HYSCAN_FIX_TRACK_UNKNOWN;

exit:
  g_free (sch_md5);
  g_free (sch_file);
  g_free (id_file);

  return version;
}

/**
 * hyscan_fix_track:
 * @db_path: путь к базе данных (каталог с проектами)
 * @track_path: путь к проекту относительно db_path
 * @cancellable: указатель на #HyScanCancellable
 *
 * Функция обновляет формат данных галса. При этом происходит
 * последовательное обновление формата данных от одной версии
 * к другой, до текущей используемой в HyScan.
 *
 * Returns: %TRUE если обновление успешно завершено, иначе %FALSE.
 */
gboolean
hyscan_fix_track (const gchar       *db_path,
                  const gchar       *track_path,
                  HyScanCancellable *cancellable)
{
  HyScanFixTrackVersion version;
  gboolean status = TRUE;

  /* Проверяем состояние базы данных и откатываем изменения
   * в случае ошибки при предыдущем обновлении. */
  if (!hyscan_fix_revert (db_path))
    return FALSE;

  version = hyscan_fix_track_get_version (db_path, track_path);
  switch (version)
    {
    case HYSCAN_FIX_TRACK_NOT_TRACK:
      status = TRUE;
      break;

    case HYSCAN_FIX_TRACK_UNKNOWN:
      status = FALSE;
      break;

    case HYSCAN_FIX_TRACK_2F9C8A44:
      if (status)
        status = hyscan_fix_track_2f9c8a44 (db_path, track_path, cancellable);

    case HYSCAN_FIX_TRACK_19A285F3:
      if (status)
        status = hyscan_fix_track_19a285f3 (db_path, track_path);

    case HYSCAN_FIX_TRACK_9726336A:
      break;

    case HYSCAN_FIX_TRACK_LAST:
      status = FALSE;
      break;
    }

  return status;
}
