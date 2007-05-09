
/***************************************************************************
 *  initializer.h - Fawkes Aspect initializer
 *
 *  Created: Tue Jan 30 13:34:54 2007
 *  Copyright  2006-2007  Tim Niemueller [www.niemueller.de]
 *
 *  $Id$
 *
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111-1307, USA.
 */

#ifndef __ASPECT_INITIALIZER_H_
#define __ASPECT_INITIALIZER_H_

#include <core/threading/thread_initializer.h>

class BlackBoard;
class Configuration;
class Logger;
class FawkesNetworkHub;
class Thread;

class AspectInitializer : public ThreadInitializer
{
 public:
  AspectInitializer(BlackBoard *blackboard, Configuration *config, Logger *logger);

  virtual void init(Thread *thread);

  void set_fnet_hub(FawkesNetworkHub *fnethub);

 private:
  BlackBoard        *blackboard;
  Configuration     *config;
  Logger            *logger;
  FawkesNetworkHub  *fnethub;
};


#endif
