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
#include <boost/log/trivial.hpp>

#include <sstream>

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

#define LOG category->name << " " <<  \
                           boost::filesystem::path(file).filename().string() \
                           << ":" << line << " " << function << "() " << \
                           debug_object(object) << \
                           gst_debug_message_get(message)

void
log_function (GstDebugCategory *category, GstDebugLevel level,
              const gchar *file,
              const gchar *function, gint line, GObject *object,
              GstDebugMessage *message, gpointer user_data)
{
  if (level > gst_debug_category_get_threshold (category) ) {
    return;
  }

  switch (level) {
  case GST_LEVEL_ERROR:
    BOOST_LOG_TRIVIAL (error) << LOG;
    break;

  case GST_LEVEL_WARNING:
    BOOST_LOG_TRIVIAL (warning) << LOG;
    break;

  case GST_LEVEL_FIXME:
  case GST_LEVEL_INFO:
    BOOST_LOG_TRIVIAL (info) << LOG;
    break;

  case GST_LEVEL_DEBUG:
    BOOST_LOG_TRIVIAL (debug) << LOG;
    break;

  case GST_LEVEL_LOG:
  case GST_LEVEL_TRACE:
    BOOST_LOG_TRIVIAL (trace) << LOG;
    break;

  default:
    break;
  }
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

  ss << LOG;

  std::cout << ss.str() << std::endl;
}

}  /* kurento */
