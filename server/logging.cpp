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

#include "logging.hpp"

#include <gst/gst.h>

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
#include <boost/log/utility/exception_handler.hpp>
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

// ----------------------------------------------------------------------------

GST_DEBUG_CATEGORY_STATIC (kms_glib_debug);

static GstDebugLevel
g_log_level_to_gst_debug_level (GLogLevelFlags log_level)
{
  switch (log_level & G_LOG_LEVEL_MASK) {
  case G_LOG_LEVEL_ERROR:
    return GST_LEVEL_ERROR;

  case G_LOG_LEVEL_CRITICAL:
    return GST_LEVEL_ERROR;

  case G_LOG_LEVEL_WARNING:
    return GST_LEVEL_WARNING;

  case G_LOG_LEVEL_MESSAGE:
    return GST_LEVEL_FIXME;

  case G_LOG_LEVEL_INFO:
    return GST_LEVEL_INFO;

  case G_LOG_LEVEL_DEBUG:
    return GST_LEVEL_DEBUG;

  default:
    return GST_LEVEL_DEBUG;
  }
}

/*
 * Based on the Glib 2.48.2 default message handler, g_log_default_handler()
 * https://github.com/GNOME/glib/blob/2.48.2/glib/gmessages.c#L1429
 *
 * NOTE: In Glib 2.50 they added a new logging mode, "structured logging", that
 * maybe we need to check out in the future.
 */
/* these are emitted by the default log handler */
#define DEFAULT_LEVELS (G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING | G_LOG_LEVEL_MESSAGE)
/* these are filtered by G_MESSAGES_DEBUG by the default log handler */
#define INFO_LEVELS (G_LOG_LEVEL_INFO | G_LOG_LEVEL_DEBUG)
static void
kms_glib_log_handler (const gchar   *log_domain,
                      GLogLevelFlags log_level,
                      const gchar   *message,
                      gpointer       unused_data)
{
  const gchar *domains;

  if ( (log_level & DEFAULT_LEVELS) || (log_level >> G_LOG_LEVEL_USER_SHIFT) ) {
    goto emit;
  }

  domains = g_getenv ("G_MESSAGES_DEBUG");

  if ( ( (log_level & INFO_LEVELS) == 0) ||
       domains == NULL ||
       (strcmp (domains, "all") != 0 && (!log_domain
                                         || !strstr (domains, log_domain) ) ) ) {
    return;
  }

emit:

  /* we can be called externally with recursion for whatever reason */
  if (log_level & G_LOG_FLAG_RECURSION) {
    //_g_log_fallback_handler (log_domain, log_level, message, unused_data);
    return;
  }

  // Forward Glib log messages through GStreamer logging
  GstDebugCategory *category = kms_glib_debug;
  GstDebugLevel level = g_log_level_to_gst_debug_level (log_level);
  gst_debug_log (category, level, GST_STR_NULL (log_domain), "", 0, NULL,
                 "%s", GST_STR_NULL (message) );
}

void
kms_init_logging ()
{
  // Forward Glib log messages through GStreamer logging
  GST_DEBUG_CATEGORY_INIT (kms_glib_debug, "glib", 0, "Glib logging");
  g_log_set_default_handler (kms_glib_log_handler, NULL);
}

// ----------------------------------------------------------------------------

/* kurento logging sinks */
boost::shared_ptr< sink_t > system_sink;

static std::string
debug_object (GObject *object)
{
  if (object == nullptr) {
    return "";
  }

  if (GST_IS_PAD (object) && GST_OBJECT_NAME (object) ) {
    boost::format fmt ("<%s:%s> ");
    fmt %
    (GST_OBJECT_PARENT (object) != nullptr
     ? GST_OBJECT_NAME (GST_OBJECT_PARENT (object) )
     : "''") %
    GST_OBJECT_NAME (object);
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

static const char *
gst_debug_level_to_cee_pri_level (GstDebugLevel level)
{
  // CEE only permits four values in the "pri" field
  switch (level) {
  case GST_LEVEL_NONE:
    return "";

  case GST_LEVEL_ERROR:
    return "ERROR";

  case GST_LEVEL_WARNING:
    return "WARN";

  case GST_LEVEL_INFO:
  case GST_LEVEL_DEBUG:
  case GST_LEVEL_LOG:
  case GST_LEVEL_FIXME:
  case GST_LEVEL_TRACE:
  case GST_LEVEL_MEMDUMP:
    return "DEBUG";

  default:
    return "";
  }
}

static void
kms_log_function (GstDebugCategory *category, GstDebugLevel level,
                  const gchar *file,
                  const gchar *function, gint line, GObject *object,
                  GstDebugMessage *message, gpointer user_data) G_GNUC_NO_INSTRUMENT;

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
      logging::add_value ("CEE_pri", gst_debug_level_to_cee_pri_level (level) ) <<
      logging::add_value ("TimeStampUTC",
                          boost::date_time::microsec_clock<boost::posix_time::ptime>::universal_time() )
      <<
      logging::add_value ("Category", expand_string (category->name,
                          25) ) << // FIXME, should be expanded by the formatter
      logging::add_value ("CategoryRaw", category->name) <<
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
                                 "%Y-%m-%dT%H:%M:%S,%f");
  date_time_formatter (rec, strm) << " ";
  strm << std::to_string (getpid() ) << " ";
  strm << logging::extract< attrs::current_thread_id::value_type > ("ThreadID",
       rec) << " ";
  strm << logging::extract< severity_level > ("Severity", rec) << " ";
  strm << logging::extract< std::string > ("Category", rec) << " ";
  strm << logging::extract< std::string > ("FileName", rec) << ":";
  strm << logging::extract< int > ("Line", rec) << " ";
  strm << logging::extract< std::string > ("Function", rec) << "() ";
  strm << logging::extract< std::string > ("GObject", rec) << " ";
  strm << rec[expr::smessage];
}

static void
json_cee_formatter (logging::record_view const &rec,
                    logging::formatting_ostream &strm)
{
  static const char *json_special = "\"\\/\b\f\n\r\t";
  static const char *json_special_replace = "\"\\/bfnrt";

  std::stringstream tmp;

  auto date_time_formatter = expr::stream
                             << expr::format_date_time< boost::posix_time::ptime > ("TimeStampUTC",
                                 "%Y-%m-%dT%H:%M:%S.%fZ");
  strm << "{\"time\":\"";
  date_time_formatter (rec, strm) << "\",";

  /* ProcessID is usually a hex string, JSON CEE expects a decimal value */
  tmp << logging::extract< attrs::current_process_id::value_type > ("ProcessID",
      rec);
  tmp.flush();
  strm << "\"proc!id\":";
  strm << strtoul (tmp.str().c_str(), 0, 0) << ",";
  tmp.str ("");
  tmp.clear();

  /* ThreadID is usually a hex string, JSON CEE expects a decimal value */
  tmp << logging::extract< attrs::current_thread_id::value_type > ("ThreadID",
      rec);
  tmp.flush();
  strm << "\"proc!tid\":";
  strm << strtoul (tmp.str().c_str(), 0, 0) << ",";
  tmp.str ("");
  tmp.clear();

  strm << "\"pname\":\"";
  strm << logging::extract< std::string > ("ProcessName", rec) << "\",";

  strm << "\"appname\":\"";
  strm << logging::extract< std::string > ("AppName", rec) << "\",";

  strm << "\"host\":\"";
  strm << logging::extract< std::string > ("FQDN", rec) << "\",";

  strm << "\"pri\":\"";
  strm << logging::extract< std::string > ("CEE_pri", rec) << "\",";

  strm << "\"subsys\":\"";
  strm << logging::extract< std::string > ("CategoryRaw", rec) << "\",";

  strm << "\"file!name\":\"";
  strm << logging::extract< std::string > ("FileName", rec) << "\",";
  strm << "\"file!line\":";
  strm << logging::extract< int > ("Line", rec) << ",";
  strm << "\"native!function\":\"";
  strm << logging::extract< std::string > ("Function", rec) << "\",";

  strm << "\"native!object\":\"";
  strm << logging::extract< std::string > ("GObject", rec) << "\",";

  strm << "\"msg\":\"";
  tmp << rec[expr::smessage];
  tmp.flush();
  const std::string &msg = tmp.str();

  for (std::string::const_iterator it = msg.begin(); it != msg.end(); it++) {
    char c = (char) * it;
    const char *special = strchr (json_special, c);

    if (special != NULL) {
      const char *replace = json_special_replace + (special - json_special);
      strm << '\\' << *replace;
    } else if (c < 0x20) {
      /* Everything below 0x20 must be escaped */
      strm << "\\u00" << std::setw (2) << std::setfill ('0') << std::hex <<
           std::uppercase << c;
    } else {
      strm << c;
    }
  }

  strm << "\"";

  strm << "}";
}

bool
kms_init_logging_files (const std::string &path, int fileSize, int fileNumber)
{
  gst_debug_remove_log_function (gst_debug_log_default);
  gst_debug_add_log_function (kms_log_function, nullptr, nullptr);

  boost::shared_ptr< logging::core > core = logging::core::get();

  boost::shared_ptr< sinks::text_file_backend > backend =
    boost::make_shared< sinks::text_file_backend > (
      keywords::file_name = path + "/" + "%Y-%m-%dT%H%M%S.%5N.pid" +
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
  init_file_collecting (system_sink, path, fileSize, fileNumber);

  /* Upon restart, scan the directory for files matching the file_name pattern */
  system_sink->locked_backend()->scan_for_files();

  core->add_sink (system_sink);

  bool structured_json_cee = false;
  const char *g_log_structured = getenv ("GST_DEBUG_STRUCTURED");

  if ( g_log_structured != NULL ) {
    if (strstr (g_log_structured, "JSON_CEE") ) {
      structured_json_cee = true;
    }
  }

  if (structured_json_cee) {
    system_sink->set_formatter (&json_cee_formatter);
  } else {
    system_sink->set_formatter (&system_formatter);
  }

  /* Set an exception handler to manage error cases such as missing
   * file write permissions */
  struct ex_handler {
    void operator() (std::runtime_error const &e) const
    {
      gst_debug_remove_log_function (kms_log_function);
      gst_debug_add_log_function (gst_debug_log_default, nullptr, nullptr);
      GST_ERROR ("Boost.Log runtime error: %s", e.what() );
    }
  };
  core->set_exception_handler (logging::make_exception_handler <
                               std::runtime_error > (ex_handler() ) );

  return true;
}

// ----------------------------------------------------------------------------

}  /* kurento */
