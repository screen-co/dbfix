/* dbfix-cli.c
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

#include "hyscan-fix-db.h"

G_LOCK_DEFINE (lock);
gboolean done = FALSE;
gchar *message = NULL;

void
log_message (HyScanFixDB *fix,
             const gchar *log_message)
{
  G_LOCK (lock);
  g_clear_pointer (&message, g_free);
  message = g_strdup (log_message);
  G_UNLOCK (lock);
}

void
completed (HyScanFixDB *fix,
           gboolean     status,
           gboolean    *done)
{
  g_atomic_int_set (done, TRUE);
}

void
clear (guint size)
{
  gchar *str = g_malloc (size + 1);

  memset (str, ' ', size);
  str[size] = 0;

  g_print ("\r%s\r", str);

  g_free (str);
}

int
main (int    argc,
      char **argv)
{
  HyScanFixDB *fix;
  HyScanCancellable *cancellable;
  guint size = 0;

  if (argc != 2)
    {
      g_print ("Usage: dbfix-cli <db-path>\r\n\r\n");
      return 0;
    }

  fix = hyscan_fix_db_new ();
  g_signal_connect (fix, "log", G_CALLBACK (log_message), NULL);
  g_signal_connect (fix, "completed", G_CALLBACK (completed), &done);

  cancellable = hyscan_cancellable_new ();
  hyscan_fix_db_upgrade (fix, argv[1], cancellable);

  while (!g_atomic_int_get (&done))
    {
      gchar *out_message;

      G_LOCK (lock);

      clear (size);
      out_message = g_strdup_printf ("[%3d%%]: %s",
                                     (guint)(100.0 * hyscan_cancellable_get (cancellable)),
                                     (message != NULL) ? message : "");
      g_print ("%s", out_message);
      size = strlen (out_message);
      g_free (out_message);

      G_UNLOCK (lock);

      g_usleep (100000);
    }

  clear (size);
  if (hyscan_fix_db_complete (fix))
    g_print ("Completed\r\n");
  else
    g_print ("%s\r\n", message);

  g_object_unref (cancellable);
  g_object_unref (fix);
}
