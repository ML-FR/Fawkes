
/***************************************************************************
 *  tf_example_thread.cpp - tf example thread
 *
 *  Created: Tue Oct 25 18:01:36 2011
 *  Copyright  2011  Tim Niemueller [www.niemueller.de]
 ****************************************************************************/

/*  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  Read the full text in the LICENSE.GPL file in the doc directory.
 */

#include "tf_example_thread.h"

#include <tf/time_cache.h>

/** @class TfExampleThread "tf_example_thread.h"
 * Main thread of tf example plugin.
 * @author Tim Niemueller
 */

using namespace fawkes;

/** Constructor. */
TfExampleThread::TfExampleThread()
  : Thread("TfExampleThread", Thread::OPMODE_WAITFORWAKEUP),
    BlockedTimingAspect(BlockedTimingAspect::WAKEUP_HOOK_THINK)
{
}


/** Destructor. */
TfExampleThread::~TfExampleThread()
{
}


void
TfExampleThread::init()
{
  __tf_listener = new tf::TransformListener(blackboard);
}


void
TfExampleThread::finalize()
{
  delete __tf_listener;
}


void
TfExampleThread::loop()
{
  bool world_frame_exists = __tf_listener->frame_exists("/world");
  bool robot_frame_exists = __tf_listener->frame_exists("/robot");

  if (! world_frame_exists || ! robot_frame_exists) {
    logger->log_warn(name(), "Frame missing: world %s   robot %s",
                     world_frame_exists ? "exists" : "missing",
                     robot_frame_exists ? "exists" : "missing");
  } else {
    tf::StampedTransform transform;
    __tf_listener->lookup_transform("/robot", "/world", transform);

    fawkes::Time now;
    double diff = now - &transform.stamp;

    tf::Quaternion q = transform.getRotation();
    tf::Vector3 &v   = transform.getOrigin();

    const tf::TimeCache *world_cache = __tf_listener->get_frame_cache("/world");
    const tf::TimeCache *robot_cache = __tf_listener->get_frame_cache("/robot");

    logger->log_info(name(), "Transform %s -> %s, %f sec old: "
                     "T(%f,%f,%f)  Q(%f,%f,%f,%f)",
                     transform.frame_id.c_str(), transform.child_frame_id.c_str(),
                     diff, v.x(), v.y(), v.z(), q.x(), q.y(), q.z(), q.w());

    logger->log_info(name(), "World cache size: %zu  Robot cache size: %zu",
                     world_cache->getListLength(), robot_cache->getListLength());

  }
}
