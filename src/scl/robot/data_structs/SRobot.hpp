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
/* \file SRobot.hpp
 *
 *  Created on: Dec 28, 2010
 *
 *  Copyright (C) 2010
 *
 *  Author: Samir Menon <smenon@stanford.edu>
 */

#ifndef SROBOT_HPP_
#define SROBOT_HPP_

#include <scl/DataTypes.hpp>

#include <scl/data_structs/SObject.hpp>
#include <scl/data_structs/SWorldParsedData.hpp>
#include <scl/data_structs/SRobotParsedData.hpp>
#include <scl/data_structs/SRobotIOData.hpp>

#include <scl/control/task/data_structs/STaskController.hpp>
#include <scl/control/gc/data_structs/SGcController.hpp>

#include <sutil/CMappedList.hpp>

namespace scl
{
  /**
   * This class encapsulates ALL the properties needed to simulate
   * and control a robot.
   *
   * It is used by CRobot objects.
   */
  class SRobot : public SObject
  {
  public:
    const SWorldParsedData* parsed_world_data_;
    SRobotParsedData * parsed_robot_data_;
    SRobotIOData* io_data_;

    /** The robot's controller data structures. */
    sutil::CMappedList<std::string,SControllerBase*> controllers_;

    /** The controller data structure */
    SControllerBase* controller_current_;

    /** Constructor sets stuff to null and initialization
     * state to false */
    SRobot();

    /** Destructor does nothing */
    virtual ~SRobot();
  };

}

#endif /* SROBOT_HPP_ */

