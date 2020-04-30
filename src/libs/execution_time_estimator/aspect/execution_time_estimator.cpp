/***************************************************************************
 *  execution_time_estimator.cpp - Aspect for a running time provider
 *
 *  Created: Thu 12 Dec 2019 19:03:19 CET 19:03
 *  Copyright  2019  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
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

#include "execution_time_estimator.h"

#include <core/exception.h>
#include <core/exceptions/software.h>

#include <algorithm>

namespace fawkes {

/** @class ExecutionTimeEstimatorManager
 * A manager for a vector of running time providers for skill simulation.
 */

/** Get the running time provider for the given skill string.
 * @param skill_string The string to get the running time for
 * @return a pointer to the provider
 * @throws IllegalArgumentException if no provider for the given skill exists
 */
std::shared_ptr<ExecutionTimeEstimator>
ExecutionTimeEstimatorManager::get_provider(const std::string &skill_string) const
{
	for (auto &provider : execution_time_estimators_) {
		if (provider->can_execute(skill_string)) {
			return std::shared_ptr<ExecutionTimeEstimator>(provider);
		}
	}
	throw IllegalArgumentException("No provider found for %s", skill_string.c_str());
}

/** Add a running time provider.
 * @param provider The provider to add
 */
void
ExecutionTimeEstimatorManager::register_provider(std::shared_ptr<ExecutionTimeEstimator> provider)
{
	execution_time_estimators_.push_back(provider);
}

/** Remove an execution time estimate provider.
 * @param provider The provider to remove
 */
void
ExecutionTimeEstimatorManager::unregister_provider(std::shared_ptr<ExecutionTimeEstimator> provider)
{
	execution_time_estimators_.erase(std::remove(execution_time_estimators_.begin(),
	                                             execution_time_estimators_.end(),
	                                             provider),
	                                 execution_time_estimators_.end());
}

/** @class ExecutionTimeEstimatorsAspect
 * An aspect to give access to the skiller simulator's running time providers.
 * Use this aspect to add a running time provider.
 *
 * @var ExecutionTimeEstimatorsAspect::execution_time_estimator_manager_
 * The ExecutionTimeEstimatorManager that is used to manage the estimators.
 * @see ExecutionTimeEstimatorManager
 */

/** Constructor. */
ExecutionTimeEstimatorsAspect::ExecutionTimeEstimatorsAspect()
: execution_time_estimator_manager_(nullptr)
{
	add_aspect("SkillExecutionTimeEstimatorAspect");
}

/** Initialize the aspect with a provider manager.
 * @param provider_manager The manager of the running time providers
 */
void
ExecutionTimeEstimatorsAspect::init_ExecutionTimeEstimatorsAspect(
  ExecutionTimeEstimatorManager *provider_manager)
{
	execution_time_estimator_manager_ = provider_manager;
}

/** Finalize the aspect. */
void
ExecutionTimeEstimatorsAspect::finalize_ExecutionTimeEstimatorsAspect()
{
}

} // namespace fawkes
