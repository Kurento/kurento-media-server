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

#include <sstream>
#include <thread>
#include <sys/time.h>

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
                           const std::string &path)
{
  sink->locked_backend()->set_file_collector (sinks::file::make_collector (
        keywords::target = path,
        keywords::max_size = 16 * 1024 * 1024,
        keywords::min_free_space = 100 * 1024 * 1024
      ) );
}

static std::string
severity_to_string (severity_level level)
{
  switch (level) {
  case error:
    return "  error";

  case warning:
    return "warning";

  case fixme:
    return "  fixme";

  case info:
    return "   info";

  case debug:
    return "  debug";

  case log:
    return "    log";

  case trace:
    return "  trace";

  default:
    return "unknown";
  }
}

static void
system_formatter (logging::record_view const &rec,
                  logging::formatting_ostream &strm)
{
  logging::value_ref< severity_level > val =
    logging::extract< severity_level > ("Severity", rec);
  auto date_time_formatter = expr::stream
                             << expr::format_date_time< boost::posix_time::ptime > ("TimeStamp",
                                 "%Y-%m-%d %H:%M:%S,%f");

  date_time_formatter (rec, strm) << " ";
  strm << logging::extract< attrs::current_process_id::value_type > ("ProcessID",
       rec) << " ";
  strm << "[" <<
       logging::extract< attrs::current_thread_id::value_type > ("ThreadID",
           rec) << "] ";
  strm << severity_to_string (val.get() ) << " ";
  strm << logging::extract< std::string > ("Category", rec) << " ";
  strm << logging::extract< std::string > ("FileName", rec) << ":";
  strm << logging::extract< int > ("Line", rec) << " ";
  strm << logging::extract< std::string > ("Function", rec) << "() ";
  strm << logging::extract< std::string > ("GObject", rec) << " ";
  strm << rec[expr::smessage];
}

bool
kms_init_logging (const std::string &path)
{
  gst_debug_remove_log_function_by_data (NULL);
  gst_debug_add_log_function (kms_log_function, NULL, NULL);

  boost::shared_ptr< logging::core > core = logging::core::get();

  boost::shared_ptr< sinks::text_file_backend > backend =
    boost::make_shared< sinks::text_file_backend > (
      keywords::file_name = path + "/" + "%Y%m%d_%H%M%S_%5N.log",
      keywords::rotation_size = 5 * 1024 * 1024,
      keywords::time_based_rotation = sinks::file::rotation_at_time_point (12, 0, 0)
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
  init_file_collecting (system_sink, path + "/logs");

  /* Upon restart, scan the directory for files matching the file_name pattern */
  system_sink->locked_backend()->scan_for_files();

  core->add_sink (system_sink);

  system_sink->set_formatter (&system_formatter);

  return true;
}

}  /* kurento */
