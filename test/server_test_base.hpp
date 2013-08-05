/*
 * server_test_base.hpp - Kurento Media Server
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

#include "MediaServerService.h"
#include <transport/TBufferTransports.h>

using namespace apache::thrift::transport;

#define START_SERVER_TEST() { int pid = start_server_test();

#define GET_SERVER_PID() pid

#define STOP_SERVER_TEST() stop_server_test(pid); }

int start_server_test ();
void stop_server_test (int pid);


struct F {
private:
  boost::shared_ptr<TTransport> transport;

public:
  int pid;
  bool initialized = false;
  boost::shared_ptr<kurento::MediaServerServiceClient> client;

  F();
  ~F();
};
