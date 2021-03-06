/* This file is part of scl, a control and simulation library
for robots and biomechanical models.

scl is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

Alternatively, you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License, or (at your option) any later version.

scl is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License and a copy of the GNU General Public License along with
scl. If not, see <http://www.gnu.org/licenses/>.
*/
/* \file CTaskGcLimitCentering.cpp
 *
 *  Created on: Mar 15, 2013
 *
 *  Copyright (C) 2013
 *
 *  Author: Samir Menon <smenon@stanford.edu>
 */

#include "CTaskGcLimitCentering.hpp"

#include <iostream>
#include <stdexcept>

namespace scl
{

  CTaskGcLimitCentering::CTaskGcLimitCentering() : CTaskBase(), data_(S_NULL)
  { }

  //************************
  // Inherited stuff
  //************************
  bool CTaskGcLimitCentering::init(STaskBase* arg_task_data,
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

      data_ = dynamic_cast<STaskGcLimitCentering*>(arg_task_data);
      dynamics_ = arg_dynamics;

      //Leaves nothing for other tasks. Uses up the entire remaining range space.
      data_->J_.setIdentity(data_->robot_->dof_,data_->robot_->dof_);
      data_->J_dyn_inv_.setIdentity(data_->robot_->dof_,data_->robot_->dof_);
      data_->null_space_.setZero(data_->robot_->dof_,data_->robot_->dof_);

      //Max position shouldn't be greater than the min position.
      if( (arg_task_data->robot_->gc_pos_limit_max_.array() <=
           arg_task_data->robot_->gc_pos_limit_min_.array()).any())
      { throw(std::runtime_error(std::string("A joint's max position <= min position : "))); }

      data_->q_goal_ = arg_task_data->robot_->gc_pos_limit_min_ +
          (arg_task_data->robot_->gc_pos_limit_max_ -
          arg_task_data->robot_->gc_pos_limit_min_)*0.5;
      std::cout<<"\nGc Max = "<<arg_task_data->robot_->gc_pos_limit_max_.transpose();
      std::cout<<"\nGc Min = "<<arg_task_data->robot_->gc_pos_limit_min_.transpose();
      std::cout<<"\nGc Centering Goal Position : "<<data_->q_goal_.transpose();

      has_been_init_ = true;
    }
    catch(std::exception& e)
    {
      std::cerr<<"\nCTaskGcLimitCentering::init() :"<<e.what();
      has_been_init_ = false;
    }
    return has_been_init_;
  }

  STaskBase* CTaskGcLimitCentering::getTaskData()
  { return data_; }

  /** Sets the current goal position */
  bool CTaskGcLimitCentering::setGoalPos(const Eigen::VectorXd & arg_goal)
  {
    if((data_->dof_task_ == arg_goal.cols() && 1 == arg_goal.rows()) ||
        (1 == arg_goal.cols() && data_->dof_task_ == arg_goal.rows()) )
    {
      data_->q_goal_ = arg_goal;
      return true;
    }
#ifdef DEBUG
    else
    {
      std::cerr<<"\nCTaskGcLimitCentering::setGoalPos() : Error : Goal vector's size != dof"<<std::flush;
      assert(false);
    }
#endif
    return false;
  }

  void CTaskGcLimitCentering::reset()
  {
    data_ = S_NULL;
    dynamics_ = S_NULL;
    has_been_init_ = false;
  }


  bool CTaskGcLimitCentering::computeServo(const SRobotSensors* arg_sensors)
  {
#ifdef DEBUG
    assert(has_been_init_);
    assert(S_NULL!=dynamics_);
#endif
    if(data_->has_been_init_)
    {
      data_->q_ = arg_sensors->q_;
      data_->dq_ = arg_sensors->dq_;

      //Compute the servo torques
      Eigen::VectorXd tmp1, tmp2;

      tmp1 = (data_->q_goal_ - data_->q_);
      tmp1 =  data_->kp_.array() * tmp1.array();

      tmp2 = -1* data_->kv_.array() * data_->dq_.array();

      //Obtain force to be applied to a unit mass floating about
      //in space (ie. A dynamically decoupled mass).
      data_->force_task_ = tmp2 + tmp1;

      data_->force_task_ = data_->force_task_.array().min(data_->force_task_max_.array());//Min of self and max
      data_->force_task_ = data_->force_task_.array().max(data_->force_task_min_.array());//Max of self and min

      // NOTE : We subtract gravity (since we want to apply an equal and opposite force
      data_->force_gc_ =  data_->gc_model_->M_gc_ * data_->force_task_ - data_->gc_model_->force_gc_grav_;
      return true;
    }
    else
    { return false; }
  }


  /** Sets the null space for the next level to zero. Ie.
   * any task below this one in the hierarchy is ignored. */
  bool CTaskGcLimitCentering::computeModel(const SRobotSensors* arg_sensors)
  {//Null space is set to zero by default and doesn't need to be set again.
    if(data_->has_been_init_)
    { return true; }
    else
    { return false; }
  }

  //************************
  // Task specific stuff
  //************************
  bool CTaskGcLimitCentering::achievedGoalPos()
  {
    sFloat dist;
    dist = fabs((data_->q_goal_ - data_->q_).norm());

    if(dist > data_->spatial_resolution_)
    { return false; }
    else
    { return true;  }
  }

}
