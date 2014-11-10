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

#include <gst/gst.h>

namespace kurento
{

void simple_log_function (GstDebugCategory *category, GstDebugLevel level,
                          const gchar *file, const gchar *function, gint line,
                          GObject *object, GstDebugMessage *message,
                          gpointer user_data) G_GNUC_NO_INSTRUMENT;

} /* kurento */

#endif /* __KURENTO_LOGGING_HPP__ */
