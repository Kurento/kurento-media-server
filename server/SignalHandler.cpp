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

#include "SignalHandler.hpp"
#include <gst/gst.h>

/* This is included to avoid problems with slots and lamdas */
#include <type_traits>
#include <sigc++/sigc++.h>
#include <event2/event_struct.h>
namespace sigc
{
template <typename Functor>
struct functor_trait<Functor, false> {
  typedef decltype (::sigc::mem_fun (std::declval<Functor &> (),
                                     &Functor::operator() ) ) _intermediate;

  typedef typename _intermediate::result_type result_type;
  typedef Functor functor_type;
};
}

#define GST_CAT_DEFAULT kurento_signal_handler
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoSignalHandler"

namespace kurento
{

SignalHandler::SignalHandler (sigset_t mask,
                              std::function< void (uint32_t) > handler,
                              Glib::RefPtr <Glib::MainContext > context) : handler (handler)
{
  int fd;

  if (sigprocmask (SIG_BLOCK, &mask, NULL) < 0) {
    throw "Failed to set signal mask";
  }

  fd = signalfd (-1, &mask, 0);

  if (fd < 0) {
    throw "Failed to create signal descriptor";
  }

  channel = Glib::IOChannel::create_from_fd (fd);
  channel->set_close_on_unref (true);
  channel->set_encoding ("");
  channel->set_buffered (false);

  source = channel->create_watch (Glib::IO_IN | Glib::IO_HUP | Glib::IO_ERR |
                                  Glib::IO_NVAL);

  // TODO: Investigate why std::bind cannot be used here
  source->connect ([this] (Glib::IOCondition cond) -> bool {
    return handleSignal (cond);
  });

  source->attach (context);
}

SignalHandler::~SignalHandler()
{
  source->destroy();
}

bool
SignalHandler::handleSignal (Glib::IOCondition cond)
{
  struct signalfd_siginfo si;
  gsize bytes_read, count = sizeof (si);
  Glib::IOStatus status;

  if (cond & (Glib::IO_NVAL | Glib::IO_ERR | Glib::IO_HUP) ) {
    return false;
  }

  /* Read signal info */
  status = channel->read ( (char *) &si, count, bytes_read);

  switch (status) {
  case Glib::IO_STATUS_ERROR:
    GST_ERROR ("Error occurred.");
    return false;

  case Glib::IO_STATUS_EOF:
    GST_DEBUG ("End of file.");
    return false;

  case Glib::IO_STATUS_AGAIN:
    GST_DEBUG ("Resource temporarily unavailable.");
    return false;

  case Glib::IO_STATUS_NORMAL:
    if (bytes_read != count) {
      GST_DEBUG ("Could not read siginfo structure");
      return false;
    }

    break;
  }

  /* Manage signal */
  handler (si.ssi_signo);

  return true;
}

SignalHandler::StaticConstructor SignalHandler::staticConstructor;

SignalHandler::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
