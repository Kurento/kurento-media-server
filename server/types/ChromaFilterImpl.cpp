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

#include "ChromaFilterImpl.hpp"
#include <generated/MediaPipeline.hpp>
#include <generated/WindowParam.hpp>
#include <KurentoException.hpp>

#define GST_CAT_DEFAULT kurento_chroma_filter_impl
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoChromaFilterImpl"

#define SET_BACKGROUND_URI "image-background"
#define SET_CALIBRATION_AREA "calibration-area"

namespace kurento
{

ChromaFilterImpl::ChromaFilterImpl (
  std::shared_ptr<WindowParam> window, const std::string &backgroundImage,
  std::shared_ptr< MediaObjectImpl > parent, int garbagePeriod) :
  FilterImpl (parent, garbagePeriod)
{
  GstStructure *aux;

  g_object_set (element, "filter-factory", "chroma", NULL);

  g_object_get (G_OBJECT (element), "filter", &chroma, NULL);

  if (chroma == NULL) {
    throw KurentoException ("Media Object not available");
  }

  aux = gst_structure_new ("calibration_area",
                           "x", G_TYPE_INT, window->getTopRightCornerX(),
                           "y", G_TYPE_INT, window->getTopRightCornerY(),
                           "width", G_TYPE_INT, window->getWidth(),
                           "height", G_TYPE_INT, window->getHeight(),
                           NULL);

  g_object_set (G_OBJECT (chroma), SET_CALIBRATION_AREA, aux, NULL);
  gst_structure_free (aux);

  g_object_set (G_OBJECT (this->chroma), SET_BACKGROUND_URI,
                backgroundImage.c_str(), NULL);

  g_object_unref (chroma);
}

void
ChromaFilterImpl::setBackground (const std::string &uri)
{
  g_object_set (G_OBJECT (chroma), SET_BACKGROUND_URI,
                uri.c_str(), NULL);
}

void
ChromaFilterImpl::unsetBackground ()
{
  g_object_set (G_OBJECT (chroma), SET_BACKGROUND_URI, NULL, NULL);
}

std::shared_ptr<MediaObject>
ChromaFilter::Factory::createObject (
  std::shared_ptr<MediaPipeline> mediaPipeline,
  std::shared_ptr<WindowParam> window, const std::string &backgroundImage,
  int garbagePeriod)
{
  std::shared_ptr<MediaObject> object (new ChromaFilterImpl (
                                         window, backgroundImage,
                                         std::dynamic_pointer_cast<MediaObjectImpl> (mediaPipeline), garbagePeriod) );

  return object;
}

ChromaFilterImpl::StaticConstructor ChromaFilterImpl::staticConstructor;

ChromaFilterImpl::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
