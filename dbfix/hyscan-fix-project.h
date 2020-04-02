/* hyscan-fix-project.h
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

#ifndef __HYSCAN_FIX_PROJECT_H__
#define __HYSCAN_FIX_PROJECT_H__

#include <hyscan-types.h>

G_BEGIN_DECLS

typedef enum
{
  HYSCAN_FIX_PROJECT_NOT_PROJECT,
  HYSCAN_FIX_PROJECT_UNKNOWN,
  HYSCAN_FIX_PROJECT_3E65462D,
  HYSCAN_FIX_PROJECT_6190124D,
  HYSCAN_FIX_PROJECT_2C71F69B,
  HYSCAN_FIX_PROJECT_E38FABCF,
  HYSCAN_FIX_PROJECT_3C282D25,
  HYSCAN_FIX_PROJECT_7F9EB90C,
  HYSCAN_FIX_PROJECT_FD8E8922,
  HYSCAN_FIX_PROJECT_DE7491C1,
  HYSCAN_FIX_PROJECT_AD1F40A3,
  HYSCAN_FIX_PROJECT_B288BA04,
  HYSCAN_FIX_PROJECT_LATEST = HYSCAN_FIX_PROJECT_B288BA04,
  HYSCAN_FIX_PROJECT_LAST
} HyScanFixProjectVersion;

HyScanFixProjectVersion  hyscan_fix_project_get_version  (const gchar *db_path,
                                                          const gchar *project_path);

gboolean                 hyscan_fix_project              (const gchar *db_path,
                                                          const gchar *project_path);

G_END_DECLS

#endif /* __HYSCAN_FIX_PROJECT_H__ */
