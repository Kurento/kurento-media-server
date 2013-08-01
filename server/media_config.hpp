/*
 * media_config.hpp - Kurento Media Server
 *
 * Copyright (C) 2013 Kurento
 * Contact: Miguel París Díaz <mparisdiaz@gmail.com>
 * Contact: José Antonio Santos Cadenas <santoscadenas@kurento.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

#define MEDIA_SERVER_ADDRESS "localhost"
#define MEDIA_SERVER_SERVICE_PORT 9090

extern GstSDPMessage *sdpPattern;

#endif /* __MEDIA_CONFIG_HPP__ */
