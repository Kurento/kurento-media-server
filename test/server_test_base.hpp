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
