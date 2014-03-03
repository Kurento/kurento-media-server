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

#include "JackVaderFilterImpl.hpp"
#include <generated/MediaPipeline.hpp>
#include <KurentoException.hpp>

#define GST_CAT_DEFAULT kurento_jack_vader_filter_impl
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoJackVaderFilterImpl"


namespace kurento
{

JackVaderFilterImpl::JackVaderFilterImpl (
  std::shared_ptr< MediaObjectImpl > parent, int garbagePeriod) :
  FilterImpl (parent, garbagePeriod)
{
  GstElement *filter = NULL;

  g_object_set (element, "filter-factory", "jackvader", NULL);

  g_object_get (element, "filter", &filter, NULL);

  if (filter == NULL) {
    throw KurentoException ("Media Object not available");
  }

  g_object_unref (filter);
}

std::shared_ptr<MediaObject>
JackVaderFilter::Factory::createObject (std::shared_ptr<MediaPipeline>
                                        mediaPipeline,
                                        int garbagePeriod)
{
  std::shared_ptr<MediaObject> object (new JackVaderFilterImpl (
                                         std::dynamic_pointer_cast<MediaObjectImpl> (mediaPipeline),
                                         garbagePeriod) );

  return object;
}

JackVaderFilterImpl::StaticConstructor JackVaderFilterImpl::staticConstructor;

JackVaderFilterImpl::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
