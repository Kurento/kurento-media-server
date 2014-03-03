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

#ifndef __MEDIA_PAD_IMPL_HPP__
#define __MEDIA_PAD_IMPL_HPP__

#include "MediaObjectImpl.hpp"
#include <generated/MediaPad.hpp>
#include <generated/MediaType.hpp>

namespace kurento
{

class MediaPadImpl : public virtual MediaPad, public MediaObjectImpl
{
public:
  MediaPadImpl (std::shared_ptr<MediaType> mediaType,
                const std::string &mediaDescription,
                std::shared_ptr<MediaObjectImpl> parent);
  virtual ~MediaPadImpl() {};

  virtual std::shared_ptr<MediaElement> getMediaElement ();
  virtual std::shared_ptr<MediaType> getMediaType ();
  virtual std::string getMediaDescription ();

  GstElement *getGstreamerElement ();
private:
  std::shared_ptr<MediaType> mediaType;
  std::string mediaDescription;
};
} /* kurento */

#endif /* __MEDIA_PAD_IMPL_HPP__ */
