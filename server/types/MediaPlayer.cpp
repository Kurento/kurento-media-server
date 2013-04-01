/*
 * MediaPlayer.cpp - Kurento Media Server
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

#include "MediaPlayer.hpp"
#include <gst/gst.h>

#define GST_CAT_DEFAULT media_player
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "MediaPlayer"

namespace kurento
{

MediaPlayer::MediaPlayer (MediaFactory &mediaFactory) : Joinable (mediaFactory)
{
}

MediaPlayer::~MediaPlayer() throw ()
{

}

void
MediaPlayer::play ()
{
  GST_INFO ("PLAYER PLAY");
}

void
MediaPlayer::pause ()
{
  GST_INFO ("PLAYER PAUSE");
}

void
MediaPlayer::stop ()
{
  GST_INFO ("PLAYER STOP");
}

MediaPlayer::StaticConstructor MediaPlayer::staticConstructor;

MediaPlayer::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);
}

} // kurento
