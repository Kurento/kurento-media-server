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

#include "FaceOverlayFilter.hpp"

#include "KmsMediaFaceOverlayFilterType_constants.h"

#include "utils/utils.hpp"
#include "utils/marshalling.hpp"
#include "KmsMediaDataType_constants.h"
#include "KmsMediaErrorCodes_constants.h"

#define GST_CAT_DEFAULT kurento_face_overlay_filter
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoFaceOverlayFilter"

namespace kurento
{
/* default constructor */
FaceOverlayFilter::FaceOverlayFilter (
  MediaSet &mediaSet, std::shared_ptr<MediaPipeline> parent,
  const std::map<std::string, KmsMediaParam> &params)
  : Filter (mediaSet, parent, g_KmsMediaFaceOverlayFilterType_constants.TYPE_NAME,
            params)
{
  g_object_set (element, "filter-factory", "faceoverlay", NULL);

  GstElement *faceOverlay;

  g_object_get (G_OBJECT (element), "filter", &faceOverlay, NULL);

  this->faceOverlay = faceOverlay;
  g_object_unref (faceOverlay);
}

FaceOverlayFilter::~FaceOverlayFilter() throw ()
{
}

void
FaceOverlayFilter::setImageOverlay (KmsMediaFaceOverlayImage image)
{
  GstStructure *imageSt;
  imageSt = gst_structure_new ("image",
                               "offsetXPercent", G_TYPE_DOUBLE, double (image.offsetXPercent),
                               "offsetYPercent", G_TYPE_DOUBLE, double (image.offsetYPercent),
                               "widthPercent", G_TYPE_DOUBLE, double (image.widthPercent),
                               "heightPercent", G_TYPE_DOUBLE, double (image.heightPercent),
                               "url", G_TYPE_STRING, image.uri.c_str(),
                               NULL);
  g_object_set (G_OBJECT (faceOverlay), "image-to-overlay", imageSt, NULL);
  gst_structure_free (imageSt);
}

void
FaceOverlayFilter::unsetImageOverlay ()
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
FaceOverlayFilter::invoke (KmsMediaInvocationReturn &_return,
                           const std::string &command,
                           const std::map< std::string, KmsMediaParam > &params)
throw (KmsMediaServerException)
{
  if (g_KmsMediaFaceOverlayFilterType_constants.SET_IMAGE_OVERLAY.compare (
        command) == 0) {
    KmsMediaFaceOverlayImage imageInfo;
    const KmsMediaParam *p;

    p = getParam (params,
                  g_KmsMediaFaceOverlayFilterType_constants.SET_IMAGE_OVERLAY_PARAM_IMAGE);

    if (p != NULL) {
      unmarshalStruct (imageInfo, p->data);
      setImageOverlay (imageInfo);
    }
  } else if (
    g_KmsMediaFaceOverlayFilterType_constants.UNSET_IMAGE_OVERLAY.compare (
      command) == 0) {
    unsetImageOverlay ();
  }

}

FaceOverlayFilter::StaticConstructor FaceOverlayFilter::staticConstructor;

FaceOverlayFilter::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} // kurento
