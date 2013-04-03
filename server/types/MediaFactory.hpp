/*
 * MediaFactory.hpp - Kurento Media Server
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

#ifndef __MEDIAFACTORY_HPP__
#define __MEDIAFACTORY_HPP__

#include "MediaObjectImpl.hpp"
#include <common/MediaSet.hpp>

namespace kurento
{

class MediaPlayer;
class MediaRecorder;
class Stream;
class Mixer;

class MediaFactory : public MediaObjectImpl, public std::enable_shared_from_this<MediaFactory>
{

public:
  MediaFactory();
  ~MediaFactory() throw();

  std::shared_ptr<MediaPlayer> createMediaPlayer();
  std::shared_ptr<MediaRecorder> createMediaRecorder();
  std::shared_ptr<Stream> createStream();
  std::shared_ptr<Mixer> createMixer (const int32_t mixerId);

private:
  MediaSet mediaSet;
};

} // kurento

#endif /* __MEDIAFACTORY_HPP__ */
