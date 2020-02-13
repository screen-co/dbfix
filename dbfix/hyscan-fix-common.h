/* hyscan-fix-common.h
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

#ifndef __HYSCAN_FIX_COMMON_H__
#define __HYSCAN_FIX_COMMON_H__

#include <glib.h>

G_BEGIN_DECLS

typedef struct _HyScanFixFileIDType HyScanFixFileIDType;
struct _HyScanFixFileIDType
{
  guint32      magic;
  guint32      version;
  guint64      dt;
};

gchar *                hyscan_fix_id_create        (void);

gchar **               hyscan_fix_dir_list         (const gchar   *path);

gboolean               hyscan_fix_file_exist       (const gchar   *db_path,
                                                    const gchar   *file_path);

gchar *                hyscan_fix_file_md5         (const gchar   *db_path,
                                                    const gchar   *file_path);

HyScanFixFileIDType    hyscan_fix_file_db_id       (const gchar   *db_path,
                                                    const gchar   *file_path);

gboolean               hyscan_fix_file_append      (const gchar   *db_path,
                                                    const gchar   *file_path,
                                                    const gchar   *str);

gboolean               hyscan_fix_file_backup      (const gchar   *db_path,
                                                    const gchar   *file_path,
                                                    gboolean       exist);

gboolean               hyscan_fix_file_copy        (const gchar   *db_path,
                                                    const gchar   *src_path,
                                                    const gchar   *dst_path,
                                                    gboolean       exist);

gboolean               hyscan_fix_file_mark_remove (const gchar   *db_path,
                                                    const gchar   *file_path);

gboolean               hyscan_fix_file_schema      (const gchar   *db_path,
                                                    const gchar   *file_path,
                                                    const gchar   *schema_version);

gboolean               hyscan_fix_log              (const gchar   *db_path,
                                                    const gchar   *format,
                                                    ...) G_GNUC_PRINTF (1, 3);

gboolean               hyscan_fix_cleanup          (const gchar   *db_path);

gboolean               hyscan_fix_revert           (const gchar   *db_path);

G_END_DECLS

#endif /* __HYSCAN_FIX_COMMON_H__ */
