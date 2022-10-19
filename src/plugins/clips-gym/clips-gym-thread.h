/***************************************************************************
 *  clips-gym-thread.h -
 *
 *  Created:
 *  Copyright
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

#include <aspect/aspect_provider.h>
#include <aspect/blackboard.h>
#include <aspect/configurable.h>
#include <aspect/logging.h>
#include <blackboard/interface_listener.h>
#include <core/threading/thread.h>
#include <plugins/clips/aspect/clips_feature.h>

#include <chrono>
#include <mutex>
#include <thread>
//#include <plugins/clips/aspect/clips.h>

// for interaction with the CX
#include <clipsmm.h>

//#include <boost/python.hpp>
//namespace py = boost::python;
#include <pybind11/embed.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
namespace py = pybind11;

#include "clips-observation-info.h"

class ClipsGymThread : public fawkes::Thread,
                       public fawkes::LoggingAspect,
                       public fawkes::ConfigurableAspect,
                       public fawkes::BlackBoardAspect,
                       public fawkes::BlackBoardInterfaceListener,
                       public fawkes::CLIPSFeature,
                       public fawkes::CLIPSFeatureAspect
{
public:
	ClipsGymThread();

	void         init() override;
	virtual void finalize();
	virtual void loop();

	// for CLIPSFeature
	virtual void clips_context_init(const std::string                   &env_name,
	                                fawkes::LockPtr<CLIPS::Environment> &clips);
	virtual void clips_context_destroyed(const std::string &env_name);

	void rl_loop_start(std::string env_name);

	fawkes::LockPtr<CLIPS::Environment> getClipsEnv();

	/*
	* Pybind 11 Module functions
	* - OpenAi Gym functions
	*/
	void initCX();
	//ClipsObservationInfo step();
	std::string step(std::string next_goal);
	void        resetCX();

	static ClipsGymThread *getInstance();
	py::list               generateActionSpace();
	py::list               generateObservationSpace();
	std::string            create_rl_env_state_from_facts();

	std::string              getGoalId(std::string action);
	std::vector<std::string> getAllFormulatedGoals();

protected:
	virtual void
	run()
	{
		Thread::run();
	}

private:
	std::map<std::string, fawkes::LockPtr<CLIPS::Environment>> envs_;
	std::string                                                clisp_env_name;
	constexpr static char cfg_prefix_[] = "/plugins/clips-gym/static/";

	static ClipsGymThread *thread_instance;
	static std::mutex      mutex;

	//helper functions
	std::vector<std::string>  splitActionToGoalParams(std::string action);
	std::string               getClipsSlotValuesAsString(std::vector<CLIPS::Value> slot_values);
	std::vector<std::string> *getClipsSlotValuesAsStringVector(std::vector<CLIPS::Value> slot_values);

	void assertRlGoalSelectionFact(std::string goalID);
};
//#endif