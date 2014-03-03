/*
 * (C) Copyright 2013 Kurento (http://kurento.org/)
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

#include "MediaServerServiceHandler.hpp"
#include "KmsMediaServer_constants.h"
#include <gst/gst.h>

#define GST_CAT_DEFAULT kurento_media_server_service_handler
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoMediaServerServiceHandler"

namespace kurento
{

MediaServerServiceHandler::MediaServerServiceHandler (
  std::map <std::string, KurentoModule *> &modules) : modules (modules)
{
}

MediaServerServiceHandler::~MediaServerServiceHandler ()
{
}

void
MediaServerServiceHandler::invokeJsonRpc (std::string &_return,
    const std::string &request)
{
  GST_DEBUG ("Json request: %s", request.c_str() );
  methods.process (request, _return);
  GST_DEBUG ("Json response: %s", _return.c_str() );
}


MediaServerServiceHandler::StaticConstructor
MediaServerServiceHandler::staticConstructor;

MediaServerServiceHandler::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} // kurento
