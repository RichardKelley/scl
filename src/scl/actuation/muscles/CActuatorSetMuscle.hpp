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
/* \file CActuatorSetMuscle.hpp
 *
 *  Created on: Jul 21, 2013
 *
 *  Copyright (C) 2013
 *
 *  Author: Samir Menon <smenon@stanford.edu>
 */

#ifndef CACTUATORSETMUSCLE_HPP_
#define CACTUATORSETMUSCLE_HPP_

#include <scl/data_structs/SRobotParsed.hpp>
#include <scl/data_structs/SRobotIO.hpp>

#include <scl/dynamics/CDynamicsBase.hpp>

#include <scl/actuation/CActuatorSetBase.hpp>
#include <scl/actuation/muscles/CActuatorMuscle.hpp>
#include <scl/actuation/muscles/data_structs/SActuatorSetMuscle.hpp>

#include <Eigen/Eigen>

#include <string>

namespace scl
{
  /** Models a muscle as a linear actuator with zero
   * force generation delay and a stiff tendon, which
   * eliminates slack.
   *
   * This class combines a set of muscles into
   */
  class CActuatorSetMuscle : public CActuatorSetBase
  {
  public:
    /* *****************************************************************
     *                        Actuator Kinematics
     * ***************************************************************** */
    /** Some actuator sets don't directly actuate the generalized coordinates
     * and require a Jacobian to compute their contribution to the generalized
     * forces.
     *
     * Each actuator instance implements a row in the Jacobian. The actuator
     * set merely collates them. */
    virtual sBool computeJacobian(
        const Eigen::VectorXd arg_q,
        Eigen::MatrixXd &ret_J);

    /* *****************************************************************
     *                        Actuator Dynamics
     * ***************************************************************** */
    /** Set the actuator commands. These will be translated into individual
     * actuator commands.
     * Gets the output of the actuator. Returned in the passed variable.
     *
     * arg_input : The actuator input (possibly arbitrary coordinates)
     * arg_state : Possibly actuator coordinates, velocities etc. Rows in matrix.
     * ret_output_force : The actuator forces such that:
     *                       Fgc = J' * ret_output_force;
     */
    virtual sBool computeDynamics(const Eigen::VectorXd& arg_input,
        const Eigen::MatrixXd& arg_state,
        Eigen::VectorXd& ret_output_force)
    { /** TODO : Implement hill-type muscles */ return false; }

    /* *****************************************************************
     *                           Accessors
     * ***************************************************************** */
    /** To simplify the confusion between name idx and numeric idx */
    int getNumberOfMuscles()
    { return muscles_.size(); }

    /** Access the data structure */
    SActuatorSetMuscle* getData()
    { return data_; }

    /** Access the data structure for reads only */
    const SActuatorSetMuscle* getData() const
    { return static_cast<const SActuatorSetMuscle*>(data_); }

    /* *****************************************************************
     *                        Initialization
     * ***************************************************************** */
    /** Initializes the actuators. This involves determining whether the muscle
     * matches the robot etc. It also sets up the Jacobians to be computed etc.
     */
    virtual sBool init(
        SActuatorSetMuscle &arg_mset,
        SGcModel &arg_rgcm,
        CDynamicsBase *arg_dynamics);

    /** Has this actuator been initialized */
    virtual sBool hasBeenInit();

    /* *****************************************************************
     *                        Constructors
     * ***************************************************************** */

    /** Default constructor. Sets stuff to NULL. */
    CActuatorSetMuscle();

    /** Default destructor. Does nothing */
    virtual ~CActuatorSetMuscle(){}

  protected:
    /** The muscle set data */
    SActuatorSetMuscle *data_;

    /** The muscles in this set */
    sutil::CMappedList<std::string,CActuatorMuscle> muscles_;

    /** Dynamics specification (to compute robot Jacobians) */
    CDynamicsBase *dynamics_;

    /** Temporary object used while computing the Jacobian */
    Eigen::VectorXd row_J_;
  };

} /* namespace scl */
#endif /* CACTUATORSETMUSCLE_HPP_ */
