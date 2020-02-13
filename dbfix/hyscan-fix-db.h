/* hyscan-fix-db.h
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

#ifndef __HYSCAN_FIX_DB_H__
#define __HYSCAN_FIX_DB_H__

#include <hyscan-cancellable.h>

G_BEGIN_DECLS

#define HYSCAN_TYPE_FIX_DB             (hyscan_fix_db_get_type ())
#define HYSCAN_FIX_DB(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), HYSCAN_TYPE_FIX_DB, HyScanFixDB))
#define HYSCAN_IS_FIX_DB(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HYSCAN_TYPE_FIX_DB))
#define HYSCAN_FIX_DB_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), HYSCAN_TYPE_FIX_DB, HyScanFixDBClass))
#define HYSCAN_IS_FIX_DB_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), HYSCAN_TYPE_FIX_DB))
#define HYSCAN_FIX_DB_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), HYSCAN_TYPE_FIX_DB, HyScanFixDBClass))

typedef struct _HyScanFixDB HyScanFixDB;
typedef struct _HyScanFixDBPrivate HyScanFixDBPrivate;
typedef struct _HyScanFixDBClass HyScanFixDBClass;

struct _HyScanFixDB
{
  GObject parent_instance;

  HyScanFixDBPrivate *priv;
};

struct _HyScanFixDBClass
{
  GObjectClass parent_class;
};

HYSCAN_API
GType                  hyscan_fix_db_get_type         (void);

HYSCAN_API
HyScanFixDB *          hyscan_fix_db_new              (void);

HYSCAN_API
void                   hyscan_fix_db_upgrade          (HyScanFixDB        *fix,
                                                       const gchar        *db_path,
                                                       HyScanCancellable  *cancellable);

HYSCAN_API
gboolean               hyscan_fix_db_complete         (HyScanFixDB        *fix);

G_END_DECLS

#endif /* __HYSCAN_FIX_DB_H__ */
