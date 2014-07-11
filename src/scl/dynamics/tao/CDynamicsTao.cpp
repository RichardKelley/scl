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
/* \file CDynamicsTao.cpp
 *
 *  Created on: Aug 23, 2010
 *
 *  Copyright (C) 2010
 *
 *  Author: Samir Menon <smenon@stanford.edu>
 */

#include "CDynamicsTao.hpp"

#include <scl/data_structs/SRobotIO.hpp>

#include <scl/dynamics/tao/CTaoRepCreator.hpp>
#include <scl/dynamics/tao/jspace/tao_util.hpp>
#include <scl/dynamics/tao/jspace/Model.hpp>

#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>

#ifdef W_QNX
extern "C" {
#include <string.h> // for qnx
}
#endif

namespace scl
{
  CDynamicsTao::CDynamicsTao()
  : robot_name_("(no robot yet)"),
    tao_tree_q_root_(NULL), tao_tree_q_dq_root_(NULL),
    model_(S_NULL),
    ndof_(0)
  {
  }

  CDynamicsTao::~CDynamicsTao()
  {
    if(S_NULL!=model_)
    { delete model_;  }
  }

  static void tao_collect_ids(taoDNode * node, std::map<int, int> & id_counter)
  {
    int const id(node->getID());
    std::map<int, int>::iterator idc(id_counter.find(id));
    if (id_counter.end() == idc) {
      id_counter.insert(std::make_pair(id, 1));
    }
    else {
      ++idc->second;
    }
    for (taoDNode * child(node->getDChild()); 0 != child; child = child->getDSibling()) {
      tao_collect_ids(child, id_counter);
    }
  }


  static bool tao_consistency_check(taoNodeRoot * root)
  {
    if (root->getID() != -1) {
      std::cout << "scl::tao_consistency_check(): root has ID " << root->getID() << " instead of -1\n";
      return false;
    }
    std::map<int, int> id_counter;
    for (taoDNode * node(root->getDChild()); 0 != node; node = node->getDSibling()) {
      tao_collect_ids(node, id_counter);
    }
    int expected_id(0);
    //NOTE TODO: XXXX  : Is this really necessary?
    for (std::map<int, int>::const_iterator idc(id_counter.begin()); idc != id_counter.end(); ++idc, ++expected_id) {
      if (idc->first != expected_id) {
        std::cout << "scl::tao_consistency_check(): ID gap, expected "
            << expected_id << " but encountered " << idc->first << "\n";
        return false;
      }
      if (1 != idc->second) {
        std::cout << "scl::tao_consistency_check(): duplicate ID " << idc->first << "\n";
        return false;
      }
    }
    return true;
  }


  bool CDynamicsTao::
  init(const SRobotParsed& arg_robot_data)
  {
    try
    {
      has_been_init_ = false;
      if (model_) {
        fprintf(stderr, "scl::CDynamicsTao::init(): already initialized\n");
        return false;
      }

      if(false == arg_robot_data.has_been_init_){
        fprintf(stderr, "scl::CDynamicsTao::init(): Passed an uninitialized robot\n");
        return false;
      }

      //Set all the robot parameters
      robot_name_ = arg_robot_data.name_;
      gravity_ = arg_robot_data.gravity_;

      //Initialize the tao tree
      tao_tree_q_root_ = scl::CTaoRepCreator::taoRootRepCreator(arg_robot_data);
      if ( ! tao_tree_q_root_) {
        std::cout << "scl::CDynamicsTao::init(`" << robot_name_
            << "'): scl::CTaoRepCreator::taoRootRepCreator() failed [invalid robot name?]\n";
        return false;
      }

      if ( ! tao_consistency_check(tao_tree_q_root_)) {
        std::cout << "scl::CDynamicsTao::init(`" << robot_name_
            << "'): consistency check failed on TAO tree\n";
        return false;
      }

      // Parse it again to get a second copy. Extra cost only incurred
      // at init time, so that should be acceptable, although it would
      // be cleaner to have TAO tree deep copy functionality somewhere.
      tao_tree_q_dq_root_ = scl::CTaoRepCreator::taoRootRepCreator(arg_robot_data);
      if ( ! tao_tree_q_dq_root_) {
        std::cout << "scl::CDynamicsTao::init(`" << robot_name_
            << "'): scl::CTaoRepCreator::taoRootRepCreator(cc) failed [invalid robot name?]\n";
        return false;
      }

      jspace::STaoTreeInfo * js_q_tree(jspace::create_bare_tao_tree_info(tao_tree_q_root_));
      jspace::STaoTreeInfo * js_q_dq_tree(jspace::create_bare_tao_tree_info(tao_tree_q_dq_root_));

      //NOTE TODO Perhaps this was what TRY_TO_CONVERT_NAMES achieved
      sutil::CMappedTree<std::basic_string<char>, scl::SRigidBody>::const_iterator itbr, itbre;
      for(itbr = arg_robot_data.rb_tree_.begin(),
          itbre = arg_robot_data.rb_tree_.end();
          itbr!=itbre; ++itbr)
      {
        std::vector<jspace::STaoNodeInfo>::iterator it, ite, icc;

        //Name the stuff in the kgm tree
        it = js_q_tree->info.begin();
        icc = js_q_dq_tree->info.begin();
        ite = js_q_tree->info.end();
        for (/**/; it != ite; ++it, ++icc)
        {
          const SRigidBody& l_ds = *itbr;
          if (l_ds.name_ == it->node->name_)
          {
            it->link_name   = itbr->name_;
            it->joint_name  = itbr->joint_name_;
            it->limit_lower = itbr->joint_limit_lower_;
            it->limit_upper = itbr->joint_limit_upper_;
            icc->link_name  = itbr->name_;
            icc->joint_name = itbr->joint_name_;
            icc->limit_lower = itbr->joint_limit_lower_;
            icc->limit_upper = itbr->joint_limit_upper_;
            break;
          }
        }
      }

      model_ = new jspace::Model();
      model_->init(js_q_tree, js_q_dq_tree, gravity_(0), gravity_(1), gravity_(2), 0);
      ndof_ = model_->getNDOF();
      state_.init(ndof_, ndof_, 0);

      has_been_init_ = true;
    }
    catch(std::exception& e)
    { std::cerr<<"\nCDynamicsTao::init() : Error : "<<e.what();  }

    return has_been_init_;
  }

  taoDNode* CDynamicsTao::getTaoIdForLink(std::string arg_link_name)
  {
    taoDNode * tmp = model_->getNodeByName(arg_link_name);
    return tmp;
  }

  sBool CDynamicsTao::integrate(SRobotIO& arg_inputs, const sFloat arg_time_interval)
  {
    //NOTE TODO : Implement this for applying external contact forces.

    //1. Compute how the applied forces influence the joint torques.
    // NOTE : This assumes that the supplied Jacobian is valid.
    /*
     * NOTE : The following is experimental. Uncomment to test
    arg_inputs.actuators_.forces_.resetIterator();
    while(S_NULL!= arg_inputs.actuators_.forces_.iterator_)
    {
      scl::SForce* tmp_f = arg_inputs.actuators_.forces_.iterator_->data_;

      arg_inputs.actuators_.force_gc_commanded_ += tmp_f->J_.transpose() * tmp_f->force_;
    }
     */

    //We will integrate the cc tree (arbit. We could integrate the kgm tree as well).
    jspace::STaoTreeInfo * tao_tree = model_->_getCCTree();

    //Uses the cc tree to integrate the dynamics.
    for (size_t ii(0); ii < ndof_; ++ii)
    {
      sInt io_ds_idx;
      io_ds_idx = tao_tree->info[ii].node->getID();

      sFloat q, dq, tau;
      q = arg_inputs.sensors_.q_(io_ds_idx);
      dq = arg_inputs.sensors_.dq_(io_ds_idx);
      tau = arg_inputs.actuators_.force_gc_commanded_(io_ds_idx);

      taoJoint * joint(tao_tree->info[ii].node->getJointList());
      joint->setQ(&q);
      joint->setDQ(&dq);
      joint->zeroDDQ();
      joint->setTau(&tau);//Set the joint torques to be applied.
    }

    //NOTE TODO: Inefficiency due to tao.
    deVector3 gravity;
    gravity[0] = gravity_(0);
    gravity[1] = gravity_(1);
    gravity[2] = gravity_(2);// m/s^2

    sFloat tstep = arg_time_interval;

    taoDynamics::fwdDynamics(tao_tree->root, &gravity);
    taoDynamics::integrate(tao_tree->root, tstep);
    taoDynamics::updateTransformation(tao_tree->root);

    //Uses the cc tree to integrate the dynamics.
    for (size_t ii(0); ii < ndof_; ++ii)
    {
      sFloat q, dq, ddq, tau;
      taoJoint * joint(tao_tree->info[ii].node->getJointList());
      joint->getQ(&q);
      joint->getDQ(&dq);
      joint->getDDQ(&ddq);
      joint->getTau(&tau);

      sInt io_ds_idx;
      io_ds_idx = tao_tree->info[ii].node->getID();
      arg_inputs.sensors_.q_(io_ds_idx) = q;
      arg_inputs.sensors_.dq_(io_ds_idx) = dq;
      arg_inputs.sensors_.ddq_(io_ds_idx) = ddq;
      arg_inputs.sensors_.force_gc_measured_(io_ds_idx) = tau;
    }

    return true;
  }

  /** TODO : This dynamics engine implementation should be stateless.
   * This present system is because the intermediate jspace implementation
   * has an extra data layer. Should be cleaned up in the future. */
  sBool CDynamicsTao::setGeneralizedCoordinates(Eigen::VectorXd &arg_q)
  {
    if(false == has_been_init_){return false; }

    //We will integrate the kgm tree (arbit. We could integrate the kgm tree as well).
    jspace::STaoTreeInfo * tao_tree = model_->_getKGMTree();

    //Uses the cc tree to integrate the dynamics.
    for (size_t ii(0); ii < ndof_; ++ii)
    {
      sInt io_ds_idx;
      io_ds_idx = tao_tree->info[ii].node->getID();

      sFloat q;
      q = arg_q(io_ds_idx);

      taoJoint * joint(tao_tree->info[ii].node->getJointList());
      joint->setQ(&q);
    }

    //We will integrate the cc tree (arbit. We could integrate the kgm tree as well).
    jspace::STaoTreeInfo * tao_tree_cc = model_->_getCCTree();

    //Uses the cc tree to integrate the dynamics.
    for (size_t ii(0); ii < ndof_; ++ii)
    {
      sInt io_ds_idx;
      io_ds_idx = tao_tree_cc->info[ii].node->getID();

      sFloat q;
      q = arg_q(io_ds_idx);

      taoJoint * joint(tao_tree_cc->info[ii].node->getJointList());
      joint->setQ(&q);
    }

    return true;
  }

  /** TODO : This dynamics engine implementation should be stateless.
   * This present system is because the intermediate jspace implementation
   * has an extra data layer. Should be cleaned up in the future. */
  sBool CDynamicsTao::setGeneralizedVelocities(Eigen::VectorXd &arg_dq)
  {
    if(false == has_been_init_){return false; }

    //We will integrate the cc tree (arbit. We could integrate the kgm tree as well).
    jspace::STaoTreeInfo * tao_tree = model_->_getKGMTree();

    //Uses the cc tree to integrate the dynamics.
    for (size_t ii(0); ii < ndof_; ++ii)
    {
      sInt io_ds_idx;
      io_ds_idx = tao_tree->info[ii].node->getID();

      sFloat dq;
      dq = arg_dq(io_ds_idx);

      taoJoint * joint(tao_tree->info[ii].node->getJointList());
      joint->setDQ(&dq);
    }

    //We will integrate the cc tree (arbit. We could integrate the kgm tree as well).
    jspace::STaoTreeInfo * tao_tree_cc = model_->_getCCTree();

    //Uses the cc tree to integrate the dynamics.
    for (size_t ii(0); ii < ndof_; ++ii)
    {
      sInt io_ds_idx;
      io_ds_idx = tao_tree_cc->info[ii].node->getID();

      sFloat dq;
      dq = arg_dq(io_ds_idx);

      taoJoint * joint(tao_tree_cc->info[ii].node->getJointList());
      joint->setDQ(&dq);
    }
    return true;
  }

  /** Sets the external generalized forces
   *
   * TODO : This dynamics engine implementation should be stateless.
   * This present system is because the intermediate jspace implementation
   * has an extra data layer. Should be cleaned up in the future. */
  sBool CDynamicsTao::setGeneralizedForces(Eigen::VectorXd &arg_fgc)
  {
    if(false == has_been_init_){return false; }

    //We will integrate the cc tree (arbit. We could integrate the kgm tree as well).
    jspace::STaoTreeInfo * tao_tree = model_->_getKGMTree();

    //Uses the cc tree to integrate the dynamics.
    for (size_t ii(0); ii < ndof_; ++ii)
    {
      sInt io_ds_idx;
      io_ds_idx = tao_tree->info[ii].node->getID();

      sFloat tau;
      tau = arg_fgc(io_ds_idx);

      taoJoint * joint(tao_tree->info[ii].node->getJointList());
      joint->setTau(&tau);//Set the joint torques to be applied.
    }

    //We will integrate the cc tree (arbit. We could integrate the kgm tree as well).
    jspace::STaoTreeInfo * tao_tree_cc = model_->_getCCTree();

    //Uses the cc tree to integrate the dynamics.
    for (size_t ii(0); ii < ndof_; ++ii)
    {
      sInt io_ds_idx;
      io_ds_idx = tao_tree_cc->info[ii].node->getID();

      sFloat tau;
      tau = arg_fgc(io_ds_idx);

      taoJoint * joint(tao_tree_cc->info[ii].node->getJointList());
      joint->setTau(&tau);//Set the joint torques to be applied.
    }
    return true;
  }

}