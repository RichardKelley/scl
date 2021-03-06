/* This file is part of scl, a control and simulation library
for robots and biomechanical models.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
/* \file ChaiGlutHandlers.cpp
 *
 *  Created on: Oct 27, 2010
 *
 *  Copyright (C) 2010
 *
 *  Author: Samir Menon <smenon@stanford.edu>
 */

/* Handler functions for chai.
 *
 * NOTE : Right now, scl only supports a single glut display
 *        instance. It renders the first chai graphics instance
 *        by default.
 */

#include <scl/graphics/chai/ChaiGlutHandlers.hpp>

#include <GL/freeglut.h>

using namespace chai3d;

namespace scl_chai_glut_interface
{
  //---------------------------------------------------------------------------
  SChaiGlobals::SChaiGlobals()
  {
    for(int i=0; i<256; ++i)
      keys_active[i] = false;
    GLOB_chaiDbptr = S_NULL;
    GLOB_gr_parsed_ds = S_NULL;
    GLOB_windowPosX = 0;
    GLOB_windowPosY = 0;
    cam_sph_x_=3.0;
    cam_sph_h_= 0.0;
    cam_sph_v_=0.0;
    cam_lookat_x_ = 0.0;
    cam_lookat_y_ = 0.0;
    cam_lookat_z_ = 0.0;
    chai_glut = S_NULL;
    chai_glut_running=true;
  }

  /**
   * Picks the passed chai definition (by string name) in the database
   */
  bool initializeGlutForChai(const std::string & arg_graphics_name,
      scl::CGraphicsChai *arg_chai_glut)
  {
    scl::SDatabase *db = scl::CDatabase::getData();
    if(S_NULL == db) { std::cout<<"\ninitializeGlutForChai() : Database not initialized"; return false; }

    scl::SGraphicsParsed* gr_parsed_ds = db->s_parser_.graphics_worlds_.at(arg_graphics_name);
    return initializeGlutForChai(gr_parsed_ds,arg_chai_glut);
  }

  /** Sets up the glut window GL stuff for a given graphics parsed spec */
  bool initializeGlutForChai(scl::SGraphicsParsed* arg_gr_parsed_ds,
      scl::CGraphicsChai *arg_chai_glut)
  {
    try
    {
      if(NULL == arg_gr_parsed_ds)
      { throw(std::runtime_error("Could not find parsed graphics data structure in the database.")); }

      std::string graphics_name = arg_gr_parsed_ds->name_;

      scl::SDatabase *db = scl::CDatabase::getData();
      if(S_NULL == db) { throw(std::runtime_error("Database not initialized")); }

      //Set all the UI points to zero.
      for(int i=0;i<SCL_NUM_UI_POINTS;++i)
      {
        db->s_gui_.ui_point_[i](0) = 0.0;
        db->s_gui_.ui_point_[i](1) = 0.0;
        db->s_gui_.ui_point_[i](2) = 0.0;
      }

      SChaiGlobals* chai_glob_ds = CChaiGlobals::getData();
      if(S_NULL == chai_glob_ds) { throw(std::runtime_error("Chai shared data singleton not initialized")); }

      if(S_NULL == arg_chai_glut)
      { throw(std::runtime_error("Passed invalid chai object")); }
      chai_glob_ds->chai_glut = arg_chai_glut;

      chai_glob_ds->GLOB_chaiDbptr = arg_chai_glut->getChaiData();
      chai_glob_ds->GLOB_gr_parsed_ds = arg_gr_parsed_ds;
      chai_glob_ds->cam_lookat_x_ = arg_gr_parsed_ds->cam_lookat_(0);
      chai_glob_ds->cam_lookat_y_ = arg_gr_parsed_ds->cam_lookat_(1);
      chai_glob_ds->cam_lookat_z_ = arg_gr_parsed_ds->cam_lookat_(2);

      //set the initial camera position:
      chai_glob_ds->cam_sph_x_=(arg_gr_parsed_ds->cam_pos_ - arg_gr_parsed_ds->cam_lookat_).norm();
      chai_glob_ds->cam_sph_v_= 180/M_PI*(std::asin(arg_gr_parsed_ds->cam_pos_[2]/chai_glob_ds->cam_sph_x_));
      chai_glob_ds->cam_sph_h_= 180/M_PI*(std::asin(arg_gr_parsed_ds->cam_pos_[1]/
          (chai_glob_ds->cam_sph_x_*cCosDeg(chai_glob_ds->cam_sph_v_))));

      if(S_NULL == chai_glob_ds->GLOB_chaiDbptr)
      { throw(std::runtime_error("Couldn't find the specified graphics instance")); }

      // retrieve the resolution of the computer display and estimate the position
      // of the GLUT window so that it is located at the center of the screen
      int screenW = glutGet(GLUT_SCREEN_WIDTH);
      int screenH = glutGet(GLUT_SCREEN_HEIGHT);

      chai_glob_ds->GLOB_windowPosX = (screenW - chai_glob_ds->GLOB_chaiDbptr->gl_width_) / 2;
      chai_glob_ds->GLOB_windowPosY = (screenH - chai_glob_ds->GLOB_chaiDbptr->gl_height_) / 2;

      chai_glob_ds->GLOB_chaiDbptr->running_ = true;

      // initialize the OpenGL GLUT window
      glutInitWindowPosition(chai_glob_ds->GLOB_windowPosX, chai_glob_ds->GLOB_windowPosX);
      glutInitWindowSize(chai_glob_ds->GLOB_chaiDbptr->gl_width_, chai_glob_ds->GLOB_chaiDbptr->gl_height_);
      glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
      glutCreateWindow("scl_jasmine");
      glewInit();

      //Set up glut's handlers
      glutDisplayFunc(scl_chai_glut_interface::updateGraphics);
      glutKeyboardFunc(scl_chai_glut_interface::keyPressed);
      glutKeyboardUpFunc(scl_chai_glut_interface::keyReleased);
      glutReshapeFunc(scl_chai_glut_interface::resizeWindow);
      glutMouseFunc(mouseClick);
      glutMotionFunc(mouseMove);
      glutPassiveMotionFunc(mousePassiveMove);
      std::string tmp_str;
      tmp_str = std::string("SCL : Standard Control Library ") + std::string(scl::VERSION);
      glutSetWindowTitle(tmp_str.c_str());

      // create a mouse menu (right button)
      glutCreateMenu(scl_chai_glut_interface::menuSelect);
      glutAddMenuEntry("full screen",
          scl_chai_glut_interface::SChaiGlobals::OPTION_FULLSCREEN);
      glutAddMenuEntry("window display",
          scl_chai_glut_interface::SChaiGlobals::OPTION_WINDOWDISPLAY);
      glutAddMenuEntry("toggle mouse cam",
          scl_chai_glut_interface::SChaiGlobals::OPTION_TOGGLE_MOUSE_CAM_SELECT);
      glutAddMenuEntry("toggle mouse move scene",
          scl_chai_glut_interface::SChaiGlobals::OPTION_TOGGLE_MOUSE_MOVE_SCENE);
      glutAttachMenu(GLUT_MIDDLE_BUTTON);
    }
    catch(std::exception & e)
    {
      std::cout<<"\ninitializeGlutForChai() Error: "<< e.what();
      return false;
    }
    return true;
  }

  //---------------------------------------------------------------------------

  void resizeWindow(int w, int h)
  {
    SChaiGlobals* chai_glob_ds = CChaiGlobals::getData();
    // update the size of the viewport
    chai_glob_ds->GLOB_chaiDbptr->gl_width_ = w;
    chai_glob_ds->GLOB_chaiDbptr->gl_height_ = h;
    glViewport(0, 0,
        chai_glob_ds->GLOB_chaiDbptr->gl_width_,
        chai_glob_ds->GLOB_chaiDbptr->gl_height_);
  }

  //---------------------------------------------------------------------------
  // callback when a keyboard key is pressed
  void keyPressed(unsigned char key, int x, int y)
  {
    CChaiGlobals::getData()->keys_active[key] = true;
  }

  // callback when a keyboard key is released
  void keyReleased(unsigned char key, int x, int y)
  {

    CChaiGlobals::getData()->keys_active[key] = false;
  }


  void keyHandler()
  {
    SChaiGlobals* chai_glob_ds = CChaiGlobals::getData();
    // escape key
    if ((chai_glob_ds->keys_active[27]) || (chai_glob_ds->keys_active[static_cast<int>('x')]))
    {
      // close everything
      closeChaiGlut();
      // set running flag to false
      chai_glob_ds->chai_glut_running = false;
    }

    // option 1-8: UI flags
    const char numarr[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8'};
    for(int i=0;i<9;++i)
    {
      if (chai_glob_ds->keys_active[static_cast<int>( numarr[i] )])
      {//Toggle the UI flag
        std::cout<<"\n Flipping ui_flag_ : "<<i<<" to "<< !scl::CDatabase::getData()->s_gui_.ui_flag_[i]<<std::flush;
        scl::CDatabase::getData()->s_gui_.ui_flag_[i] = !scl::CDatabase::getData()->s_gui_.ui_flag_[i] ;
        chai_glob_ds->keys_active[static_cast<int>(numarr[i])] = false;
      }
    }

    // option 10-19: UI flags
    const char numarr2[] = {')', '!', '@', '#', '$', '%', '^', '&', '*', '('};
    for(int i=0;i<=9;++i)
    {
      if (chai_glob_ds->keys_active[static_cast<int>( numarr2[i] )])
      {//Toggle the UI flag
        std::cout<<"\n Flipping ui_flag_ : "<<10+i<<" to "<< !scl::CDatabase::getData()->s_gui_.ui_flag_[10+i]<<std::flush;
        scl::CDatabase::getData()->s_gui_.ui_flag_[10+i] = !scl::CDatabase::getData()->s_gui_.ui_flag_[10+i] ;
        chai_glob_ds->keys_active[static_cast<int>(numarr2[i])] = false;
      }
    }

    // shift+9  is a special flag for moving all ui points independently
    if (chai_glob_ds->keys_active[static_cast<int>('9')])
    {
      if(scl::CDatabase::getData()->s_gui_.ui_point_selector_ == 0)
      {
        std::cout<<"\nGLUT: Moving all operational points simultaneously with 's', 'w', 'd', 'a', 'e', 'q'";
        scl::CDatabase::getData()->s_gui_.ui_point_selector_ = 1;
      }
      else
      {
        std::cout<<"\nGLUT: Moving all operational points independently";
        scl::CDatabase::getData()->s_gui_.ui_point_selector_ = 0;
      }
      chai_glob_ds->keys_active[static_cast<int>('9')] = false;
    }

    /** Control the gui 3D points. Can control 6 points total using the glut buttons */
    const char arr[] = {'s', 'w', 'd', 'a', 'e', 'q', 'k', 'i', 'l', 'j', 'o', 'u', 'g', 't', 'h', 'f', 'y', 'r',
        'S', 'W', 'D', 'A', 'E', 'Q', 'K', 'I', 'L', 'J', 'O', 'U', 'G', 'T', 'H', 'F', 'Y', 'R'};
    //Can set this multiplier to modulate the motion of the operational points
    const scl::sFloat opt_mult=1;
    if(scl::CDatabase::getData()->s_gui_.ui_point_selector_ == 0)
    {//Control each point independently
      for(unsigned int i=0; i<6; ++i)
      {//Loop over 6 operational points
        for(unsigned int j=0; j<3; ++j)
        {//Loop over 3 orthogonal directionss
          if (chai_glob_ds->keys_active[static_cast<int>( arr[6*i+2*j] )])
          {//Increment op point
            scl::CDatabase::getData()->s_gui_.ui_point_[i](j) += opt_mult*0.01;
            //chai_glob_ds->keys_active[static_cast<int>( arr[6*i+2*j] )] = false;
          }
          if (chai_glob_ds->keys_active[static_cast<int>( arr[6*i+2*j+1] )])
          {//Decrement op point
            scl::CDatabase::getData()->s_gui_.ui_point_[i](j) -= opt_mult*0.01;
            //chai_glob_ds->keys_active[static_cast<int>( arr[6*i+2*j+1] )] = false;
          }
        }
      }
    }
    else
    {//Control all points simultaneously
      for(unsigned int j=0; j<3; ++j)
      {//Loop over 3 orthogonal directions
        if (chai_glob_ds->keys_active[static_cast<int>( arr[2*j] )])
        {//Increment op point
          for(unsigned int i=0; i<12; ++i)
          {//Loop over all operational points
            scl::CDatabase::getData()->s_gui_.ui_point_[i](j) += opt_mult*0.01;
          }
        }
        //chai_glob_ds->keys_active[static_cast<int>( arr[2*j] )] = false;
        if (chai_glob_ds->keys_active[static_cast<int>( arr[2*j+1] )])
        {//Decrement op point
          for(unsigned int i=0; i<12; ++i)
          {//Loop over all operational points
            scl::CDatabase::getData()->s_gui_.ui_point_[i](j) -= opt_mult*0.01;
          }
          //chai_glob_ds->keys_active[static_cast<int>( arr[2*j+1] )] = false;
        }
      }
    }

    /** Control the ui_ints. / keeps incrementing and cycles around.. */
    const char ui_int_arr[] = {',',  '.', '>'};
    if (chai_glob_ds->keys_active[static_cast<int>( ui_int_arr[0] )])
    {//Decrement the UI int
      scl::CDatabase::getData()->s_gui_.ui_int_[scl::CDatabase::getData()->s_gui_.ui_int_selector_]--;
      chai_glob_ds->keys_active[static_cast<int>(numarr[0])] = false;
      const timespec ts = {0, 90000000};/*50ms : Need time to pull finger off kbd*/ nanosleep(&ts,NULL);
    }
    if (chai_glob_ds->keys_active[static_cast<int>( ui_int_arr[1] )])
    {//Decrement the UI int
      scl::CDatabase::getData()->s_gui_.ui_int_[scl::CDatabase::getData()->s_gui_.ui_int_selector_]++;
      chai_glob_ds->keys_active[static_cast<int>(numarr[1])] = false;
      const timespec ts = {0, 90000000};/*50ms : Need time to pull finger off kbd*/ nanosleep(&ts,NULL);
    }
    if (chai_glob_ds->keys_active[static_cast<int>( ui_int_arr[2] )])
    {//Increment and toggle the UI int
      scl::CDatabase::getData()->s_gui_.ui_int_selector_ = (scl::CDatabase::getData()->s_gui_.ui_int_selector_+1)%SCL_NUM_UI_INTS;
      chai_glob_ds->keys_active[static_cast<int>(numarr[2])] = false;
      std::cout<<"\n UI INT SELECTED : "<<scl::CDatabase::getData()->s_gui_.ui_int_selector_
          <<" : "<<scl::CDatabase::getData()->s_gui_.ui_int_[scl::CDatabase::getData()->s_gui_.ui_int_selector_]
          <<std::flush;
      const timespec ts = {0, 60000000};/*50ms : Need time to pull finger off kbd*/ nanosleep(&ts,NULL);
    }

    //Pause the controller
    if (chai_glob_ds->keys_active[static_cast<int>('p')])
    {
      scl::CDatabase::getData()->pause_ctrl_dyn_ = true;
      std::cout<<"\nGLUT: Controller and Simulation Paused. Press 'P' to unpause or ';' to step. "<<std::flush;
      chai_glob_ds->keys_active[static_cast<int>('p')] = false;
    }

    //Unpause the controller
    if (chai_glob_ds->keys_active[static_cast<int>('P')])
    {
      scl::CDatabase::getData()->pause_ctrl_dyn_ = false;
      std::cout<<"\nGLUT: Controller and Simulation Unpaused.";
      chai_glob_ds->keys_active[static_cast<int>('P')] = false;
    }

    //Step the controller
    if (chai_glob_ds->keys_active[static_cast<int>(';')])
    {
      scl::CDatabase::getData()->step_ctrl_dyn_ = true;
      std::cout<<"+1"<<std::flush;
      chai_glob_ds->keys_active[static_cast<int>(';')] = false;
    }

    //Toggle logging
    if (chai_glob_ds->keys_active[static_cast<int>('/')])
    {
      scl::CDatabase::getData()->param_logging_on_ = ~scl::CDatabase::getData()->param_logging_on_;
      if(scl::CDatabase::getData()->param_logging_on_)
      { std::cout<<"\nGLUT: Starting logging"<<std::flush;  }
      else
      { std::cout<<"\nGLUT: Stopping logging"<<std::flush;  }
      chai_glob_ds->keys_active[static_cast<int>('/')] = false;
    }

    //Modulate a robot's links
    static int link_idx = 0;
    if(scl::CDatabase::getData()->s_io_.io_data_.at(0) != NULL)
    {//Can't modulate links if there is no robot.
      if (chai_glob_ds->keys_active[static_cast<int>('+')])
      {
        link_idx++;

        scl::SDatabase* db = scl::CDatabase::getData();
        scl::SRobotIO* io = db->s_io_.io_data_.at(0);
        if(link_idx >= io->sensors_.q_.size())
        { link_idx = io->sensors_.q_.size()-1;  }

        std::cout<<"\nGLUT: "<<io->name_<<" : Link : "<<link_idx<<std::flush;
        chai_glob_ds->keys_active[static_cast<int>('+')] = false;
      }
      if (chai_glob_ds->keys_active[static_cast<int>('-')])
      {
        link_idx--;
        if(0>link_idx) link_idx = 0;

        scl::SDatabase* db = scl::CDatabase::getData();
        scl::SRobotIO* io = db->s_io_.io_data_.at(0);

        std::cout<<"\nGLUT: "<<io->name_<<" : Link : "<<link_idx<<std::flush;
        chai_glob_ds->keys_active[static_cast<int>('-')] = false;
      }

      // Position keyboard shortcuts.
      if (chai_glob_ds->keys_active[static_cast<int>(']')])
      {
        scl::SDatabase* db = scl::CDatabase::getData();
        scl::SRobotIO* io = db->s_io_.io_data_.at(0);
        if(link_idx >= io->sensors_.q_.size())
        { link_idx = io->sensors_.q_.size()-1;  }
        io->sensors_.q_(link_idx) += 0.1;
      }
      if (chai_glob_ds->keys_active[static_cast<int>('}')])
      {
        scl::SDatabase* db = scl::CDatabase::getData();
        scl::SRobotIO* io = db->s_io_.io_data_.at(0);
        if(link_idx >= io->sensors_.q_.size())
        { link_idx = io->sensors_.q_.size()-1;  }
        io->sensors_.q_(link_idx) += 0.3;
      }

      if (chai_glob_ds->keys_active[static_cast<int>('[')])
      {
        scl::SDatabase* db = scl::CDatabase::getData();
        scl::SRobotIO* io = db->s_io_.io_data_.at(0);
        if(link_idx >= io->sensors_.q_.size())
        { link_idx = io->sensors_.q_.size()-1;  }
        io->sensors_.q_(link_idx) -= 0.1;
      }
      if (chai_glob_ds->keys_active[static_cast<int>('{')])
      {
        scl::SDatabase* db = scl::CDatabase::getData();
        scl::SRobotIO* io = db->s_io_.io_data_.at(0);
        if(link_idx >= io->sensors_.q_.size())
        { link_idx = io->sensors_.q_.size()-1;  }
        io->sensors_.q_(link_idx) -= 0.3;
      }
    }
  }

  //---------------------------------------------------------------------------

  void menuSelect(int value)
  {
    SChaiGlobals* chai_glob_ds = CChaiGlobals::getData();
    switch (value)
    {
      // enable full screen display
      case SChaiGlobals::OPTION_FULLSCREEN:
        glutFullScreen();
        break;

        // reshape window to original size
      case SChaiGlobals::OPTION_WINDOWDISPLAY:
        glutReshapeWindow(chai_glob_ds->GLOB_chaiDbptr->gl_width_
            , chai_glob_ds->GLOB_chaiDbptr->gl_height_);
        break;

        //Whether click-drag rotates the camera or adds a force.
      case SChaiGlobals::OPTION_TOGGLE_MOUSE_CAM_SELECT:
        chai_glob_ds->GLOB_chaiDbptr->mouse_mode_cam_ = !(chai_glob_ds->GLOB_chaiDbptr->mouse_mode_cam_);
        break;

        //Whether click-drag rotates the camera or adds a force.
      case SChaiGlobals::OPTION_TOGGLE_MOUSE_MOVE_SCENE:
        chai_glob_ds->GLOB_chaiDbptr->mouse_mode_move_scene_ = !(chai_glob_ds->GLOB_chaiDbptr->mouse_mode_move_scene_);
        break;
    }
  }

  //---------------------------------------------------------------------------
  // stop the simulation
  void closeChaiGlut(void)
  { CChaiGlobals::getData()->GLOB_chaiDbptr->running_ = false;  }

  //---------------------------------------------------------------------------
  void updateGraphics(void)
  {
    SChaiGlobals* chai_glob_ds = CChaiGlobals::getData();
    //Update the IO
    keyHandler();

#ifdef DEBUG
    bool flag;
    flag = chai_glob_ds->chai_glut->updateGraphics();
    if (false == flag) { printf("\nChai-Glut Error: Could not update graphics successfully.");  }
#else
    //NOTE : No checks in release mode
    chai_glob_ds->chai_glut->updateGraphics();
#endif

    /**
     * NOTE TODO : Implement showing a clock on the display
     *
    std::stringstream ss;
    ss << scl::CSystemClock::get_clock()->get_sim_time();
    std::string tmp;
    ss >> tmp;
    glutBitmapString(GLUT_BITMAP_HELVETICA_10,(const unsigned char*) tmp.c_str());
     */

    // Swap buffers
    glutSwapBuffers();

    // check for any OpenGL errors
#ifdef DEBUG
    GLenum err;
    err = glGetError();
    if (err != GL_NO_ERROR)
    { printf("Error:  %s\n", gluErrorString(err));  }
#endif

    // inform the GLUT window to call updateGraphics again (next frame)
    if (chai_glob_ds->GLOB_chaiDbptr->running_)
    { glutPostRedisplay();  }
  }

  //---------------------------------------------------------------------------

  //---------------------------------------------------------------------------

  void mouseClick(int button, int state, int x, int y)
  {
    SChaiGlobals* chai_glob_ds = CChaiGlobals::getData();
    // mouse button down
    if (state == GLUT_DOWN)
    {
      chai_glob_ds->GLOB_chaiDbptr->mouse_button_pressed_ = true;
      chai_glob_ds->GLOB_chaiDbptr->mouse_x_ = x;
      chai_glob_ds->GLOB_chaiDbptr->mouse_y_ = y;
      chai_glob_ds->GLOB_chaiDbptr->mouse_button_ = button;
    }

    // mouse button up
    else if (state == GLUT_UP)
    {
      chai_glob_ds->GLOB_chaiDbptr->mouse_button_pressed_ = false;
    }
  }

  //---------------------------------------------------------------------------

  void mouseMove(int x, int y)
  {
    SChaiGlobals* chai_glob_ds = CChaiGlobals::getData();
    if(true==chai_glob_ds->GLOB_chaiDbptr->mouse_mode_cam_)
    {
      if(chai_glob_ds->GLOB_chaiDbptr->mouse_button_pressed_)
      {
        int mod = glutGetModifiers();
        if (chai_glob_ds->GLOB_chaiDbptr->mouse_mode_move_scene_ && (chai_glob_ds->GLOB_chaiDbptr->mouse_button_ == GLUT_LEFT_BUTTON))
        {//Adjust camera lookat euclidean position x and y
          chai_glob_ds->cam_lookat_y_ = chai_glob_ds->cam_lookat_y_ - 0.01 * (x - chai_glob_ds->GLOB_chaiDbptr->mouse_x_);
          chai_glob_ds->cam_lookat_z_ = chai_glob_ds->cam_lookat_z_ + 0.01 * (y - chai_glob_ds->GLOB_chaiDbptr->mouse_y_);
        }
        else if (chai_glob_ds->GLOB_chaiDbptr->mouse_mode_move_scene_ && (chai_glob_ds->GLOB_chaiDbptr->mouse_button_ == GLUT_RIGHT_BUTTON))
        {//Adjust camera lookat euclidean position z
          chai_glob_ds->cam_lookat_x_ = chai_glob_ds->cam_lookat_x_ + 0.01 * (x - chai_glob_ds->GLOB_chaiDbptr->mouse_x_);
        }
        else if (chai_glob_ds->GLOB_chaiDbptr->mouse_button_ == GLUT_RIGHT_BUTTON)
        {//Adjust camera cylindrical position radius
          chai_glob_ds->cam_sph_x_ = chai_glob_ds->cam_sph_x_ - 0.01 * (y - chai_glob_ds->GLOB_chaiDbptr->mouse_y_);
        }
        else if (chai_glob_ds->GLOB_chaiDbptr->mouse_button_ == GLUT_LEFT_BUTTON)
        {//Adjust camera cylindrical position horizontal and vertical angles
          chai_glob_ds->cam_sph_h_ = chai_glob_ds->cam_sph_h_ - (x - chai_glob_ds->GLOB_chaiDbptr->mouse_x_);
          chai_glob_ds->cam_sph_v_ = chai_glob_ds->cam_sph_v_ + (y - chai_glob_ds->GLOB_chaiDbptr->mouse_y_);
        }
        updateCameraPosition();
      }
    }
    else
    {//Attach a force

    }

    chai_glob_ds->GLOB_chaiDbptr->mouse_x_ = x;
    chai_glob_ds->GLOB_chaiDbptr->mouse_y_ = y;
  }

  void mousePassiveMove(int x, int y)
  {
    SChaiGlobals* chai_glob_ds = CChaiGlobals::getData();
    chai_glob_ds->GLOB_chaiDbptr->mouse_x_ = x;
    chai_glob_ds->GLOB_chaiDbptr->mouse_y_ = y;
  }

  //---------------------------------------------------------------------------

  void updateCameraPosition()
  {
    SChaiGlobals* chai_glob_ds = CChaiGlobals::getData();
    // check values
    static scl::SGraphicsParsed* gr_ds = chai_glob_ds->GLOB_gr_parsed_ds;

    const int sph_max = 120, sph_xmax = 0.001;
    if (chai_glob_ds->cam_sph_x_ < sph_xmax) { chai_glob_ds->cam_sph_x_ = sph_xmax; }
    if (chai_glob_ds->cam_sph_v_ > sph_max) { chai_glob_ds->cam_sph_v_ = sph_max; }
    if (chai_glob_ds->cam_sph_v_ < -sph_max) { chai_glob_ds->cam_sph_v_ = -sph_max; }

    //Update the position the camera looks at
    gr_ds->cam_lookat_(0) = chai_glob_ds->cam_lookat_x_;
    gr_ds->cam_lookat_(1) = chai_glob_ds->cam_lookat_y_;
    gr_ds->cam_lookat_(2) = chai_glob_ds->cam_lookat_z_;

    // compute position of camera in space
    cVector3d pos = cAdd(
        cVector3d(gr_ds->cam_lookat_(0),gr_ds->cam_lookat_(1),gr_ds->cam_lookat_(2)),
        cVector3d(
            chai_glob_ds->cam_sph_x_ * cCosDeg(chai_glob_ds->cam_sph_h_) * cCosDeg(chai_glob_ds->cam_sph_v_),
            chai_glob_ds->cam_sph_x_ * cSinDeg(chai_glob_ds->cam_sph_h_) * cCosDeg(chai_glob_ds->cam_sph_v_),
            chai_glob_ds->cam_sph_x_ * cSinDeg(chai_glob_ds->cam_sph_v_)
        )
    );

    // set new position to camera
    chai_glob_ds->GLOB_chaiDbptr->chai_cam_->set(pos,
        cVector3d(gr_ds->cam_lookat_(0), gr_ds->cam_lookat_(1), gr_ds->cam_lookat_(2)),
        cVector3d(gr_ds->cam_up_(0),gr_ds->cam_up_(1),gr_ds->cam_up_(2)) );

    // recompute global positions
    chai_glob_ds->GLOB_chaiDbptr->chai_world_->computeGlobalPositions(true);
  }

}
