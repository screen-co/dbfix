/* hyscan-fix-track.h
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

#ifndef __HYSCAN_FIX_TRACK_H__
#define __HYSCAN_FIX_TRACK_H__

#include <hyscan-cancellable.h>

G_BEGIN_DECLS

typedef enum
{
  HYSCAN_FIX_TRACK_NOT_TRACK,
  HYSCAN_FIX_TRACK_UNKNOWN,
  HYSCAN_FIX_TRACK_2F9C8A44,
  HYSCAN_FIX_TRACK_19A285F3,
  HYSCAN_FIX_TRACK_9726336A,
  HYSCAN_FIX_TRACK_E8B616CC,
  HYSCAN_FIX_TRACK_423880D1,
  HYSCAN_FIX_TRACK_49A23606,
  HYSCAN_FIX_TRACK_LATEST = HYSCAN_FIX_TRACK_49A23606,
  HYSCAN_FIX_TRACK_LAST
} HyScanFixTrackVersion;

HyScanFixTrackVersion  hyscan_fix_track_get_version   (const gchar        *db_path,
                                                       const gchar        *track_path);

gboolean               hyscan_fix_track               (const gchar        *db_path,
                                                       const gchar        *track_path,
                                                       HyScanCancellable  *cancellable);

G_END_DECLS

#endif /* __HYSCAN_FIX_TRACK_H__ */
