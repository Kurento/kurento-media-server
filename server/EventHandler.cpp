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

#include "EventHandler.hpp"
#include <utils/utils.hpp>
#include <gst/gst.h>

#include "KmsMediaHandlerService.h"

#define GST_CAT_DEFAULT kurento_event_handler
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoEventHandler"

namespace kurento
{

EventHandler::EventHandler (std::shared_ptr <MediaObject> object) :
  object (object)
{
  generateUUID (id);
}

EventHandler::~EventHandler()
{
  GST_INFO ("Disconnect event handler %s", id.c_str() );

  try {
    std::shared_ptr <MediaObject> obj = object.lock();

    if (obj) {
      conn.disconnect();
    }
  } catch (...) {
  }
}

EventHandler::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

EventHandler::StaticConstructor EventHandler::staticConstructor;


} /* kurento */
