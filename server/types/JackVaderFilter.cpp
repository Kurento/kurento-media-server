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

#include "JackVaderFilter.hpp"

#include "KmsMediaJackVaderFilterType_constants.h"

#define GST_CAT_DEFAULT kurento_jack_vader_filter
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoJackVaderFilter"

namespace kurento
{

JackVaderFilter::JackVaderFilter (std::shared_ptr<MediaPipeline> parent)
  : Filter (parent, g_KmsMediaJackVaderFilterType_constants.TYPE_NAME)
{
  element = gst_element_factory_make ("filterelement", NULL);
  g_object_set (element, "filter-factory", "jackvader", NULL);
  g_object_ref (element);
  gst_bin_add (GST_BIN (parent->pipeline), element);
  gst_element_sync_state_with_parent (element);
}

JackVaderFilter::~JackVaderFilter() throw ()
{
  gst_bin_remove (GST_BIN ( ( (std::shared_ptr<MediaPipeline> &) parent)->pipeline), element);
  gst_element_set_state (element, GST_STATE_NULL);
  g_object_unref (element);
}

JackVaderFilter::StaticConstructor JackVaderFilter::staticConstructor;

JackVaderFilter::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);
}

} // kurento
