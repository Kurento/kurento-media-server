/*
 * MediaManager.hpp - Kurento Media Server
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

#ifndef __MEDIA_MANAGER_HPP__
#define __MEDIA_MANAGER_HPP__

#include "MediaObjectImpl.hpp"
#include <common/MediaSet.hpp>

namespace kurento
{

class SdpEndPoint;
class UriEndPoint;
class HttpEndPoint;
class Mixer;
class Filter;

class MediaManager : public MediaObjectImpl, public std::enable_shared_from_this<MediaManager>
{

public:
  MediaManager();
  ~MediaManager() throw();

  std::shared_ptr<SdpEndPoint> createSdpEndPoint(const SdpEndPointType::type type);
  std::shared_ptr<SdpEndPoint> createSdpEndPoint(const SdpEndPointType::type type, const std::string& sdp);
  std::shared_ptr<UriEndPoint> createUriEndPoint (const UriEndPointType::type type, const std::string& uri);
  std::shared_ptr<HttpEndPoint> createHttpEndPoint ();
  std::shared_ptr<Mixer> createMixer (const MixerType::type type);
  std::shared_ptr<Filter> createFilter (const FilterType::type type);

private:
  MediaSet mediaSet;
};

} // kurento

#endif /* __MEDIA_MANAGER_HPP__ */
