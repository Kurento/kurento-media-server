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

#include <gst/gst.h>
#include "ServiceFactory.hpp"
#include "ThriftService.hpp"
#include "RabbitMQService.hpp"

#define GST_CAT_DEFAULT kurento_service_factory
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoServiceFactory"

#define FACTORY_GROUP "Server"
#define FACTORY_SERVICE "service"

namespace kurento
{

Service *ServiceFactory::create_service (Glib::KeyFile &confFile)
{
  std::string service;

  service = confFile.get_string (FACTORY_GROUP, FACTORY_SERVICE);

  if (service == "Thrift") {
    return new ThriftService (confFile);
  } else if (service == "RabbitMQ") {
    return new RabbitMQService (confFile);
  }

  throw Glib::OptionError (Glib::OptionError::UNKNOWN_OPTION,
                           "Service " + service + " not found");
}

ServiceFactory::StaticConstructor ServiceFactory::staticConstructor;

ServiceFactory::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
