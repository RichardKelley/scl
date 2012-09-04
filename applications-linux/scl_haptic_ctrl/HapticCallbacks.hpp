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
/* \file HapticCallbacks.hpp
 *
 *  Created on: Sep 16, 2011
 *
 *  Copyright (C) 2011
 *
 *  Author: Samir Menon <smenon@stanford.edu>
 */

#ifndef HAPTICCALLBACKS_HPP_
#define HAPTICCALLBACKS_HPP_

#include <scl/Singletons.hpp>

#include <sutil/CRegisteredPrintables.hpp>

#include <scl/robot/GenericCallbacks.hpp>
#include <scl/robot/GenericPrintables.hpp>

#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>

namespace scl_app
{
  /** This function registers all the console io callbacks */
  bool registerCallbacks()
  {
    bool flag;
    try
    {
      /** ************************************************************************
       * Add a help function to the command line shell
       * *************************************************************************/
      flag = sutil::callbacks::add<scl::CCallbackHelp, std::string, std::vector<std::string> >(
          std::string("help") );
      if(false == flag){throw(std::runtime_error("Could not add a help callback"));  }

      /** ************************************************************************
       * Add an echo function to the command line shell
       * *************************************************************************/
      flag = sutil::callbacks::add<scl::CCallbackEcho, std::string, std::vector<std::string> >(
          std::string("echo") );
      if(false == flag){throw(std::runtime_error("Could not add an echo callback"));  }

      /** ************************************************************************
       * Add a print callback. NOTE : You also need to add printables to print
       * *************************************************************************/
      flag = sutil::callbacks::add<scl::CCallbackPrint, std::string, std::vector<std::string> >(
          std::string("print") );
      if(false == flag){throw(std::runtime_error("Could not add a print callback"));  }

      /** ************************************************************************
       * Add all the printables : Stuff that can be printed by the print callback.
       * *************************************************************************/
      flag = scl::addRobotPrintables();
      if(false == flag){throw(std::runtime_error("Could not add callbacks to print robot info"));  }
    }
    catch(std::exception &e)
    {
      std::cout<<"\nregisterCallbacks() : Error: "<<e.what();
      return false;
    }
    return true;
  }
}

#endif /* HAPTICCALLBACKS_HPP_ */