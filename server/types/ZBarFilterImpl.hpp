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

#ifndef __Z_BAR_FILTER_IMPL_HPP__
#define __Z_BAR_FILTER_IMPL_HPP__

#include "FilterImpl.hpp"
#include <generated/ZBarFilter.hpp>

namespace kurento
{

class ZBarFilterImpl: public virtual ZBarFilter, public FilterImpl
{
public:
  ZBarFilterImpl (std::shared_ptr<MediaObjectImpl> parent, int garbagePeriod);
  virtual ~ZBarFilterImpl() throw ();

private:
  GstElement *zbar;
  gulong bus_handler_id;

  guint64 lastTs = G_GUINT64_CONSTANT (0);
  std::string lastType;
  std::string lastSymbol;

  std::function<void (GstMessage *) > busMessageLambda;

  void barcodeDetected (guint64 ts, std::string &type, std::string &symbol);

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;

};

} /* kurento */

#endif /* __Z_BAR_FILTER_IMPL_HPP__ */
