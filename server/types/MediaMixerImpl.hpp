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

#ifndef __MEDIA_MIXER_IMPL_HPP__
#define __MEDIA_MIXER_IMPL_HPP__

#include "MediaObjectImpl.hpp"
#include <gst/gst.h>
#include <generated/MediaMixer.hpp>

namespace kurento
{

class MediaMixerImpl : public virtual MediaMixer, public MediaObjectImpl,
  public std::enable_shared_from_this<MediaMixerImpl>
{
public:
  MediaMixerImpl (const std::string &factoryName,
                  std::shared_ptr<MediaObjectImpl> parent, int garbagePeriod);
  virtual ~MediaMixerImpl() throw ();

  GstElement *getGstreamerElement() {
    return element;
  };

protected:
  GstElement *element;

private:

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} /* kurento */

#endif /* __MEDIA_MIXER_IMPL_HPP__ */
