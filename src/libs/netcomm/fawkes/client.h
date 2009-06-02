
/***************************************************************************
 *  client.h - Fawkes network client
 *
 *  Created: Tue Nov 21 18:42:10 2006
 *  Copyright  2006  Tim Niemueller [www.niemueller.de]
 *
 *  $Id$
 *
 ****************************************************************************/

/*  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version. A runtime exception applies to
 *  this software (see LICENSE.GPL_WRE file mentioned below for details).
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  Read the full text in the LICENSE.GPL_WRE file in the doc directory.
 */

#ifndef __NETCOMM_FAWKES_CLIENT_H_
#define __NETCOMM_FAWKES_CLIENT_H_

#include <netcomm/fawkes/message_queue.h>
#include <netcomm/fawkes/message.h>
#include <netcomm/fawkes/component_ids.h>

#include <core/exception.h>
#include <core/utils/lock_map.h>

namespace fawkes {

class StreamSocket;
class Mutex;
class WaitCondition;
class FawkesNetworkClientHandler;
class FawkesNetworkClientSendThread;
class FawkesNetworkClientRecvThread;

class HandlerAlreadyRegisteredException : public Exception
{
 public:
  HandlerAlreadyRegisteredException();
};

class FawkesNetworkClient
{
 friend class FawkesNetworkClientSendThread;
 friend class FawkesNetworkClientRecvThread;
 public:
 FawkesNetworkClient();
 FawkesNetworkClient(const char *hostname, unsigned short int port, const char *ip = NULL);
 FawkesNetworkClient(unsigned int id, const char *hostname, 
                     unsigned short int port, const char *ip = NULL);
  ~FawkesNetworkClient();

  void connect();
  void disconnect();
  void connect(const char *hostname, unsigned short int port);
  void connect(const char *hostname, const char *ip, unsigned short int port);

  void enqueue(FawkesNetworkMessage *message);
  void enqueue_and_wait(FawkesNetworkMessage *message);

  void wait(unsigned int component_id);
  void wake(unsigned int component_id);

  void interrupt_connect();

  void register_handler(FawkesNetworkClientHandler *handler, unsigned int component_id);
  void deregister_handler(unsigned int component_id);

  bool connected() const throw();

  bool has_id() const;
  unsigned int id() const;

  const char *get_hostname() const;
  const char *get_ip() const;

 private:
  void recv();
  void notify_of_connection_established();
  void notify_of_connection_dead();

  void wake_handlers(unsigned int cid);
  void dispatch_message(FawkesNetworkMessage *m);
  void connection_died();
  void set_send_slave_alive();
  void set_recv_slave_alive();

  char *__hostname;
  char *__ip;
  unsigned short int __port;

  StreamSocket *s;

  typedef LockMap<unsigned int, FawkesNetworkClientHandler *> HandlerMap;
  HandlerMap  handlers;

  WaitCondition *__connest_waitcond;
  Mutex         *__connest_mutex;
  bool           __connest;
  bool           __connest_interrupted;

  Mutex                         *__recv_mutex;
  WaitCondition                 *__recv_waitcond;
  std::map<unsigned int, bool>   __recv_received;
  FawkesNetworkClientRecvThread *__recv_slave;
  FawkesNetworkClientSendThread *__send_slave;
  bool                           __recv_slave_alive;
  bool                           __send_slave_alive;

  bool connection_died_recently;
  Mutex *slave_status_mutex;
  bool _has_id;
  unsigned int _id;
};

} // end namespace fawkes

#endif
