/* This file is part of scl, a control and simulation library
for robots and biomechanical models.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
/* \file test_dynamics.cpp
 *
 *  Copyright (C) 2010
 *
 *  Author: Samir Menon <smenon@stanford.edu>
 */

#include "test_dynamics.hpp"

#include <sutil/CSystemClock.hpp>

#include <scl/DataTypes.hpp>
#include <scl/Singletons.hpp>
#include <scl/robot/DbRegisterFunctions.hpp>
#include <scl/parser/sclparser/CParserScl.hpp>

#include <scl/dynamics/analytic/CDynamicsAnalyticRPP.hpp>
#include <scl_ext/dynamics/scl_spatial/CDynamicsSclSpatial.hpp>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>
#include <cmath>
#include <stdio.h>
#include <math.h>

#include <Eigen/Dense>

using namespace scl;
using namespace std;

namespace scl_test
{
  /** Tests the performance of analytical dynamics implementations in scl. */
  void test_dynamics_scl_sp_vs_analytic_rpp(int id)
  {
    scl_ext::CDynamicsSclSpatial * dynamics = S_NULL;
    scl::sUInt r_id=0;
    bool flag;

    const double test_precision = 0.00001;

    try
    {
      //0. Create vars
      long long i; //Counters
      long long imax; //Counter limits
      sClock t1,t2; //Clocks: pre and post

      //Test database
      scl::SDatabase * db = scl::CDatabase::getData();
      if(S_NULL==db)
      { throw(std::runtime_error("Database not initialized."));  }
      else
      { std::cout<<"\nTest Result ("<<r_id++<<")  Initialized database"<<std::flush;  }

      db->dir_specs_ = scl::CDatabase::getData()->cwd_ + std::string("../../specs/");

      //0. Parse the file for robots
      std::string tmp_infile;
      tmp_infile = db->dir_specs_ + "Bot-RPP/Bot-RPPCfg.xml";
      std::cout<<"\nTest Result ("<<r_id++<<")  Opening file : "<<tmp_infile;

      scl::CParserScl tmp_lparser;

      //1 Create robot from the file specification (And register it with the db)
      std::string robot_name = "rppbot";
      flag = scl_registry::parseRobot(tmp_infile, robot_name, &tmp_lparser);
      if(false == flag)
      { throw(std::runtime_error("Could not register robot with the database"));  }
      else
      {
        std::cout<<"\nTest Result ("<<r_id++<<")  Created a robot "
            <<robot_name<<" on the pile"<<std::flush;
      }

      // Check the robot was parsed.
      scl::SRobotParsed *rob_ds = db->s_parser_.robots_.at(robot_name);
      if(NULL == rob_ds)
      { throw(std::runtime_error("Could not find registered robot in the database"));  }

      // IF the robot wasn't sorted, issue a warning and set present order as sorted
      std::vector<std::string> tmp_sort_order;
      flag = rob_ds->rb_tree_.sort_get_order(tmp_sort_order);
      if(false == flag)
      {
        std::cout<<"\nWARNING : Robot branching representation is not sorted by default. Sorting. Robot = "<<rob_ds->name_;

        // Get the present node ordering.
        sutil::CMappedTree<std::string, SRigidBody>::const_iterator it,ite;
        for(it = rob_ds->rb_tree_.begin(), ite = rob_ds->rb_tree_.end();
                  it!=ite; ++it)
        { tmp_sort_order.push_back(it->name_); }

        // Sort it.
        flag = rob_ds->rb_tree_.sort(tmp_sort_order);
        if(false == flag)
        { throw(std::runtime_error("Could not sort unsorted robot branching representation."));  }
      }

      //*********** Create the dynamics computational object *************
      dynamics = new scl_ext::CDynamicsSclSpatial();
      if (S_NULL==dynamics)
      { throw(std::runtime_error("Failed to allocate memory for scl spatial dynamics."));  }

      // Initialize the dynamics computational object
      flag = dynamics->init(*rob_ds);
      if (false==flag) { throw(std::runtime_error("Failed to initialize scl spatial dynamics."));  }
      else { std::cout<<"\nTest Result ("<<r_id++<<")  Initialized scl spatial dynamics for the robot.";  }

      //*********** Create and intialize the analytic dynamics computational object *************
      scl::CDynamicsAnalyticRPP dyn_anlyt;
      flag = dyn_anlyt.init(*rob_ds);
      if (false==flag) { throw(std::runtime_error("Failed to initialize analytic dynamics."));  }

      SRobotIO * io_ds;
      io_ds = scl::CDatabase::getData()->s_io_.io_data_.at(robot_name);
      if(S_NULL == io_ds)
      { throw(std::runtime_error("Could not find the robot's I/O data structure in the database"));  }

      //******************* Now test the actual implementation ******************
      io_ds->sensors_.q_.setZero(3);
      io_ds->sensors_.dq_.setZero(3);
      io_ds->sensors_.ddq_.setZero(3);

      dynamics->integrate(*io_ds, 0.001); // Sets the dynamic state. Also sets q to some nonzero value.

      // *********************************************************************************************************
      //                              Set up the robot's dynamics data struct
      // *********************************************************************************************************
      // These are used to compute the full dynamics model later in the code..
      scl::SGcModel rob_gc_model;
      flag = rob_gc_model.init(*rob_ds);
      if(false == flag)
      { throw(std::runtime_error("Could not create a dynamic object for the robot"));  }

//      // *********************************************************************************************************
//      //                                   Test Transformation Matrices
//      // *********************************************************************************************************
//      // Set up variables.
//      std::string link_name;
//      Eigen::Affine3d Tscl_spatial, Tanlyt;
//
//      // Loop through all links and get transformations.
//      sutil::CMappedTree<std::string, SRigidBody>::const_iterator it,ite;
//      for(it = rob_ds->rb_tree_.begin(), ite = rob_ds->rb_tree_.end();
//          it!=ite; ++it)
//      {
//        // Test : Link 0
//        link_name = it->name_;
//
//        // Skip the root node (all matrices are zero).
//        if(it->is_root_) { continue; }
//
//        flag = dynamics->computeTransform_Depracated(dynamics->getIdForLink(link_name),Tscl_spatial);
//        if (false==flag) { throw(std::runtime_error("Failed to compute scl_spatial transformation matrix."));  }
//
//        flag = dyn_anlyt.computeTransformationMatrix(io_ds->sensors_.q_, dyn_anlyt.getIdForLink(link_name),
//            dyn_anlyt.getIdForLink("root"), Tanlyt);
//        if (false==flag) { throw(std::runtime_error(std::string("Failed to compute analytic transformation matrix at: ") + link_name));  }
//
//        for(int i=0; i<4 && flag; i++)
//          for(int j=0; j<4 && flag; j++)
//          { flag = flag && (fabs(Tscl_spatial.matrix()(i,j) - Tanlyt.matrix()(i,j))<test_precision);  }
//
//        if (false==flag)
//        {
//          std::cout<<"\nScl spatial transform Org->"<<link_name<<":\n"<<Tscl_spatial.matrix();
//          std::cout<<"\nAnalytic transform Org->"<<link_name<<":\n"<<Tanlyt.matrix();
//          throw(std::runtime_error("Scl spatial and analytic transformation matrices don't match."));
//        }
//        else { std::cout<<"\nTest Result ("<<r_id++<<")  Analytic and scl_spatial transformations match for zero position : "<<link_name;  }
//
//#ifdef DEBUG
//        std::cout<<"\nScl spatial transform Org->"<<link_name<<":\n"<<Tscl_spatial.matrix();
//        std::cout<<"\nAnalytic transform Org->"<<link_name<<":\n"<<Tanlyt.matrix();
//#endif
//      }
//
//      // *********************************************************************************************************
//      //                               Test Transformation Matrix for a range of GCs
//      // *********************************************************************************************************
//      double gcstep=0.1;
//      for (double a=-3.14;a<3.14;a+=gcstep)
//        for (double b=-3.14;b<3.14;b+=gcstep)
//          for (double c=-3.14;c<3.14;c+=gcstep)
//          {
//            io_ds->sensors_.q_(0) = a;
//            io_ds->sensors_.q_(1) = b;
//            io_ds->sensors_.q_(2) = c;
//            flag = dynamics->setGeneralizedCoordinates(io_ds->sensors_.q_);
//            if (false==flag)
//            {
//              std::cout<<"\nGeneralized Coordinates: "<<io_ds->sensors_.q_.transpose();
//              throw(std::runtime_error("Failed to update scl_spatial's internal state.."));
//            }
//
//            // Loop through all links and get transformations.
//            for(it = rob_ds->rb_tree_.begin(), ite = rob_ds->rb_tree_.end();
//                it!=ite; ++it)
//            {
//              // Test : Link 0
//              link_name = it->name_;
//
//              // Skip the root node (all matrices are zero).
//              if(it->is_root_) { continue; }
//
//              flag = dynamics->computeTransform_Depracated(dynamics->getIdForLink(link_name),Tscl_spatial);
//              if (false==flag) { throw(std::runtime_error("Failed to compute scl_spatial transformation matrix."));  }
//
//              flag = dyn_anlyt.computeTransformationMatrix(io_ds->sensors_.q_, dyn_anlyt.getIdForLink(link_name),
//                  dyn_anlyt.getIdForLink("root"), Tanlyt);
//              if (false==flag) { throw(std::runtime_error(std::string("Failed to compute analytic transformation matrix at: ") + link_name));  }
//
//              for(int i=0; i<4 && flag; i++)
//                for(int j=0; j<4 && flag; j++)
//                { flag = flag && (fabs(Tscl_spatial.matrix()(i,j) - Tanlyt.matrix()(i,j))<test_precision); }
//
//              if (false==flag)
//              {
//                std::cout<<"\nGeneralized Coordinates: "<<io_ds->sensors_.q_.transpose();
//                std::cout<<"\nScl spatial transform Org->"<<link_name<<":\n"<<Tscl_spatial.matrix();
//                std::cout<<"\nAnalytic transform Org->"<<link_name<<":\n"<<Tanlyt.matrix();
//                throw(std::runtime_error("Scl spatial and analytic transformation matrices don't match."));
//              }
//            }
//          }
//      std::cout<<"\nTest Result ("<<r_id++<<")  Analytic and scl_spatial transformations match for all links and gcs [-pi,pi]";
//
//      // *********************************************************************************************************
//      //                                         Test Generalized Inertia Matrix
//      // *********************************************************************************************************
//      // Set up variables.
//      Eigen::MatrixXd Mgc_scl_spatial, Mgc_anlyt;
//
//      flag = dynamics->computeGCModel(&(io_ds->sensors_),&rob_gc_model);
//      if (false==flag) { throw(std::runtime_error("Failed to compute scl_spatial model matrices (for generalized inertia)."));  }
//
//      Mgc_scl_spatial = rob_gc_model.M_gc_;
//
//      flag = dyn_anlyt.computeMgc(io_ds->sensors_.q_, Mgc_anlyt);
//      if (false==flag) { throw(std::runtime_error("Failed to compute analytic generalized inertia."));  }
//
//      for(int i=0; i<3; i++)
//        for(int j=0; j<3; j++)
//        { flag = flag && (fabs(Mgc_scl_spatial(i,j) - Mgc_anlyt(i,j))<test_precision); }
//
//      if (false==flag)
//      {
//        std::cout<<"\nScl spatial Mgc:\n"<<Mgc_scl_spatial;
//        std::cout<<"\nAnalytic Mgc:\n"<<Mgc_anlyt;
//        throw(std::runtime_error("Scl spatial and analytic Generalized Inertias don't match."));
//      }
//      else { std::cout<<"\nTest Result ("<<r_id++<<")  Analytic and scl_spatial Generalized Inertias match for zero position";  }
//
//#ifdef DEBUG
//      std::cout<<"\nScl spatial Mgc:\n"<<Mgc_scl_spatial;
//      std::cout<<"\nAnalytic Mgc:\n"<<Mgc_anlyt;
//#endif
//
//      // *********************************************************************************************************
//      //                          Test Generalized Inertia Matrix for a range of GCs
//      // *********************************************************************************************************
//      for (double a=-3.14;a<3.14;a+=gcstep)
//        for (double b=-3.14;b<3.14;b+=gcstep)
//          for (double c=-3.14;c<3.14;c+=gcstep)
//          {
//            io_ds->sensors_.q_(0) = a;
//            io_ds->sensors_.q_(1) = b;
//            io_ds->sensors_.q_(2) = c;
//
//            flag = dynamics->computeGCModel(&(io_ds->sensors_),&rob_gc_model);
//            if (false==flag) { throw(std::runtime_error("Failed to compute scl_spatial model matrices (for generalized inertia)."));  }
//
//            Mgc_scl_spatial = rob_gc_model.M_gc_;
//
//            flag = dyn_anlyt.computeMgc(io_ds->sensors_.q_, Mgc_anlyt);
//            if (false==flag) { throw(std::runtime_error("Failed to compute analytic generalized inertia."));  }
//
//            for(int i=0; i<3; i++)
//              for(int j=0; j<3; j++)
//              { flag = flag && (fabs(Mgc_scl_spatial(i,j) - Mgc_anlyt(i,j))<test_precision); }
//
//            if (false==flag)
//            {
//              std::cout<<"\nGeneralized Coordinates: "<<io_ds->sensors_.q_.transpose();
//              std::cout<<"\nScl spatial Mgc:\n"<<Mgc_scl_spatial;
//              std::cout<<"\nAnalytic Mgc:\n"<<Mgc_anlyt;
//              throw(std::runtime_error("Scl spatial and analytic Generalized Inertias don't match."));
//            }
//          }
//      std::cout<<"\nTest Result ("<<r_id++<<")  Analytic and scl_spatial Generalized Inertias match for all gcs [-pi, pi]";

//      // *********************************************************************************************************
//      //                                         Test Com Jacobians
//      // *********************************************************************************************************
//      // Set up variables.
//      Eigen::MatrixXd Jcom_scl_spatial, Jcom_anlyt;
//      Eigen::VectorXd pos;
//      pos.setZero(3);
//
//      for(it = rob_ds->rb_tree_.begin(), ite = rob_ds->rb_tree_.end();
//          it!=ite; ++it)
//      {
//        // Test : Link 0
//        link_name = it->name_;
//
//        // Skip the root node (all matrices are zero).
//        if(it->is_root_) { continue; }
//
//        pos = it->com_;
//        flag = dynamics->computeJacobian_Depracated(dynamics->getIdForLink(link_name),pos,Jcom_scl_spatial);
//        if (false==flag) { throw(std::runtime_error("Failed to compute scl_spatial com Jacobian."));  }
//
//        flag = dyn_anlyt.computeJcom(io_ds->sensors_.q_, dyn_anlyt.getIdForLink(link_name), Jcom_anlyt);
//        if (false==flag) { throw(std::runtime_error("Failed to compute analytic com Jacobian."));  }
//
//        for(int i=0; i<3; i++)
//          for(int j=0; j<3; j++)
//          { flag = flag && fabs(Jcom_scl_spatial(i,j) - Jcom_anlyt(i,j)) < test_precision; }
//
//        if (false==flag)
//        {
//          std::cout<<"\nScl spatial Jcom_"<<link_name<<":\n"<<Jcom_scl_spatial;
//          std::cout<<"\nAnalytic Jcom_"<<link_name<<":\n"<<Jcom_anlyt;
//          throw(std::runtime_error("Scl spatial and analytic Jacobians don't match."));
//        }
//        else { std::cout<<"\nTest Result ("<<r_id++<<")  Analytic and scl_spatial com Jacobians match for zero position : "<<link_name;  }
//
//#ifdef DEBUG
//        std::cout<<"\nScl spatial Jcom_"<<link_name<<":\n"<<Jcom_scl_spatial;
//        std::cout<<"\nAnalytic Jcom_"<<link_name<<":\n"<<Jcom_anlyt;
//#endif
//      }
//
//      // *********************************************************************************************************
//      //                                Test Com Jacobians Performance
//      // *********************************************************************************************************
//#ifdef DEBUG
//      std::cout<<"\n\n *** Testing Jacobian performance for a variety of GCs *** ";
//#endif
//      gcstep = gcstep / 2.0;
//      //Test Analytic dynamics performance
//      sutil::CMappedTree<std::string, SRigidBodyDyn>::const_iterator itd,itde;
//      t1 = sutil::CSystemClock::getSysTime();
//      for (double a=-3.14;a<3.14;a+=gcstep)
//        for (double b=-3.14;b<3.14;b+=gcstep)
//          for (double c=-3.14;c<3.14;c+=gcstep)
//          {
//            io_ds->sensors_.q_<<a,b,c;
//            for(itd = rob_gc_model.rbdyn_tree_.begin(), itde = rob_gc_model.rbdyn_tree_.end();
//                itd!=itde; ++itd)
//            {
//              if(itd->link_ds_->is_root_) { continue; } // Skip the root node (all matrices are zero).
//              dyn_anlyt.computeJcom(io_ds->sensors_.q_, dyn_anlyt.getIdForLink(itd->name_), Jcom_anlyt);
//            }
//          }
//      t2 = sutil::CSystemClock::getSysTime();
//      std::cout<<"\nTest Result ("<<r_id++<<")  Analytic Jacobian performance: "
//          <<pow(double(int(6.28/double(gcstep))),3)*3<<" Jacobians in "<<t2-t1<<"s. \n\t\tFreq : "
//          <<pow(double(int(6.28/double(gcstep))),3)*3.0/(t2-t1)<<" Hz";
//
//      //Test SCL dynamics performance
//      t1 = sutil::CSystemClock::getSysTime();
//      for (double a=-3.14;a<3.14;a+=gcstep)
//        for (double b=-3.14;b<3.14;b+=gcstep)
//          for (double c=-3.14;c<3.14;c+=gcstep)
//          {
//            io_ds->sensors_.q_<<a,b,c;
//            for(it = rob_ds->rb_tree_.begin(), ite = rob_ds->rb_tree_.end();
//                it!=ite; ++it)
//            {
//              // Skip the root node (all matrices are zero).
//              if(it->is_root_) { continue; }
//              dynamics->computeJacobian_Depracated(dynamics->getIdForLink(link_name),it->com_,Jcom_scl_spatial);
//            }
//          }
//      t2 = sutil::CSystemClock::getSysTime();
//      std::cout<<"\nTest Result ("<<r_id++<<")  Scl spatial Jacobian performance: "
//          <<pow(double(int(6.28/double(gcstep))),3)*3<<" Jacobians in "<<t2-t1<<"s. \n\t\tFreq : "
//          <<pow(double(int(6.28/double(gcstep))),3)*3.0/(t2-t1)<<" Hz";
//
//      // ********************************************************************************************************

      // ********************************************************************************************************
      //Delete stuff
      if(S_NULL!= dynamics)  {  delete dynamics; dynamics = S_NULL; }

      std::cout<<"\nTest #"<<id<<" : Succeeded.";
    }
    catch (std::exception& ee)
    {
      std::cout<<"\nTest Result ("<<r_id++<<") : "<<ee.what();
      std::cout<<"\nTest #"<<id<<" : Failed.";

      if(S_NULL!= dynamics)  {  delete dynamics; dynamics = S_NULL; }
    }
  }
}

