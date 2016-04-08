/*
 * (C) Copyright 2015 Kurento (http://kurento.org/)
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 */

#include "ResourceManager.hpp"
#include <gst/gst.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <KurentoException.hpp>
#include <MediaSet.hpp>

#include <sys/resource.h>

#define GST_CAT_DEFAULT kurento_resource_manager
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoResourceManager"

namespace kurento
{

static int maxOpenFiles = 0;
static int maxThreads = 0;

static int
get_int (std::string &str, char sep, int nToken)
{
  size_t start = str.find_first_not_of (sep), end;
  int count = 0;

  while (start != std::string::npos) {
    end = str.find (sep, start);

    if (count++ == nToken) {
      str[end] = '\0';
      return atoi (&str.c_str() [start]);
    }

    start = str.find_first_not_of (sep, end);
  }

  return 0;
}

static int
getNumberOfThreads ()
{
  std::string stat;
  std::ifstream stat_file ("/proc/self/stat");

  std::getline (stat_file, stat);
  stat_file.close();

  return get_int (stat, ' ', 19);
}

static int
getMaxThreads ()
{
  if (maxThreads == 0) {
    struct rlimit limits;
    getrlimit (RLIMIT_NPROC, &limits);

    maxThreads = limits.rlim_cur;
  }

  return maxThreads;
}

static void
checkThreads (float limit_percent)
{
  int nThreads;

  nThreads = getNumberOfThreads ();

  if (nThreads > getMaxThreads () * limit_percent ) {
    throw KurentoException (NOT_ENOUGH_RESOURCES, "Too many threads");
  }
}

static int
getMaxOpenFiles ()
{
  if (maxOpenFiles == 0) {
    struct rlimit limits;
    getrlimit (RLIMIT_NOFILE, &limits);

    maxOpenFiles = limits.rlim_cur;
  }

  return maxOpenFiles;
}

static int
getNumberOfOpenFiles ()
{
  int openFiles = 0;
  DIR *d;
  struct dirent *dir;

  d = opendir ("/proc/self/fd");

  while ( (dir = readdir (d) ) != NULL) {
    openFiles ++;
  }

  closedir (d);

  return openFiles;
}

static void
checkOpenFiles (float limit_percent)
{
  int nOpenFiles;

  nOpenFiles = getNumberOfOpenFiles ();

  if (nOpenFiles > getMaxOpenFiles () * limit_percent ) {
    throw KurentoException (NOT_ENOUGH_RESOURCES, "Too many open files");
  }
}

void
checkResources (float limit_percent)
{
  checkThreads (limit_percent);
  checkOpenFiles (limit_percent);
}

void killServerOnLowResources (float limit_percent)
{
  MediaSet::getMediaSet()->signalEmptyLocked.connect ([limit_percent] () {
    GST_DEBUG ("MediaSet empty, checking resources");

    try {
      checkResources (limit_percent);
    } catch (KurentoException &e) {
      if (e.getCode() == NOT_ENOUGH_RESOURCES) {
        GST_ERROR ("Resources over the limit, server will be killed");
        kill ( getpid(), SIGTERM );
      }
    }
  });
}

} /* kurento */

static void init_debug (void) __attribute__ ( (constructor) );

static void
init_debug (void)
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}
