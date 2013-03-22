/*
 * MediaRecorder.cpp - Kurento Media Server
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

#include "MediaRecorder.hpp"
#include <gst/gst.h>

#define GST_CAT_DEFAULT media_recorder
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "media_recorder"

namespace kurento
{

MediaRecorder::MediaRecorder (MediaFactory &mediaFactory) : Joinable (mediaFactory)
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);
}

MediaRecorder::~MediaRecorder() throw ()
{

}

void
MediaRecorder::record ()
{
  GST_INFO ("RECORDER RECORD");
}

void
MediaRecorder::pause ()
{
  GST_INFO ("RECORDER PAUSE");
}

void
MediaRecorder::stop ()
{
  GST_INFO ("RECORDER STOP");
}

} // kurento
