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
/* \file SGraphicsParsedData.hpp
 *
 *  Created on: Sep 5, 2010
 *
 *  Copyright (C) 2011
 *
 *  Author: Samir Menon <smenon@stanford.edu>
 */

#ifndef SGRAPHICSPARSEDDATA_HPP_
#define SGRAPHICSPARSEDDATA_HPP_

#include <vector>
#include <scl/DataTypes.hpp>
#include <scl/data_structs/SObject.hpp>

namespace scl
{
  /**
   * Contains all the static information for rendering a scenegraph.
   *
   * Set cameras, lights etc. here.
   */
  struct SGraphicsParsedData : public SObject
  {
    /** To position and orient a light */
    struct SLight
    {
      sFloat pos_[3];//The light's position.
      sFloat lookat_[3];//Point the light in a direction.
    };

    /** Camera settings for this world. */
    sFloat cam_pos_[3];//The camera's position.
    sFloat cam_lookat_[3];//Position the camera looks at.
    sFloat cam_up_[3];//The direction of the up-vector.
    sFloat cam_clipping_dist_[2];//Defines rendering limits along cam's line of sight (meters)

    sFloat background_color_[3];//RGB.

    /** Light source */
    std::vector<SLight> lights_;

    SGraphicsParsedData() : SObject(std::string("SGraphicsParsedData")){}
  };

}

#endif /* SGRAPHICSPARSEDDATA_HPP_ */
