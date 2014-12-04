/*
 * (C) Copyright 2014 Kurento (http://kurento.org/)
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

#include "logging.hpp"

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <sstream>
#include <thread>
#include <sys/time.h>

namespace kurento
{

static std::string
debug_object (GObject *object)
{
  if (object == NULL) {
    return "";
  }

  if (GST_IS_PAD (object) && GST_OBJECT_NAME (object) ) {
    boost::format fmt ("<%s:%s> ");
    fmt % (GST_OBJECT_PARENT (object) != NULL ? GST_OBJECT_NAME (GST_OBJECT_PARENT (
             object) ) : "''") % GST_OBJECT_NAME (object);
    return fmt.str();
  }

  if (GST_IS_OBJECT (object) && GST_OBJECT_NAME (object) ) {
    boost::format fmt ("<%s> ");
    fmt % GST_OBJECT_NAME (object);
    return fmt.str();
  }

  if (G_IS_OBJECT (object) ) {
    boost::format fmt ("<%s@%p> ");
    fmt %  G_OBJECT_TYPE_NAME (object) % object;
    return fmt.str();
  }

  boost::format fmt ("<%p> ");
  fmt % object;
  return fmt.str();
}

static std::string
expand_string (std::string str, int len)
{
  std::string ret = str;
  int sp = len - str.size ();

  for (int i = sp; i > 0; i--) {
    str.append (" ");
  }

  return str;
}

#define LOG expand_string(category->name, 25) << " " <<  \
    boost::filesystem::path(file).filename().string() \
    << ":" << line << " " << function << "() " << \
    debug_object(object) << \
    gst_debug_message_get(message)

static std::string
getDateTime ()
{
  timeval curTime;
  struct tm *timeinfo;
  char buffer[22];

  gettimeofday (&curTime, NULL);;
  timeinfo = localtime (&curTime.tv_sec);

  strftime (buffer, 22, "%Y-%m-%d %H:%M:%S", timeinfo);
  std::string datetime (buffer);
  datetime.append (".");

  std::string micros = std::to_string (curTime.tv_usec);

  while (micros.size() < 6) {
    micros = "0" + micros;
  }

  datetime.append (micros);
  return datetime;
}

void
simple_log_function (GstDebugCategory *category, GstDebugLevel level,
                     const gchar *file,
                     const gchar *function, gint line, GObject *object,
                     GstDebugMessage *message, gpointer user_data)
{
  std::stringstream ss;

  if (level > gst_debug_category_get_threshold (category) ) {
    return;
  }

  ss << getDateTime() << " ";
  ss << getpid() << " ";
  ss << "[" << std::this_thread::get_id () << "]";

  switch (level) {
  case GST_LEVEL_ERROR:
    ss << "   error ";
    break;

  case GST_LEVEL_WARNING:
    ss << " warning ";
    break;

  case GST_LEVEL_FIXME:
    ss << "   fixme ";
    break;

  case GST_LEVEL_INFO:
    ss << "    info ";
    break;

  case GST_LEVEL_DEBUG:
    ss << "   debug ";
    break;

  case GST_LEVEL_LOG:
    ss << "     log ";
    break;

  case GST_LEVEL_TRACE:
    ss << "   trace ";
    break;

  default:
    return;
  }

  ss << LOG << std::endl;

  std::cout << ss.str();
  std::cout.flush ();
}

}  /* kurento */
