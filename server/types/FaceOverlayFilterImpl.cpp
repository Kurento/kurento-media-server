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

#include "FaceOverlayFilterImpl.hpp"
#include <generated/MediaPipeline.hpp>
#include <KurentoException.hpp>

#define GST_CAT_DEFAULT kurento_face_overlay_filter_impl
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoFaceOverlayFilterImpl"


namespace kurento
{

FaceOverlayFilterImpl::FaceOverlayFilterImpl (
  std::shared_ptr< MediaObjectImpl > parent, int garbagePeriod) :
  FilterImpl (parent, garbagePeriod)
{
  g_object_set (element, "filter-factory", "faceoverlay", NULL);

  g_object_get (G_OBJECT (element), "filter", &faceOverlay, NULL);

  if (faceOverlay == NULL) {
    throw KurentoException ("Media Object not available");
  }

  g_object_unref (faceOverlay);
}

void
FaceOverlayFilterImpl::unsetOverlayedImage ()
{
  GstStructure *imageSt;
  imageSt = gst_structure_new ("image",
                               "offsetXPercent", G_TYPE_DOUBLE, 0.0,
                               "offsetYPercent", G_TYPE_DOUBLE, 0.0,
                               "widthPercent", G_TYPE_DOUBLE, 0.0,
                               "heightPercent", G_TYPE_DOUBLE, 0.0,
                               "url", G_TYPE_STRING, NULL,
                               NULL);
  g_object_set (G_OBJECT (faceOverlay), "image-to-overlay", imageSt, NULL);
  gst_structure_free (imageSt);
}

void
FaceOverlayFilterImpl::setOverlayedImage (const std::string &uri,
    float offsetXPercent, float offsetYPercent, float widthPercent,
    float heightPercent)
{
  GstStructure *imageSt;
  imageSt = gst_structure_new ("image",
                               "offsetXPercent", G_TYPE_DOUBLE, double (offsetXPercent),
                               "offsetYPercent", G_TYPE_DOUBLE, double (offsetYPercent),
                               "widthPercent", G_TYPE_DOUBLE, double (widthPercent),
                               "heightPercent", G_TYPE_DOUBLE, double (heightPercent),
                               "url", G_TYPE_STRING, uri.c_str(),
                               NULL);
  g_object_set (G_OBJECT (faceOverlay), "image-to-overlay", imageSt, NULL);
  gst_structure_free (imageSt);
}

std::shared_ptr<MediaObject>
FaceOverlayFilter::Factory::createObject (std::shared_ptr<MediaPipeline>
    mediaPipeline,
    int garbagePeriod)
{
  std::shared_ptr<MediaObject> object (new FaceOverlayFilterImpl (
                                         std::dynamic_pointer_cast<MediaObjectImpl> (mediaPipeline),
                                         garbagePeriod) );

  return object;
}

FaceOverlayFilterImpl::StaticConstructor
FaceOverlayFilterImpl::staticConstructor;

FaceOverlayFilterImpl::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
