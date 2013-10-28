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
/* \file CTaskOpPosPIDA1OrderInfTime.hpp
 *
 *  Created on: Aug 9, 2013
 *
 *  Copyright (C) 2013
 *
 *  Author: Samir Menon <smenon@stanford.edu>
 */

#ifndef COPPOINTTASKPIDA1ORDERINFTIME_HPP_
#define COPPOINTTASKPIDA1ORDERINFTIME_HPP_

#include <string>
#include <vector>

#include <scl/DataTypes.hpp>

#include <scl/control/task/tasks/data_structs/STaskOpPosPIDA1OrderInfTime.hpp>

#include <scl/control/task/CTaskBase.hpp>

#include <Eigen/Dense>
#include <Eigen/SVD>


namespace scl
{
  /** Computes the operational space  forces for a single
   * 3-d (x,y,z) goal point Euclidean task
   *
   * It computes:
   *
   * 1. The task model (computes, mass, jacobian, inv jacobian,
   * coriolis, centrifugal and gravity matrices/vectors).
   *
   * 2. The task servo (computes the dynamically decoupled task
   * forces and the torques. uses the task model to do so).
   *
   * NOTE :
   * This differs from the plain operational space controller in
   * that it also incorporates integral gain. For many applications,
   * integral gain is actually not desirable since it introduces
   * a non-deterministic time-varying control signal to the equations.
   * To make this work well, one might have to tune gains manually
   * depending on the motor task involved.
   *
   * Use with caution.
   */
class CTaskOpPosPIDA1OrderInfTime : public scl::CTaskBase
{
public:
  /********************************
   * CTaskBase API
   *********************************/
  /** Computes the task torques */
  virtual bool computeServo(const SRobotSensors* arg_sensors);

  /** Computes the dynamics (task model)
   * Assumes that the data_->model_.gc_model_ has been updated. */
  virtual bool computeModel();

  /* **************************************************************
   *                   Status Get/Set Functions
   * ************************************************************** */
  /** Return this task controller's task data structure.*/
  virtual STaskBase* getTaskData();

  /** Sets the current goal position */
  virtual bool setGoalPos(const Eigen::VectorXd & arg_goal);

  /** Sets the current goal velocity */
  virtual bool setGoalVel(const Eigen::VectorXd & arg_goal);

  /** Sets the current goal acceleration */
  virtual bool setGoalAcc(const Eigen::VectorXd & arg_goal);

  /** Gets the current goal position. Returns false if not supported by task. */
  virtual bool getGoalPos(Eigen::VectorXd & arg_goal) const
  { arg_goal = data_->x_goal_; return true; }

  /** Gets the current goal velocity. Returns false if not supported by task. */
  virtual bool getGoalVel(Eigen::VectorXd & arg_goal) const
  { arg_goal = data_->dx_goal_; return true; }

  /** Gets the current goal acceleration. Returns false if not supported by task. */
  virtual bool getGoalAcc(Eigen::VectorXd & arg_goal) const
  { arg_goal = data_->ddx_goal_; return true; }

  /** Gets the current position. Returns false if not supported by task. */
  virtual bool getPos(Eigen::VectorXd & arg_pos) const
  { arg_pos = data_->x_; return true; }

  /** Gets the current velocity. Returns false if not supported by task. */
  virtual bool getVel(Eigen::VectorXd & arg_vel) const
  { arg_vel = data_->dx_; return true; }

  /** Gets the current acceleration. Returns false if not supported by task. */
  virtual bool getAcc(Eigen::VectorXd & arg_acc) const
  { arg_acc = data_->ddx_; return true; }

  /* *******************************
   * CTaskOpPosPID specific functions
   * ******************************* */
  /** Whether the task has achieved its goal position. */
  sBool achievedGoalPos();

  void setFlagComputeOpPosGravity(sBool arg_compute_grav)
  { flag_compute_gravity_ = arg_compute_grav; }

  /** Whether the integral gain has activated. */
  sBool integralGainActive()
  { return flag_integral_gain_active_;  }

  /* *******************************
   * CTaskOpPos specific functions
   * ******************************* */
  /** Default constructor : Does nothing   */
  CTaskOpPosPIDA1OrderInfTime();

  /** Default destructor : Does nothing.   */
  virtual ~CTaskOpPosPIDA1OrderInfTime(){}

  /** Initializes the task object. Required to set output
   * gc force dofs */
  virtual bool init(STaskBase* arg_task_data,
      CDynamicsBase* arg_dynamics);

  /** Resets the task by removing its data.
   * NOTE : Does not deallocate its data structure*/
  virtual void reset();

protected:
  /** The actual data structure for this computational object */
  STaskOpPosPIDA1OrderInfTime* data_;

  /** Temporary variables */
  Eigen::VectorXd tmp1, tmp2;

  /** For inverting the lambda matrix (when it gets singular) */
  Eigen::ColPivHouseholderQR<Eigen::Matrix3d> qr_;

  /** True when the lambda_inv matrix turns singular. */
  sBool lambda_inv_singular_;

  /** For inverting the operational space inertia matrix
   * near singularities. 3x3 for operational point tasks. */
  Eigen::JacobiSVD<Eigen::Matrix3d > svd_;
  Eigen::Matrix3d singular_values_;

  sBool flag_compute_gravity_;
  sBool flag_integral_gain_active_;
};

}

#endif /* COPPOINTTASKPIDA1ORDERINFTIME_HPP_ */
