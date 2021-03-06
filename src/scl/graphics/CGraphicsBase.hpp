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
/* \file CGraphicsBase.hpp
 *
 *  Created on: Aug 27, 2010
 *
 *  Copyright (C) 2010
 *
 *  Author: Samir Menon <smenon@stanford.edu>
 */

#ifndef CGRAPHICSBASE_HPP_
#define CGRAPHICSBASE_HPP_

#include <scl/DataTypes.hpp>
#include <scl/data_structs/SObject.hpp>

#include <string>

#include <Eigen/Eigen>

namespace scl
{

struct SRigidBody;
struct SRobotSensors;

class CGraphicsBase : public SObject
{
public:
  /** Default constructor sets initialization state to false */
  CGraphicsBase() : SObject("CGraphicsBase") {}

  /** Default destructor does nothing. */
  virtual ~CGraphicsBase(){}

  /********************************************************
    *   Initialization Functions: Sets up a scenegraph, camera
    *   etc..
    *********************************************************/
   /** Initializes a graphics world using the camera information
    * in the passed data structure.
    *
    * Returns,
    * true  : Successfully created a world
    * false : Failed (or the data structure was invalid). */
   virtual sBool initGraphics(
       /** The graphics data structure loaded from the xml file. Specifies camera etc. information. */
       const SGraphicsParsed* arg_gr_ds)=0;

   /** Initialization state */
   virtual sBool hasBeenInit() {  return has_been_init_;  }

   /** Deallocates the graphics and resets all associated variables.. */
   virtual sBool destroyGraphics()=0;

   /** Adds a robot's meshes to the graphics rendering environment.
    *
    * A robot is defined as:
    * 1. Anything whose dynamics are integrated by the physics simulator
    * 2. Any real world entity subject to the laws of physics */
   virtual sBool addRobotToRender(
       /** The static robot specification */
       const SRobotParsed *arg_rob_parsed,
       /** The generalized coordinates etc. */
       const SRobotIO* arg_rob_io)=0;

   /** Removes a robot's meshes from the graphics rendering environment.
    *
    * A robot is defined as:
    * 1. Anything whose dynamics are integrated by the physics simulator
    * 2. Any real world entity subject to the laws of physics */
   virtual sBool removeRobotFromRender(
       const std::string& arg_robot)=0;

   /** Adds a static mesh to render. Indexed by its name.
    *
    * A mesh is defined as anything that DOESN"T obey the laws of
    * physics. It is merely rendered (possibly with collision etc). */
   virtual sBool addMeshToRender(
       const std::string& arg_mesh_name,
       const std::string& arg_mesh_file,
       const Eigen::Vector3d& arg_pos,
       const Eigen::Matrix3d& arg_rot)=0;

   /** Removes a static mesh from the rendered scene. Indexed by its name.
    *
    * A mesh is defined as anything that DOESN"T obey the laws of
    * physics. It is merely rendered (possibly with collision etc). */
   virtual sBool removeMeshFromRender(const std::string& arg_mesh_name)=0;

   /** Adds a muscle system to the graphics rendering environment
    *
    * A muscle system is defined as:
    * 1. A set of line segments, with connection points on a robot links certain links.
    * 2. A parent robot to whose links the muscles attach. */
   virtual sBool addMusclesToRender(
       /** The robot to which the muscles will be attached */
       const std::string& arg_robot,
       /** The set of muscles to be attached */
       const SActuatorSetMuscleParsed& arg_mset,
       const sBool add_musc_via_points)=0;

   /** Removes a muscle system from the graphics rendering environment
    *
    * A muscle system is defined as:
    * 1. A set of line segments, with connection points on a robot links certain links.
    * 2. A parent robot to whose links the muscles attach. */
   virtual sBool removeMusclesFromRender(
       const std::string& arg_robot,
       const std::string& arg_mset)=0;


   /********************************************************
    *   Rendering Functions: Will actually render the scene
    *   A set of OpenGL calls.
    *********************************************************/
   /** Updates the rotation and translation transformations
    * for all the robots' associated CGenerericObject branching
    * representations. */
   virtual sBool updateGraphics()=0;

   /** Updates the rotation and translation transformations
    * for all associated robots */
   virtual sBool updateGraphicsForRobots()=0;

   /** Updates the rotation and translation transformations
    * for all meshes. */
   virtual bool updateGraphicsForMeshes()=0;

   /** Updates the rotation and translation transformations
    * for all muscles systems. */
   virtual bool updateGraphicsForMuscles()=0;
};

}

#endif /* CGRAPHICSBASE_HPP_ */
