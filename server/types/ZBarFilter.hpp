/*
 * ZBarFilter.hpp - Kurento Media Server
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

#ifndef __ZBAR_FILTER_HPP__
#define __ZBAR_FILTER_HPP__

#include "Filter.hpp"

namespace kurento
{

class ZBarFilter : public Filter
{
public:
  ZBarFilter (std::shared_ptr<MediaPipeline> parent);
  ~ZBarFilter() throw ();

private:

  gulong bus_handler_id;

  GstElement *zbar;

  guint64 lastTs = G_GUINT64_CONSTANT (0);
  std::string lastType;
  std::string lastSymbol;

  void barcodeDetected(guint64 ts, std::string& type, std::string& symbol);

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;

  friend void zbar_receive_message (GstBus *bus, GstMessage *message, gpointer element);
};

} // kurento

#endif /* __ZBAR_FILTER_HPP__ */
