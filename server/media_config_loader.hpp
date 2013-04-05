/*
 * media_config_loader.hpp - Kurento Media Server
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

#ifndef __MEDIA_CONFIG_LOADER_HPP__
#define __MEDIA_CONFIG_LOADER_HPP__

#include <gst/sdp/gstsdpmessage.h>
#include <glibmm.h>

#define SERVER_GROUP "Server"

#define MEDIA_SERVER_ADDRESS "localhost"
#define MEDIA_SERVER_SERVICE_PORT 9090

GstSDPMessage *load_session_descriptor (Glib::KeyFile &configFile);

#endif /* __MEDIA_CONFIG_LOADER_HPP__ */
