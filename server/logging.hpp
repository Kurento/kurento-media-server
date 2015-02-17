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

bool kms_init_logging (const std::string &path);

} /* kurento */

#endif /* __KURENTO_LOGGING_HPP__ */
