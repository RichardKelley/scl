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
/* \file SRobotLink.hpp
 *
 *  Created on: Jul 22, 2010
 *
 *  Copyright (C) 2010
 *
 *  Author: Samir Menon <smenon@stanford.edu>
 */ 

#ifndef SROBOTLINK_HPP_
#define SROBOTLINK_HPP_

#include <string>
#include <vector>

#include <scl/DataTypes.hpp>

#include <scl/data_structs/SObject.hpp>

#include <Eigen/Dense>

#include <sutil/CMappedList.hpp>

namespace scl
{

/** The graphics specification for one rigid body. */
class SRobotLinkGraphics
{
public:
  // Eigen requires redefining the new operator for classes that contain fixed size Eigen member-data.
  // See http://eigen.tuxfamily.org/dox/StructHavingEigenMembers.html
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  Eigen::VectorXd pos_in_parent_;
  //NOTE TODO : Not using Eigen::Quaternion here because of weird Eigen dynamic_cast errors! Figure them out later.
  Eigen::VectorXd ori_parent_quat_; //x y z real
  Eigen::VectorXd scaling_; //Between 0 and 1. Default is 1.

  sInt collision_type_;//NOTE TODO : Should be an enum

  std::string file_name_;

  SRobotLinkGraphics()
  {
    pos_in_parent_.setZero(3);
    ori_parent_quat_.setZero(4);
    ori_parent_quat_(3) = 1;
    scaling_.setConstant(3,1);//x y z scaling. Default = 1
    collision_type_ = 0;
    file_name_ = "";
  }
};

/**
 * This structure contains all the information required to construct
 * a robot link. Each robot is completely defined by a tree of such 
 * links.
 */
struct SRobotLink : SObject
{
public:    
  // Eigen requires redefining the new operator for classes that contain fixed size Eigen member-data.
  // See http://eigen.tuxfamily.org/dox/StructHavingEigenMembers.html
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	//Constructor@End of the class:
	
	//***********************
  //Robot Branching data:
  
  //1. Identifiers
  std::string robot_name_;
  sInt link_id_; //Ids are not read in from a file. They are autogenerated by scl
  
  //2. Tree structure information: (Enables manual tree parsing)
  SRobotLink* parent_addr_;
  std::vector<SRobotLink*> child_addrs_;

  //***********************
  
  //***********************
  //2. Link Properties
  sBool is_root_;

	/***
	 * Position and orientation in the parent frame.
	 * And the transformation matrix from the parent
	 * frame to this frame
	 */
	Eigen::Vector3d pos_in_parent_;

	sRotationType ori_type_parent_;
	/** In files, Quaternion is stored in the form << x, y, z, w/real >> */
  Eigen::Quaternion<sFloat> ori_parent_quat_;

  Eigen::Matrix4d t_from_parent_;

  /***
   * Position and orientation in global coordinates
   * (wrt the ground frame)
   * And the transformation matrix from the global
   * frame to this frame
   */
  Eigen::Vector3d pos_global_;

  sRotationType ori_type_global_;
  Eigen::Quaternion<sFloat> ori_global_quat_;
  Eigen::AngleAxis<sFloat> ori_global_axis_angle_;

  Eigen::Matrix4d t_global_;

  /***
   * Mass and inertia properties
   */
  Eigen::Vector3d com_; //Center of mass
  sFloat mass_;
  Eigen::Vector3d inertia_;

  Eigen::Vector3d rot_axis_;
  sFloat rot_angle_;

  sInt link_is_fixed_;

  /****************************
   * 3. Joint information
   */
  std::string joint_name_;
  std::string parent_name_;
  sFloat joint_limit_lower_, joint_limit_upper_;
  sFloat joint_default_pos_;
  sJointType joint_type_;
  //***********************
  
  //****************
  //3. Graphics data
  std::vector<SRobotLinkGraphics> graphics_obj_vec_;
  sInt collision_type_;
  sRenderType render_type_;
  //****************

  //***********
  /** Constructor */
  SRobotLink();

  /** Sets the default parameter values */
	void init();
	//****************
};

}//end of namespace scl_parser

#endif /*ROBOTLINK_HPP_*/
