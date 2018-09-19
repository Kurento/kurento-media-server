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

#ifndef __KURENTO_LOGGING_HPP__
#define __KURENTO_LOGGING_HPP__

#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>

namespace src = boost::log::sources;
namespace keywords = boost::log::keywords;

namespace kurento
{

enum severity_level {
  error,
  warning,
  fixme,
  info,
  debug,
  log,
  trace,
  undefined
};

typedef src::severity_channel_logger_mt <
severity_level,     // the type of the severity level
std::string         // the type of the channel name
> kms_logger_mt;

BOOST_LOG_ATTRIBUTE_KEYWORD (category, "Category", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD (filename, "FileName", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD (gstline, "GstLine", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD (function, "Function", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD (gobject, "GObject", std::string)

BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT (system_logger, kms_logger_mt)
{
  kms_logger_mt logger (keywords::channel = "system");

  return logger;
}

void kms_init_logging ();

bool kms_init_logging_files (const std::string &path, int fileSize,
    int fileNumber);

} /* kurento */

#endif /* __KURENTO_LOGGING_HPP__ */
