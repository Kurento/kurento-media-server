/*
 * server_test_base.cpp - Kurento Media Server
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

#include "server_test_base.hpp"

#include <boost/test/unit_test.hpp>

#include <unistd.h>
#include <sys/wait.h>
#include <glib.h>

G_LOCK_DEFINE (mutex);

static void
sig_handler (int sig)
{
  if (sig == SIGCONT)
    G_UNLOCK (mutex);
}

int
start_server_test ()
{
  pid_t childpid = -1;

  signal (SIGCONT, sig_handler);

  G_LOCK (mutex);
  childpid = fork();

  if (childpid >= 0) {
    if (childpid == 0) {
      execl ("./server/kurento", "kurento", NULL);
    } else {
      G_LOCK (mutex);
      G_UNLOCK (mutex);
    }
  } else {
    BOOST_FAIL ("Error executing server");
  }

  return childpid;
}

void
stop_server_test (int pid)
{
  int status;

  kill (pid, SIGINT);
  wait (&status);
}
