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

#ifndef __ZBAR_FILTER_HPP__
#define __ZBAR_FILTER_HPP__

#include "Filter.hpp"

namespace kurento
{

class ZBarFilter : public Filter
{
public:
  ZBarFilter (MediaSet &mediaSet, std::shared_ptr<MediaPipeline> parent,
              const std::map<std::string, KmsMediaParam> & params)
  throw (KmsMediaServerException);
  ~ZBarFilter() throw ();

private:
  void init (std::shared_ptr<MediaPipeline> parent);

  gulong bus_handler_id;

  GstElement *zbar;

  guint64 lastTs = G_GUINT64_CONSTANT (0);
  std::string lastType;
  std::string lastSymbol;

  void barcodeDetected (guint64 ts, std::string &type, std::string &symbol);
  void raiseEvent (guint64 ts, std::string &type, std::string &symbol);

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
