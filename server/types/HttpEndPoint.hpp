/*
 * HttpEndPoint.hpp - Kurento Media Server
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

#ifndef __HTTP_END_POINT_HPP__
#define __HTTP_END_POINT_HPP__

#include "EndPoint.hpp"

namespace kurento
{

class HttpEndPoint : public EndPoint
{
public:
  HttpEndPoint (std::shared_ptr<MediaPipeline> parent);
  ~HttpEndPoint() throw ();

  std::string getUrl ();
  void setUrl (std::string);

private:
  std::string url;

private:
  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
  gulong urlRemovedHandlerId;

  friend gboolean register_http_end_point (gpointer data);
};

} // kurento

#endif /* __HTTP_END_POINT_HPP__ */
