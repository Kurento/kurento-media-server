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

#ifndef __MEDIA_CONFIG_HPP__
#define __MEDIA_CONFIG_HPP__

#include <gst/sdp/gstsdpmessage.h>
#include <glibmm.h>

#define DEFAULT_CONFIG_FILE "/etc/kurento/kurento.conf"

#define SERVER_GROUP "Server"
#define MEDIA_SERVER_ADDRESS_KEY "serverAddress"
#define MEDIA_SERVER_SERVICE_PORT_KEY "serverPort"
#define SDP_PATTERN_KEY "sdpPattern"

#define HTTP_EP_SERVER_GROUP "HttpEPServer"
#define HTTP_EP_SERVER_ADDRESS_KEY MEDIA_SERVER_ADDRESS_KEY
#define HTTP_EP_SERVER_SERVICE_PORT_KEY MEDIA_SERVER_SERVICE_PORT_KEY
#define HTTP_EP_SERVER_ANNOUNCED_ADDRESS_KEY "announcedAddress"

#define MEDIA_SERVER_ADDRESS "localhost"
#define MEDIA_SERVER_SERVICE_PORT 9090

#define HTTP_EP_SERVER_SERVICE_PORT ((MEDIA_SERVER_SERVICE_PORT) + 1)

extern GstSDPMessage *sdpPattern;

#endif /* __MEDIA_CONFIG_HPP__ */
