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

#include "ChromaFilter.hpp"

#include "KmsMediaChromaFilterType_constants.h"

#include "utils/utils.hpp"
#include "utils/marshalling.hpp"
#include "KmsMediaDataType_constants.h"
#include "KmsMediaErrorCodes_constants.h"

#define GST_CAT_DEFAULT kurento_chroma_filter
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoChromaFilter"

#define SET_BACKGROUND_URI "image-background"
#define SET_CALIBRATION_AREA "calibration-area"

namespace kurento
{
/* default constructor */
ChromaFilter::ChromaFilter (
  MediaSet &mediaSet, std::shared_ptr<MediaPipeline> parent,
  const std::map<std::string, KmsMediaParam> &params)
  : Filter (mediaSet, parent, g_KmsMediaChromaFilterType_constants.TYPE_NAME,
            params)
{
  const KmsMediaParam *p;
  KmsMediaChromaConstructorParams constructorParams;
  GstElement *chroma;
  GstStructure *aux;

  p = getParam (params,
                g_KmsMediaChromaFilterType_constants.CONSTRUCTOR_PARAMS_DATA_TYPE);

  if (p == NULL) {
    KmsMediaServerException except;

    createKmsMediaServerException (except,
                                   g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_ILLEGAL_PARAM_ERROR,
                                   "Constructor Parameters not found");
    throw except;
  }

  element = gst_element_factory_make ("filterelement", NULL);

  g_object_set (element, "filter-factory", "chroma", NULL);
  g_object_ref (element);
  gst_bin_add (GST_BIN (parent->pipeline), element);
  gst_element_sync_state_with_parent (element);
  g_object_get (G_OBJECT (element), "filter", &chroma, NULL);
  this->chroma = chroma;

  if (this->chroma == NULL) {
    g_object_unref (element);
    KmsMediaServerException except;

    createKmsMediaServerException (except,
                                   g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_NOT_AVAILAIBLE,
                                   "Media Object not available");
    throw except;
  }

  unmarshalStruct (constructorParams, p->data);
  aux = gst_structure_new ("calibration_area",
                           "x", G_TYPE_INT, constructorParams.calibrationArea.x,
                           "y", G_TYPE_INT, constructorParams.calibrationArea.y,
                           "width", G_TYPE_INT, constructorParams.calibrationArea.width,
                           "height", G_TYPE_INT, constructorParams.calibrationArea.height,
                           NULL);

  g_object_set (G_OBJECT (this->chroma), SET_CALIBRATION_AREA, aux, NULL);
  gst_structure_free (aux);

  if ( constructorParams.__isset.backgroundImage) {
    g_object_set (G_OBJECT (this->chroma), SET_BACKGROUND_URI,
                  constructorParams.backgroundImage.uri.c_str(), NULL);
  }

  g_object_unref (chroma);
}

ChromaFilter::~ChromaFilter() throw ()
{
  gst_bin_remove (GST_BIN (std::dynamic_pointer_cast<MediaPipeline>
                           (parent)->pipeline ), element);
  gst_element_set_state (element, GST_STATE_NULL);
  g_object_unref (element);
}

void
ChromaFilter::setBackground (KmsMediaChromaBackgroundImage backgroundImage)
{
  g_object_set (G_OBJECT (this->chroma), SET_BACKGROUND_URI,
                backgroundImage.uri.c_str(), NULL);
}

void
ChromaFilter::unsetBackground ()
{
  g_object_set (G_OBJECT (this->chroma), SET_BACKGROUND_URI, NULL, NULL);
}

void
ChromaFilter::invoke (KmsMediaInvocationReturn &_return,
                      const std::string &command,
                      const std::map< std::string, KmsMediaParam > &params)
throw (KmsMediaServerException)
{
  if (g_KmsMediaChromaFilterType_constants.SET_BACKGROUND.compare (
        command) == 0) {
    KmsMediaChromaBackgroundImage backgroundImage;
    const KmsMediaParam *p;
    /* extract window params from param */
    p = getParam (params,
                  g_KmsMediaChromaFilterType_constants.SET_BACKGROUND_PARAM_BACKGROUND_IMAGE);

    if (p != NULL) {
      unmarshalStruct (backgroundImage, p->data);
      setBackground (backgroundImage);
    }
  } else if (g_KmsMediaChromaFilterType_constants.UNSET_BACKGROUND.compare (command) == 0) {
    unsetBackground ();
  }
}

ChromaFilter::StaticConstructor ChromaFilter::staticConstructor;

ChromaFilter::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} // kurento
