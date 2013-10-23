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

#include "MediaObjectParent.hpp"
#include "common/MediaSet.hpp"

#define GST_CAT_DEFAULT kurento_media_object_parent
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoMediaObjectParent"

namespace kurento
{

MediaObjectParent::MediaObjectParent (MediaSet &mediaSet,
                                      const std::map < std::string,
                                      KmsMediaParam > & params) :
  MediaObjectImpl (params), mediaSet (mediaSet)
{

}

MediaObjectParent::MediaObjectParent (MediaSet &mediaSet,
                                      std::shared_ptr< MediaObjectImpl > parent,
                                      const std::map< std::string, KmsMediaParam > &params) :
  MediaObjectImpl (parent, params), mediaSet (mediaSet)
{

}

MediaObjectParent::~MediaObjectParent () throw ()
{
}

void MediaObjectParent::registerChild (std::shared_ptr< MediaObjectImpl > child)
{
  mediaSet.put (child);
}

MediaSet &MediaObjectParent::getMediaSet()
{
  return mediaSet;
}

MediaObjectParent::StaticConstructor MediaObjectParent::staticConstructor;

MediaObjectParent::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} // kurento
