/***************************************************************************
 *  rl_test_thread.cpp -
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

#include "rl-test-thread.h"

#include <chrono>
#include <future>
#include <iostream>
#include <regex>
#include <string>
#include <thread>

//for calling boost python from plugin
//#define BOOST_BIND_GLOBAL_PLACEHOLDERS

#include <core/threading/mutex_locker.h>

#include <boost/bind/bind.hpp>
#include <clipsmm.h>

using namespace std;

using namespace fawkes;

/** @class RLTestThread
 *  The plugin thread, initializes the aspect.
 *
 *  @author
 */

constexpr char RLTestThread::cfg_prefix_[];

RLTestThread::RLTestThread()
: Thread("RLTestThread", Thread::OPMODE_WAITFORWAKEUP), //Thread::OPMODE_CONTINUOUS),//
  BlackBoardInterfaceListener("RLTestThread"),
  CLIPSFeature("rl-test"),
  CLIPSFeatureAspect(this)
{
}

std::string
getConfigStringReplacedBasedir(Configuration *config, std::string configEntry)
{
	std::string config_value =
	  std::regex_replace(config->get_string(configEntry), std::regex("@BASEDIR@"), BASEDIR);
	return config_value;
}

bool
//RLTestThread::
trainingRlAgent(Configuration *config)
{
	bool        is_done       = false;
	std::string rl_agent_name = config->get_string("/rl-agent/name");
	std::string rl_agent_dir  = getConfigStringReplacedBasedir(config, "/rl-agent/dir");
	//std::regex_replace(config->get_string("/rl-agent/dir"), std::regex("@BASEDIR@"), BASEDIR);
	std::cout << rl_agent_dir << std::endl;
	std::string training_script = config->get_string("/python/training-script");
	std::string training_dir    = getConfigStringReplacedBasedir(config, "/python/dir");
	//std::regex_replace(config->get_string("/python/dir"), std::regex("@BASEDIR@"), BASEDIR);

	std::string env = config->get_string("/rl-agent/env_name");

	std::string execution_script = config->get_string("/python/execution-script");
	std::string execution_dir    = getConfigStringReplacedBasedir(config, "/python/dir");
	//std::regex_replace(config->get_string("/python/dir"), std::regex("@BASEDIR@"), BASEDIR);

	std::string env_dir = getConfigStringReplacedBasedir(config, "/python/env-dir");
	//std::regex_replace(config->get_string("/python/env-dir"), std::regex("@BASEDIR@"), BASEDIR);

	std::string bin_plugins_dir = getConfigStringReplacedBasedir(config, "/python/plugins-dir");
	//std::regex_replace(config->get_string("/python/plugins-dir"), std::regex("@BASEDIR@"), BASEDIR);

	py::scoped_interpreter guard{};

	try {
		py::object main_namespace = py::module_::import("__main__").attr("__dict__");
		py::exec("import sys", main_namespace);
		py::exec("print(\"Hello From python\")", main_namespace);

		//necessary to include other python scripts - e.g. PDDLExtension
		py::str sysPathAppend = (py::str)("sys.path.append(\"" + training_dir + "\")");
		py::exec(sysPathAppend, main_namespace);

		//necessary to include other python scripts - e.g. ClipsWorld
		py::str sysPathAppend2 = (py::str)("sys.path.append(\"" + env_dir + "\")");
		py::exec(sysPathAppend2, main_namespace);

		py::str sysPathAppend3 = (py::str)("sys.path.append(\"" + bin_plugins_dir + "\")");
		py::exec(sysPathAppend3, main_namespace);

		py::exec("print(sys.path)");

		//Add env name extracted from the config file to python
		py::str env_name = (py::str)("env_name = \"" + env + "\"");
		py::exec(env_name, main_namespace);

		//Add dir of env_name.pddl and problem.pddl extracted from the config file to python
		py::str dir_path = (py::str)("dir_path = \"" + rl_agent_dir + "\"");
		py::exec(dir_path, main_namespace);

		//Adds dir + name, where the rl agent should be saved to python
		std::cout << "RL Agent: " + rl_agent_dir + "/" + rl_agent_name << std::endl;
		py::str file_name = (py::str)("file_name = \"" + rl_agent_dir + "/" + rl_agent_name + "\"");
		py::exec(file_name, main_namespace);

		//For TrainingClipsWorld.py
		/* 		py::str action_space =
		  (py::str)("action_space = ['TOWER-C1#a#b', 'TOWER-C1#a#c', 'TOWER-C1#a#d' , 'TOWER-C1#a#e']");
		py::exec(action_space, main_namespace);
		py::exec("print(action_space)");

		std::string obs("obs_space = [");
		obs += "'clear(a)','clear(b)','clear(c)','clear(d)','clear(e)'";
		obs += ",'handempty(robo1)','handfull(robo1)','holding(a)','holding(b)','holding(c)','holding("
		       "d)','holding(e)'";
		obs += ",'on(a,b)','on(a,c)','on(a,d)','on(a,e)','on(b,a)','on(b,c)'";
		obs += ",'on(b,d)','on(b,e)','on(c,a)','on(c,b)','on(c,d)','on(c,e)'";
		obs += ",'on(d,a)','on(d,b)','on(d,c)','on(d,e)','on(e,a)','on(e,b)'";
		obs += ",'on(e,c)','on(e,d)'";
		obs += ",'ontable(a)', 'ontable(b)', 'ontable(c)', 'ontable(d)', 'ontable(e)']";

		py::str obs_space = (py::str)(obs);
		py::exec(obs_space, main_namespace);
		py::exec("print(obs_space)"); */

		//TODO maybe added to config - value of training timesteps
		py::str timesteps = (py::str)("timesteps = 1000");
		py::exec(timesteps, main_namespace);

		//printing python variables with config values
		//main_print =
		py::exec("print(\"Config values for env_name and path: \" + env_name + \" \" + dir_path)",
		         main_namespace);

		std::cout << "Training script: " + rl_agent_dir + "/" + training_script << std::endl;
		py::str training_script_path = (py::str)(rl_agent_dir + "/" + training_script);
		//py::exec_file(training_script_path, main_namespace, main_namespace);
		auto result = py::eval_file(training_script_path, main_namespace);
		std::cout << "DONE EVALUATING TRAINING SCRIPT - I should probably give feedback to clips"
		          << std::endl;
		py::print(result);
		is_done = true;

	} catch (py::error_already_set &e) {
		py::module::import("traceback").attr("print_exception")(e.type(), e.value(), e.trace());
		std::cout << "PYTHON EXCEPTION:" << std::endl;
		std::cout << e.what() << std::endl;
	} catch (...) {
		PyErr_Print();
		PyErr_Clear();
	}

	return is_done;
}

std::string
RLTestThread::executeRlAgent(std::string facts)
{
	std::string rl_agent_name = config->get_string("/rl-agent/name");
	std::string rl_agent_dir =
	  std::regex_replace(config->get_string("/rl-agent/dir"), std::regex("@BASEDIR@"), BASEDIR);
	std::cout << rl_agent_dir << std::endl;
	std::string training_script = config->get_string("/python/training-script");
	std::string training_dir =
	  std::regex_replace(config->get_string("/python/dir"), std::regex("@BASEDIR@"), BASEDIR);

	std::string env = config->get_string("/rl-agent/env_name");

	std::string execution_script = config->get_string("/python/execution-script");
	std::string execution_dir =
	  std::regex_replace(config->get_string("/python/dir"), std::regex("@BASEDIR@"), BASEDIR);

	std::string env_script = config->get_string("/python/env-script");
	std::string env_dir =
	  std::regex_replace(config->get_string("/python/env-dir"), std::regex("@BASEDIR@"), BASEDIR);

	std::string selected_action = "";

	//for pybind11
	py::scoped_interpreter guard{};

	try {
		py::object main_namespace2 = py::module_::import("__main__").attr("__dict__");
		//py::object main_sys =
		py::exec("import sys", main_namespace2);
		//py::object main_print =
		py::exec("print(\"Hello from executing RL agent python\")", main_namespace2);

		//necessary to include other python scripts - e.g. PDDLExtension
		py::str sysPathAppend = (py::str)("sys.path.append(\"" + training_dir + "\")");
		//py::object main_missingPath =
		py::exec(sysPathAppend, main_namespace2);

		//necessary to include other python scripts - e.g. ClipsWorld
		py::str sysPathAppend2 = (py::str)("sys.path.append(\"" + env_dir + "\")");
		//py::object main_missingPath =
		py::exec(sysPathAppend2, main_namespace2);

		//Add env name extracted from the config file to python
		py::str env_name = (py::str)("env_name = \"" + env + "\"");
		py::exec(env_name, main_namespace2);

		//Add dir of env_name.pddl and problem.pddl extracted from the config file to python
		py::str dir_path = (py::str)("dir_path = \"" + rl_agent_dir + "\"");
		py::exec(dir_path, main_namespace2);

		//Adds dir + name, where the rl agent should be saved to python
		std::cout << "RL Agent: " + rl_agent_dir + "/" + rl_agent_name << std::endl;
		py::str file_name = (py::str)("file_name = \"" + rl_agent_dir + "/" + rl_agent_name + "\"");
		py::exec(file_name, main_namespace2);

		//TODO maybe added to config - value of training timesteps
		py::str timesteps = (py::str)("timesteps = 1000");
		py::exec(timesteps, main_namespace2);

		//printing python variables with config values
		//main_print =
		py::exec("print(\"Config values for env_name and path: \" + env_name + \" \" + dir_path)",
		         main_namespace2);

		//py::str obs = (py::str) ("obs = [0., 1., 1., 1., 0., 1., 1., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 1., 1., 1.,]");
		//py::exec(obs, main_namespace2);
		py::str obs = (py::str)("obs =" + facts);
		py::exec(obs, main_namespace2);
		py::exec("print(obs)", main_namespace2);

		std::cout << "Execution script: " + execution_dir + "/" + execution_script << std::endl;
		py::str execution_script_path = (py::str)(execution_dir + "/" + execution_script);
		//py::exec_file(execution_script_path, main_namespace2, main_namespace2);
		py::eval_file(execution_script_path, main_namespace2);

		//py::exec("result = env.env.actions[action]", main_namespace2);
		//py::object main_print2 =
		py::exec("print(\"Result py: \", result)", main_namespace2);

		//extract form boost python
		//selected_action = py::extract<std::string>(main_namespace2["result"]);
		//pybind11
		auto result     = main_namespace2["result"].cast<std::string>();
		selected_action = result;
		std::cout << "\nResult: " + selected_action << std::endl;

	} catch (py::error_already_set &e) {
		py::module::import("traceback").attr("print_exception")(e.type(), e.value(), e.trace());
		std::cout << "PYTHON EXCEPTION:" << std::endl;
		std::cout << e.what() << std::endl;
	} catch (...) {
		PyErr_Print();
		PyErr_Clear();
	}

	//Py_Finalize();

	std::cout << "Finished rl agent execution" << std::endl;
	return selected_action;
}

void
RLTestThread::init()
{
	std::cout << "Init RLTestThread start" << std::endl;
	std::string rl_agent_name = config->get_string("/rl-agent/name");
	std::string rl_agent_dir  = config->get_string("/rl-agent/dir");
	bool        training_mode = config->get_bool("/rl-agent/training-mode");
	std::cout << rl_agent_dir + rl_agent_name + " " + std::to_string(training_mode) << std::endl;

	//setup interface
	rl_gs_interface = blackboard->open_for_writing<RLAgentGoalSelectionInterface>("goal-selection");
	rl_gs_interface->set_msg_id(0);
	rl_gs_interface->set_final(false);
	rl_gs_interface->write();

	//setup interface listener
	bbil_add_message_interface(rl_gs_interface);
	blackboard->register_listener(this, BlackBoard::BBIL_FLAG_MESSAGES);

	std::cout << "Finished RLTestThread" << std::endl;

	startedTraining = false;
	//wakeup(); //activates any loop
}

void
RLTestThread::loop()
{
	std::cout << "In RLTestThread Loop " + goal << std::endl;
	rl_gs_interface->set_final(true);
	rl_gs_interface->set_success(true);
	rl_gs_interface->set_next_select_goal("RL TEST GOAL FROM LOOP");
	rl_gs_interface->write();

	bool training_mode = config->get_bool("/rl-agent/training-mode");

	if (training_mode && !startedTraining) {
		//trainingRlAgent();
		training_done = std::async(std::launch::async, trainingRlAgent, config);

		startedTraining = true;
	} else if (training_mode && startedTraining) {
		std::cout << "Check if training_done future is vailid" << std::endl;
		std::cout << training_done.valid() << "\n" << std::endl;
		int sec = 1000;
		std::cout << "Wait for " << sec << " msec to check future.get" << std::endl;
		std::future_status status = training_done.wait_for(std::chrono::milliseconds(sec));
		if (status == std::future_status::ready) {
			std::cout << "Future: " << training_done.get() << std::endl;
		}
	}

	std::cout << "End RLTestThread Loop " << std::endl;
}

void
RLTestThread::finalize()
{
	//Py_Finalize();
	blackboard->close(rl_gs_interface);
}

bool
RLTestThread::bb_interface_message_received(Interface *interface, fawkes::Message *message) noexcept
{
	if (message->is_of_type<RLAgentGoalSelectionInterface::GSelectionMessage>()) {
		RLAgentGoalSelectionInterface::GSelectionMessage *msg =
		  (RLAgentGoalSelectionInterface::GSelectionMessage *)message;
		rl_gs_interface->set_msg_id(msg->id());
		rl_gs_interface->set_final(false);
		rl_gs_interface->write();
		if (std::string(msg->goal()) != "")
			goal = msg->goal();
		//wakeup(); //activates loop where the generation is done
	} else {
		logger->log_error(name(), "Received unknown message of type %s, ignoring", message->type());
	}
	return false;
}

void
RLTestThread::clips_context_init(const std::string &env_name, LockPtr<CLIPS::Environment> &clips)
{
	std::cout << "Start RLTestThread clips_context_init\n" << std::endl;

	envs_[env_name] = clips;
	logger->log_info(name(), "Called to initialize environment %s", env_name.c_str());

	clips.lock();
	clips->evaluate("(printout t \"Hello from CLIPS aspect in RL test plugin\" crlf crlf)");
	/*clips->add_function("rl-extract-executable-fact",
						   sigc::slot<void, CLIPS::Value, std::string>(sigc::bind<0>(
						  sigc::mem_fun(*this, &RLTestThread::clips_rl_extract_executable_facts),
						  env_name)));*/
	clips->add_function("rl-goal-selection-start",
	                    sigc::slot<void, CLIPS::Value, std::string>(
	                      sigc::bind<0>(sigc::mem_fun(*this, &RLTestThread::rl_goal_selection),
	                                    env_name)));

	//clips->refresh_agenda();
	//clips->run();
	clips.unlock();
}

void
RLTestThread::clips_context_destroyed(const std::string &env_name)
{
	envs_.erase(env_name);
	logger->log_info(name(), "Removing environment %s", env_name.c_str());
}

fawkes::LockPtr<CLIPS::Environment>
RLTestThread::getClipsEnv(std::string env_name)
{
	fawkes::LockPtr<CLIPS::Environment> clips = envs_[env_name];
	return clips;
}

//ToDo return std::vector<string> literals
std::string
RLTestThread::create_rl_env_state_from_facts(std::string env_name)
{
	std::cout << "In create rl env state from facts" << std::endl;
	fawkes::LockPtr<CLIPS::Environment> clips = getClipsEnv(env_name);
	clips.lock();
	std::cout << "In create env state - locked clips" << std::endl;
	CLIPS::Fact::pointer fact             = clips->get_facts();
	std::string          env_state_string = "{";
	while (fact) {
		CLIPS::Template::pointer tmpl  = fact->get_template();
		std::size_t              found = tmpl->name().find("domain-fact");
		//std::size_t found2 = tmpl->name().find("wm-fact");//"predicate");
		//std::size_t found3 = tmpl->name().find("goal"); //"domain"

		if (found != std::string::npos) {
			std::vector<std::string> slot_names = fact->slot_names();
			std::string              fact_value = "";
			for (std::string s : slot_names) {
				fact_value += " Slot " + s + ": ";
				//std::cout << "Slot name: " + s << std::endl;
				std::vector<CLIPS::Value> slot_values = fact->slot_value(s);
				std::string               value       = "";
				for (std::size_t i = 0; i < slot_values.size(); i++) //for(CLIPS::Value v: slot_values)
				{
					auto v = slot_values[i];
					value += clipsValueToString(v);
					if (slot_values.size() > 1
					    && i != (slot_values.size() - 1)) //v != slot_values[slot_values.size()-1])
					{
						value += ",";
					}
				}

				if (s == "name") {
					env_state_string += "\"" + value + "(";
				}
				if (s == "param-values") {
					env_state_string += value + ")\","; //value.substr(0, value.length()-2) + "),";
				}
				fact_value += " " + value;
			}
			//std::cout << fact_value <<std::endl;
		}
		fact = fact->next();
	}
	env_state_string = env_state_string.substr(0, env_state_string.length() - 1) + "}";
	std::cout << env_state_string << std::endl;
	std::cout << "Finished passing all facts " << std::endl;
	clips.unlock();
	return env_state_string;
}

void
RLTestThread::rl_goal_selection(std::string env_name, CLIPS::Value parent_goal_id, std::string to)
{
	//get current env state from clips
	std::string facts = create_rl_env_state_from_facts(env_name); //todo save return value as obs
	bool        training_mode = true;
	if (!training_mode) {
		std::string nextAction = executeRlAgent(
		  facts); //ToDo pass obs and save return value /selected goal in a fact/return it to clips
		std::cout << nextAction << std::endl;
		fawkes::LockPtr<CLIPS::Environment> clips = getClipsEnv(env_name);
		clips.lock();
		clips->evaluate("(printout t \"Finished executeRlAgent asserting fact with next goal\" )");

		CLIPS::Value             v = CLIPS::Value(nextAction, CLIPS::TYPE_STRING);
		CLIPS::Template::pointer temp =
		  clips->get_template("rl-goal-selection"); //("rl-init-test-fact");
		CLIPS::Fact::pointer fact = CLIPS::Fact::create(**clips, temp);
		fact->set_slot("next-goal-id", v);
		clips->assert_fact(fact); //"(rl-init-test-fact )");
		clips.unlock();
	} else if (!startedTraining) {
		std::cout << "In rl_goal_selection - executing RL Agent is not active!" << std::endl;
		//trainingRlAgent();
		training_done   = std::async(std::launch::async, trainingRlAgent, config);
		startedTraining = true;
	} else {
		std::cout << "In rl_goal_selection - nothing to do" << std::endl;
	}
}

std::string
RLTestThread::clipsValueToString(CLIPS::Value v)
{
	std::string value = "";
	switch (v.type()) {
	case CLIPS::TYPE_FLOAT:
		// std::cout << v.as_float() << std::endl;
		value += std::to_string(v.as_float());
		break;

	case CLIPS::TYPE_INTEGER:
		//std::cout << v.as_integer() << std::endl;
		value += std::to_string(v.as_integer());
		break;

	default:
		//std::cout << v.as_string() <<std::endl;
		value += v.as_string();
	}
	return value;
}