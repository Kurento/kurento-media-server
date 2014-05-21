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

#ifndef __SIGNAL_HANDLER_HPP__
#define __SIGNAL_HANDLER_HPP__

#include <sys/signalfd.h>
#include <glibmm.h>

namespace kurento
{

class SignalHandler
{

public:

  SignalHandler (sigset_t mask,
                 std::function < void (uint32_t /* signal */) > handler,
                 Glib::RefPtr <Glib::MainContext > context = Glib::MainContext::get_default() );
  ~SignalHandler ();

private:

  bool handleSignal (Glib::IOCondition cond);

  Glib::RefPtr<Glib::IOSource> source;
  Glib::RefPtr<Glib::IOChannel> channel;
  std::function<void (uint32_t) > handler;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;

};

} // kurento

#endif /* __SIGNAL_HANDLER_HPP__ */
