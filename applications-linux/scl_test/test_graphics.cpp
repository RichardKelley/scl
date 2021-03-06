/* This file is part of scl, a control and simulation library
for robots and biomechanical models.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
/* \file test_graphics.cpp
 *
 *  Copyright (C) 2010
 *
 *  Author: Samir Menon <smenon@stanford.edu>
 */

#include "test_graphics.hpp"

#include <scl/DataTypes.hpp>
#include <scl/Singletons.hpp>
#include <scl/robot/DbRegisterFunctions.hpp>
#include <scl/parser/sclparser/CParserScl.hpp>
#include <scl/graphics/chai/CGraphicsChai.hpp>
#include <scl/graphics/chai/ChaiGlutHandlers.hpp>

#include <sutil/CSystemClock.hpp>
#include <Eigen/Dense>

#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <math.h>

#include <GL/freeglut.h>

using namespace scl;
using namespace std;

namespace scl_test
{
  /**
   * Tests the (chai) graphics subsystem
   *
   * Reads in a toy robot specification and renders it.
   */
  void test_graphics(int id, const std::string &file_name,
      int argc, char **argv)
  {
    scl::sUInt r_id=0;
    try
    {
      //0. Create vars
      long long i; //Counters
      long long imax; //Counter limits
      sClock t1,t2; //Clocks: pre and post
      bool flag;

      // Test database
      scl::SDatabase* db = scl::CDatabase::getData();
      if(S_NULL==db)
      { throw(std::runtime_error("Database not initialized"));  }
      else
      { std::cout<<"\nTest Result ("<<r_id++<<")  Initialized database"<<std::flush;  }

      scl::CDatabase::getData()->dir_specs_ = "../../specs/";
      /******************************Parsing************************************/

      //1. Create robot from a file specification (And register it with the db)
      std::vector<std::string> robot_names;
      scl::SRobotParsed* rob_ds;
      scl::CParserScl tmp_lparser;
      flag = tmp_lparser.listRobotsInFile(file_name,robot_names);
      if(false == flag)
      { throw(std::runtime_error("Could not read robot names from the file"));  }
      else
      {std::cout<<"\nTest Result ("<<r_id++<<")  Creating a robot specification for "
        <<robot_names[0]<<" in the database"<<std::flush;}


      flag = scl_registry::parseRobot(file_name,
          robot_names[0], &tmp_lparser);

      if(false == flag)
      { throw(std::runtime_error("Could not register robot with the database"));  }
      else
      {
        std::cout<<"\nTest Result ("<<r_id++<<")  Created a robot "
            <<robot_names[0]<<" in the database"<<std::flush;
      }

      //1.b. Pull out the robot's ds from the db
      rob_ds = db->s_parser_.robots_.at(robot_names[0]);
      if(S_NULL == rob_ds)
      { throw(std::runtime_error("Could not find robot in the database"));  }
      else
      {
        std::cout<<"\nTest Result ("<<r_id++<<")  Found robot "
            <<robot_names[0]<<" in the database"<<std::flush;
      }

      scl::SRobotIO* rob_io_ds;
      rob_io_ds = db->s_io_.io_data_.at(robot_names[0]);
      if(S_NULL == rob_io_ds)
      { throw(std::runtime_error("Robot I/O data structure does not exist in the database"));  }
      else
      {
        std::cout<<"\nTest Result ("<<r_id++<<")  Found robot I/O data structure "
            <<robot_names[0]<<" in the database"<<std::flush;
      }

      //Ok. Now we have a robot's specification specification parsed into
      //the database. Lets move on.

      //2. Read in the graphics spec from a file...
      std::vector<std::string> graphics_names;
      flag = tmp_lparser.listGraphicsInFile(file_name,graphics_names);
      if(false == flag)
      { throw(std::runtime_error("Could not read graphics names from the file"));  }
      else
      {std::cout<<"\nTest Result ("<<r_id++<<")  Creating a graphics specification for "
        <<graphics_names[0]<<" in the database"<<std::flush;}

      flag = scl_registry::parseGraphics(file_name,
          graphics_names[0], &tmp_lparser);

      if(false == flag)
      { throw(std::runtime_error("Could not register graphics with the database"));  }
      else
      {
        std::cout<<"\nTest Result ("<<r_id++<<")  Created a graphics "
            <<graphics_names[0]<<" in the database"<<std::flush;
      }

      //2.b. Pull out the graphics's ds from the db
      SGraphicsParsed * gr_ds = db->s_parser_.graphics_worlds_.at(graphics_names[0]);
      if(S_NULL==gr_ds)
      {
        throw(std::runtime_error(
            "Could not find any graphics specification in the database. Should have been parsed by now."));
      }
      else
      {
        std::cout<<"\nTest Result ("<<r_id++<<")  Found graphics specification "
            <<gr_ds->name_<<" in the database"<<std::flush;
      }

      scl::SGraphicsChai *chai_ds = db->s_gui_.chai_data_.at(graphics_names[0]);
      if(S_NULL==chai_ds)
      {
        throw(std::runtime_error(
            "Could not find any chai object in the database. Should have been parsed by now."));
      }
      else
      {
        std::cout<<"\nTest Result ("<<r_id++<<")  Found chai object "
            <<chai_ds->name_<<" in the database"<<std::flush;
      }


      /******************************Chai Initialization************************************/
      //3. Initialize a chai graphics object
      CGraphicsChai chai_gr;
      flag = chai_gr.initGraphics(gr_ds,chai_ds);
      if(false==flag)
      { throw(std::runtime_error("Couldn't initialize chai graphics")); }
      else
      { std::cout<<"\nTest Result ("<<r_id++<<")  Initialized chai graphics"<<std::flush;  }

      //5. Add a robot to render
      flag = chai_gr.addRobotToRender(rob_ds,rob_io_ds);
      if(false==flag)
      { throw(std::runtime_error("Couldn't find robot to render")); }
      else
      { std::cout<<"\nTest Result ("<<r_id++<<")  Initialized robot "<<robot_names[0]<<" to render"<<std::flush;  }

      //-----------------------------------------------------------------------
      // OPEN GL - WINDOW DISPLAY
      //-----------------------------------------------------------------------
      // initialize GLUT
      if(1!=glutGet(GLUT_INIT_STATE))
      {
        glutInit(&argc, argv);
        db->s_gui_.glut_initialized_ = true;
      }

      if(false == scl_chai_glut_interface::initializeGlutForChai(gr_ds->name_, &chai_gr))
      { throw(std::runtime_error("Glut initialization error")); }

      //6. Test graphics rendering speed.
      imax = 100;
      t1 = sutil::CSystemClock::getSysTime();
      for(i=0; i<imax; ++i)
      {
        chai_gr.updateGraphics();
      }
      t2 = sutil::CSystemClock::getSysTime();
      std::cout<<"\nTest Result ("<<r_id++<<") "<<imax<<" graphics updates."
          <<" Time: "<<t2-t1<<std::flush;

      //-----------------------------------------------------------------------
      // START SIMULATION
      //-----------------------------------------------------------------------
      // Start the main graphics rendering loop
      t1 = sutil::CSystemClock::getSysTime();
      for(int glIter=0;glIter<imax;glIter++)
      { glutMainLoopEvent();  }
      t2 = sutil::CSystemClock::getSysTime();
      std::cout<<"\nTest Result ("<<r_id++<<") "<<imax<<" graphics+glut updates."
          <<" Time: "<<t2-t1<<std::flush;

      std::cout<<"\nTest #"<<id<<" : Succeeded.";
    }
    catch(std::exception & e)
    {
      std::cout<<"\nTest Result ("<<r_id++<<") "<< e.what();
      std::cout<<"\nTest #"<<id<<" Failed.";
    }
  }
}
