/*
 * memory.cpp - Kurento Media Server
 *
 * Copyright (C) 2013 Kurento
 * Contact: Miguel París Díaz <mparisdiaz@gmail.com>
 * Contact: José Antonio Santos Cadenas <santoscadenas@kurento.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include <string.h>
#include <stdlib.h>

#define MAX_PROC_FILE_LEN 20

int
get_data_memory (int pid)
{
  int mem;

  gchar proc_file[MAX_PROC_FILE_LEN];
  GIOChannel *chan;
  GError *err = NULL;
  GIOStatus st;
  char *line = NULL;
  char *vmdata = NULL;
  size_t len;

  g_snprintf (proc_file, MAX_PROC_FILE_LEN, "%s%d%s", "/proc/", pid, "/status");
  chan = g_io_channel_new_file (proc_file, "r", &err);

  if (chan == NULL && err != NULL) {
    g_printerr ("%s:%d: %s", __FILE__, __LINE__, err->message);
    g_error_free (err);
  }

  g_assert (chan != NULL);

  /* Read memory size data from /proc/self/status */
  while (!vmdata) {
    st = g_io_channel_read_line (chan, &line, &len, NULL, &err);

    if (st != G_IO_STATUS_NORMAL && err != NULL) {
      g_printerr ("%s:%d: %s", __FILE__, __LINE__, err->message);
      g_error_free (err);
    }

    g_assert (st == G_IO_STATUS_NORMAL);

    if (!strncmp (line, "VmRSS:", 6) ) {
      vmdata = g_strdup (&line[6]);
    }

    g_free (line);
  }

  g_io_channel_shutdown (chan, FALSE, NULL);
  g_io_channel_unref (chan);

  /* Get rid of " kB\n"*/
  len = strlen (vmdata);
  vmdata[len - 4] = 0;

  mem = atoi (vmdata);

  g_free (vmdata);

  return mem;
}
