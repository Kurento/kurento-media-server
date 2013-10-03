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

#include "RecorderEndPoint.hpp"

#include "RecorderEndPointType_constants.h"

namespace kurento
{

RecorderEndPoint::RecorderEndPoint (std::shared_ptr<MediaPipeline> parent, const std::string &uri)
  : UriEndPoint (parent, g_RecorderEndPointType_constants.TYPE_NAME)
{
  element = gst_element_factory_make ("recorderendpoint", NULL);

  g_object_set (G_OBJECT (element), "uri", uri.c_str(), NULL);

  g_object_ref (element);
  gst_bin_add (GST_BIN (parent->pipeline), element);
  gst_element_sync_state_with_parent (element);
}

RecorderEndPoint::~RecorderEndPoint() throw ()
{
  gst_bin_remove (GST_BIN ( ( (std::shared_ptr<MediaPipeline> &) parent)->pipeline), element);
  gst_element_set_state (element, GST_STATE_NULL);
  g_object_unref (element);
}

} // kurento
