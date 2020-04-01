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

#include "hyscan-fix-assistant.h"
#include <hyscan-config.h>
#include <glib/gi18n.h>

#ifdef G_OS_WIN32
#include <Windows.h>
#endif

int
main (int    argc,
      char **argv)
{
  GtkWidget *assistant;

  setlocale (LC_ALL, "");
  bindtextdomain (GETTEXT_PACKAGE, hyscan_config_get_locale_dir ());
  bindtextdomain ("libhyscangtk", hyscan_config_get_locale_dir());
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  bind_textdomain_codeset ("libhyscangtk" , "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  gtk_init (&argc, &argv);

  assistant = hyscan_fix_assistant_new (hyscan_config_get_profile_dirs ());

  gtk_widget_show_all (assistant);

  gtk_main ();

  gtk_widget_destroy (assistant);

  return 0;
}

#ifdef G_OS_WIN32
int WINAPI
wWinMain (HINSTANCE hInst,
          HINSTANCE hPreInst,
          LPWSTR    lpCmdLine,
          int       nCmdShow)
{
  int argc;
  char **argv;

  gint result;

  argv = g_win32_get_command_line ();
  argc = g_strv_length (argv);

  result = main (argc, argv);

  g_strfreev (argv);

  return result;
}
#endif
