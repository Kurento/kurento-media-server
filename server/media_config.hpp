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
#define SDP_PATTERN_KEY "sdpPattern"

#define WEB_RTC_END_POINT_GROUP "WebRtcEndPoint"
#define WEB_RTC_END_POINT_STUN_SERVER_ADDRESS_KEY "stunServerAddress"
#define WEB_RTC_END_POINT_STUN_SERVER_PORT_KEY "stunServerPort"
#define WEB_RTC_END_POINT_TURN_URL_KEY "turnURL"
#define WEB_RTC_END_POINT_PEM_CERTIFICATE_KEY "pemCertificate"

#define STUN_SERVER_ADDRESS "77.72.174.167"
#define STUN_SERVER_PORT 0

#define HTTP_SERVICE_GROUP "HttpEPServer"
#define HTTP_SERVICE_ADDRESS "serverAddress"
#define HTTP_SERVICE_PORT "serverPort"
#define HTTP_SERVICE_ANNOUNCED_ADDRESS "announcedAddress"

#define HTTP_DEFAULT_PORT 9091

#define HTTP_EP_SERVER_SERVICE_PORT ((MEDIA_SERVER_SERVICE_PORT) + 1)

#endif /* __MEDIA_CONFIG_HPP__ */
