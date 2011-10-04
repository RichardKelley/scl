/* This file is part of Busylizzy, a control and simulation library
for robots and biomechanical models.

Busylizzy is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

Alternatively, you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License, or (at your option) any later version.

Busylizzy is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License and a copy of the GNU General Public License along with
Busylizzy. If not, see <http://www.gnu.org/licenses/>.
*/
/* \file CTaskGc.cpp
 *
 *  Created on: Jul 23, 2010
 *
 *  Copyright (C) 2010
 *
 *  Author: Samir Menon <smenon@stanford.edu>
 */

#include "CTaskGc.hpp"

#include <iostream>
#include <stdexcept>

namespace scl
{

  CTaskGc::CTaskGc() : CTaskBase(), data_(S_NULL)
  { }

  //************************
  // Inherited stuff
  //************************
  bool CTaskGc::init(STaskBase* arg_task_data,
      CDynamicsBase* arg_dynamics)
  {
    try
    {
      if(S_NULL == arg_task_data)
      { throw(std::runtime_error("Passed a null task data structure"));  }

      if(false == arg_task_data->has_been_init_)
      { throw(std::runtime_error("Passed an uninitialized task data structure"));  }

      if(S_NULL == arg_dynamics)
      { throw(std::runtime_error("Passed a null dynamics object"));  }

      if(false == arg_dynamics->hasBeenInit())
      { throw(std::runtime_error("Passed an uninitialized dynamics object"));  }

      data_ = dynamic_cast<STaskGc*>(arg_task_data);
      dynamics_ = arg_dynamics;

      //Leaves nothing for other tasks. Uses up the entire remaining range space.
      data_->null_space_.setZero(data_->robot_->dof_,data_->robot_->dof_);

      has_been_init_ = true;
    }
    catch(std::exception& e)
    {
      std::cerr<<"\nCTaskGc::init() :"<<e.what();
      has_been_init_ = false;
    }
    return has_been_init_;
  }

  STaskBase* CTaskGc::getTaskData()
  { return data_; }

  void CTaskGc::reset()
  {
    data_ = S_NULL;
    dynamics_ = S_NULL;
    has_been_init_ = false;
  }


  bool CTaskGc::computeServo(const SRobotSensorData* arg_sensors)
  {
#ifdef W_TESTING
    assert(has_been_init_);
    assert(S_NULL!=dynamics_);
#endif
    if(data_->has_been_init_)
    {

      //Compute the servo torques
      Eigen::VectorXd tmp1, tmp2;

      tmp1 = (data_->q_goal_ - arg_sensors->q_);
      tmp1 =  data_->kp_.array() * tmp1.array();

      tmp2 = (data_->dq_goal_ - arg_sensors->dq_);
      tmp2 = data_->kv_.array() * tmp2.array();

      //Obtain force to be applied to a unit mass floating about
      //in space (ie. A dynamically decoupled mass).
      data_->force_task_ = data_->ddq_goal_ + tmp2 + tmp1;

      data_->force_task_ = data_->force_task_.array().min(data_->force_task_max_.array());//Min of self and max
      data_->force_task_ = data_->force_task_.array().max(data_->force_task_min_.array());//Max of self and min

      data_->force_gc_ =  data_->gc_model_->A_ * data_->force_task_ + data_->gc_model_->g_;
      return true;
    }
    else
    { return false; }
  }


  /** Sets the null space for the next level to zero. Ie.
   * any task below this one in the hierarchy is ignored. */
  bool CTaskGc::computeModel()
  {
    if(data_->has_been_init_)
    {
      return true;
    }
    else
    { return false; }
  }

}
