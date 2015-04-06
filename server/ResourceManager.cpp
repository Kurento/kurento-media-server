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

#define GST_CAT_DEFAULT kurento_resource_manager
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoResourceManager"

namespace kurento
{

static int maxThreads = 500;

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

void
checkResources (void)
{
  int nThreads;

  nThreads = getNumberOfThreads ();

  if (nThreads > maxThreads) {
    throw KurentoException (NOT_ENOUGH_RESOURCES, "Too many threads");
  }
}

} /* kurento */

static void init_debug (void) __attribute__ ( (constructor) );

static void
init_debug (void)
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}
