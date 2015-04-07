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

#include <sys/resource.h>

#define GST_CAT_DEFAULT kurento_resource_manager
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoResourceManager"

namespace kurento
{

static const float LIMIT_PERCENT = 0.8;

static int maxOpenFiles = 0;
static int maxThreads = 0;

static void
tokenize (std::string str, char sep, std::vector<std::string> &tokens)
{
  size_t start = str.find_first_not_of (sep), end = start;

  while (start != std::string::npos) {
    end = str.find (sep, start);
    tokens.push_back (str.substr (start, end - start) );
    start = str.find_first_not_of (sep, end);
  }
}

static int
getNumberOfThreads ()
{
  std::string stat;
  std::ifstream stat_file ("/proc/self/stat");
  std::vector <std::string> tokens;

  std::getline (stat_file, stat);
  tokenize (stat, ' ', tokens);

  stat_file.close();

  return atoi (tokens[19].c_str() );
}

static int
getMaxThreads ()
{
  if (maxThreads == 0) {
    struct rlimit limits;
    getrlimit (RLIMIT_NPROC, &limits);

    maxThreads = limits.rlim_cur * LIMIT_PERCENT;
  }

  return maxThreads;
}

static void
checkThreads ()
{
  int nThreads;

  nThreads = getNumberOfThreads ();

  if (nThreads > getMaxThreads () ) {
    throw KurentoException (NOT_ENOUGH_RESOURCES, "Too many threads");
  }
}

static int
getMaxOpenFiles ()
{
  if (maxOpenFiles == 0) {
    struct rlimit limits;
    getrlimit (RLIMIT_NOFILE, &limits);

    maxOpenFiles = limits.rlim_cur * LIMIT_PERCENT;
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
checkOpenFiles ()
{
  int nOpenFiles;

  nOpenFiles = getNumberOfOpenFiles ();

  if (nOpenFiles > getMaxOpenFiles () ) {
    throw KurentoException (NOT_ENOUGH_RESOURCES, "Too many open files");
  }
}

void
checkResources (void)
{
  checkThreads ();
  checkOpenFiles ();
}

} /* kurento */

static void init_debug (void) __attribute__ ( (constructor) );

static void
init_debug (void)
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}
