/*
 * (C) Copyright 2014 Kurento (http://kurento.org/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <gst/gst.h>

#include "logging.hpp"

#include <boost/format.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sources/channel_feature.hpp>
#include <boost/log/sources/channel_logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <sys/types.h>
#include <unistd.h>


namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

typedef sinks::synchronous_sink< sinks::text_file_backend > sink_t;

namespace kurento
{

/* kurento logging sinks */
boost::shared_ptr< sink_t > system_sink;

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

static severity_level
gst_debug_level_to_severity_level (GstDebugLevel level)
{
  switch (level) {
  case GST_LEVEL_ERROR:
    return error;

  case GST_LEVEL_WARNING:
    return warning;

  case GST_LEVEL_FIXME:
    return fixme;

  case GST_LEVEL_INFO:
    return info;

  case GST_LEVEL_DEBUG:
    return debug;

  case GST_LEVEL_LOG:
    return log;

  case GST_LEVEL_TRACE:
    return trace;

  default:
    return undefined;
  }
}

static void
kms_log_function (GstDebugCategory *category, GstDebugLevel level,
                  const gchar *file,
                  const gchar *function, gint line, GObject *object,
                  GstDebugMessage *message, gpointer user_data)
{
  if (level > gst_debug_category_get_threshold (category) ) {
    return;
  }

  severity_level severity = gst_debug_level_to_severity_level (level);

  if (severity == undefined) {
    return;
  }

  BOOST_LOG_SEV (system_logger::get(), severity) <<
      logging::add_value ("Category", expand_string (category->name, 25) ) <<
      logging::add_value ("FileName",
                          boost::filesystem::path (file).filename().string() ) <<
      logging::add_value ("Line", line) <<
      logging::add_value ("Function", function) <<
      logging::add_value ("GObject", debug_object (object) ) <<
      gst_debug_message_get (message);
}

void init_file_collecting (boost::shared_ptr< sink_t > sink,
                           const std::string &path,
                           int fileSize,
                           int fileNumber)
{
  sink->locked_backend()->set_file_collector (sinks::file::make_collector (
        keywords::target = path,
        keywords::max_size = (fileSize * fileNumber) * 1024 * 1024,
        keywords::min_free_space = fileSize * 1024 * 1024
      ) );
}

/* The formatting logic for the severity level */
template< typename CharT, typename TraitsT >
inline std::basic_ostream< CharT, TraitsT > &operator<< (
  std::basic_ostream< CharT, TraitsT > &strm, severity_level lvl)
{
  static const char *const str[] = {
    "  error",
    "warning",
    "  fixme",
    "   info",
    "  debug",
    "    log",
    "  trace",
    "unknown"
  };

  if (static_cast< std::size_t > (lvl) < (sizeof (str) / sizeof (*str) ) ) {
    strm << str[lvl];
  } else {
    strm << static_cast< int > (lvl);
  }

  return strm;
}

static void
system_formatter (logging::record_view const &rec,
                  logging::formatting_ostream &strm)
{
  auto date_time_formatter = expr::stream
                             << expr::format_date_time< boost::posix_time::ptime > ("TimeStamp",
                                 "%Y-%m-%d %H:%M:%S,%f");
  date_time_formatter (rec, strm) << " ";
  strm << std::to_string (getpid() ) << " ";
  strm << "[" <<
       logging::extract< attrs::current_thread_id::value_type > ("ThreadID",
           rec) << "] ";
  strm << logging::extract< severity_level > ("Severity", rec) << " ";
  strm << logging::extract< std::string > ("Category", rec) << " ";
  strm << logging::extract< std::string > ("FileName", rec) << ":";
  strm << logging::extract< int > ("Line", rec) << " ";
  strm << logging::extract< std::string > ("Function", rec) << "() ";
  strm << logging::extract< std::string > ("GObject", rec) << " ";
  strm << rec[expr::smessage];
}

bool
kms_init_logging (const std::string &path, int fileSize, int fileNumber)
{
  gst_debug_remove_log_function (gst_debug_log_default);
  gst_debug_add_log_function (kms_log_function, NULL, NULL);

  boost::shared_ptr< logging::core > core = logging::core::get();

  boost::shared_ptr< sinks::text_file_backend > backend =
    boost::make_shared< sinks::text_file_backend > (
      keywords::file_name = path + "/" + "media-server_%Y-%m-%d_%H-%M-%S.%5N.pid" +
                            std::to_string (getpid() ) + ".log",
      keywords::rotation_size = fileSize * 1024 * 1024,
      keywords::time_based_rotation = sinks::file::rotation_at_time_point (0, 0, 0)
    );

  /* Enable auto-flushing after each log record written */
  backend->auto_flush (true);

  /* Wrap it into the frontend and register in the core.   */
  /* The backend requires synchronization in the frontend. */
  system_sink = boost::shared_ptr< sink_t > (new sink_t (backend) );

  /*Set up filter to pass only records that have the necessary attributes */
  system_sink->set_filter (
    expr::has_attr< std::string > ("Channel") &&
    expr::attr< std::string > ("Channel") == "system"
  );

  /* Set up where the rotated files will be stored */
  init_file_collecting (system_sink, path + "/logs", fileSize, fileNumber);

  /* Upon restart, scan the directory for files matching the file_name pattern */
  system_sink->locked_backend()->scan_for_files();

  core->add_sink (system_sink);

  system_sink->set_formatter (&system_formatter);

  return true;
}

}  /* kurento */
