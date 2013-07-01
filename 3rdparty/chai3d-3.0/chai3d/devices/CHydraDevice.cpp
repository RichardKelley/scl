//==============================================================================
/*
    Software License Agreement (BSD License)
    Copyright (c) 2003-2012, CHAI3D.
    (www.chai3d.org)

    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials provided
    with the distribution.

    * Neither the name of CHAI3D nor the names of its contributors may
    be used to endorse or promote products derived from this software
    without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
    ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE. 

    \author    <http://www.chai3d.org>
    \author    Francois Conti
    \author    Force Dimension - www.forcedimension.com
    \version   $MAJOR.$MINOR.$RELEASE $Rev: 931 $
*/
//==============================================================================

//------------------------------------------------------------------------------
#include "system/CGlobals.h"
#include "devices/CHydraDevice.h"
//------------------------------------------------------------------------------
#if defined(C_ENABLE_HYDRA_DEVICE_SUPPORT)
//------------------------------------------------------------------------------
#ifndef DOXYGEN_SHOULD_SKIP_THIS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
namespace chai3d {
//------------------------------------------------------------------------------

// DHD function availability
bool cDeltaDevice::sdhdOpenID                               = true;
bool cDeltaDevice::sdhdClose                                = true;
bool cDeltaDevice::sdhdGetDeviceCount                       = true;
bool cDeltaDevice::sdhdGetDeviceID                          = true;
bool cDeltaDevice::sdhdGetSystemType                        = true;
bool cDeltaDevice::sdhdReset                                = true;
bool cDeltaDevice::sdhdGetButton                            = true;
bool cDeltaDevice::sdhdGetPosition                          = true;
bool cDeltaDevice::sdhdGetLinearVelocity                    = true;
bool cDeltaDevice::sdhdGetOrientationRad                    = true;
bool cDeltaDevice::sdhdGetOrientationFrame                  = true;
bool cDeltaDevice::sdhdSetForce                             = true;
bool cDeltaDevice::sdhdSetTorque                            = true;
bool cDeltaDevice::sdhdSetForceAndTorque                    = true;
bool cDeltaDevice::sdhdSetForceAndGripperForce              = true;
bool cDeltaDevice::sdhdSetForceAndTorqueAndGripperForce     = true;
bool cDeltaDevice::sdhdGetGripperThumbPos                   = true;
bool cDeltaDevice::sdhdGetGripperFingerPos                  = true;
bool cDeltaDevice::sdhdGetGripperAngleRad                   = true;
bool cDeltaDevice::sdhdEnableExpertMode                     = true;
bool cDeltaDevice::sdhdDisableExpertMode                    = true;
bool cDeltaDevice::sdhdEnableForce                          = true;
bool cDeltaDevice::sdhdIsLeftHanded                         = true;
bool cDeltaDevice::sdhdSetBaseAngleZDeg                     = true;
bool cDeltaDevice::sdhdSetVelocityThreshold                 = true;

// DRD function availability
bool cDeltaDevice::sdrdOpenID                               = true;
bool cDeltaDevice::sdrdClose                                = true;
bool cDeltaDevice::sdrdIsInitialized                        = true;
bool cDeltaDevice::sdrdAutoInit                             = true;
bool cDeltaDevice::sdrdStop                                 = true;

#if defined(WIN32) | defined(WIN64)
HINSTANCE fdDLL = NULL;

/*
int  (__stdcall *dhdSetForceAndGripperForce)          (double fx, double fy, double fz, double f, char ID);
*/

#else

#endif
//------------------------------------------------------------------------------

// Initialize dhd dll reference count
int cDeltaDevice::m_activeDeltaDevices = 0;

//------------------------------------------------------------------------------
#endif  // DOXYGEN_SHOULD_SKIP_THIS
//------------------------------------------------------------------------------

//==============================================================================
/*!
    Constructor of cDeltaDevice.
*/
//==============================================================================
cDeltaDevice::cDeltaDevice(unsigned int a_deviceNumber)
{
    // init variables
    statusEnableForcesFirstTime = true;

    // name of the device manufacturer
    m_specifications.m_manufacturerName = "Razer";

    // device is not yet available or ready
    m_deviceAvailable   = false;
    m_deviceReady       = false;
    m_deviceType        = -1;
    m_activeDeltaDevices++;

#if defined(WIN32)
    // load 32-bit fd library (favoring DRD over DHD)
    if (fdDLL==NULL) fdDLL = LoadLibrary("drd.dll");
    if (fdDLL==NULL) fdDLL = LoadLibrary("dhd.dll");
#endif
#if defined(WIN64)
    // load 64-bit fd library (favoring DRD over DHD)
    if (fdDLL==NULL) fdDLL = LoadLibrary("drd64.dll");
    if (fdDLL==NULL) fdDLL = LoadLibrary("dhd64.dll");
#endif

#if defined(WIN32) | defined(WIN64)
    // check if DLL loaded correctly
    if (fdDLL == NULL)
    {
        return;
    }

    // load different callbacks
    
    /*
    dhdGetDeviceCount = (int (__stdcall*)(void))GetProcAddress(fdDLL, "dhdGetDeviceCount");
    if (dhdGetDeviceCount == NULL) { sdhdGetDeviceCount = false; }
    */

#endif

    // display a message if some function calls are missing
    if (   (!sdhdGetDeviceCount)
        || (!sdhdGetDeviceID)
        || (!sdhdGetSystemType)
        || (!sdhdOpenID)
        || (!sdhdClose)
        || (!sdhdGetButton)
        || (!sdhdReset)
        || (!sdhdGetPosition)
        || (!sdhdGetLinearVelocity)
        || (!sdhdSetForce)
        || (!dhdSetForceAndTorque)
        || (!sdhdGetOrientationRad)
        || (!sdhdGetOrientationFrame)
        || (!sdhdSetForceAndGripperForce)
        || (!sdhdSetForceAndTorqueAndGripperForce)
        || (!sdhdGetGripperThumbPos)
        || (!sdhdGetGripperFingerPos)
        || (!dhdGetGripperAngleRad)
        || (!sdhdEnableExpertMode)
        || (!sdhdDisableExpertMode)
        || (!sdhdEnableForce)
        || (!sdhdIsLeftHanded)
        || (!sdhdSetBaseAngleZDeg)
        || (!sdhdSetVelocityThreshold)
        || (!sdrdOpenID)
        || (!sdrdClose)
        || (!sdrdIsInitialized)
        || (!sdrdAutoInit)
        || (!sdrdStop))
    {
        printf("NOTICE: For optimal performances, we suggest that you update the drivers \n        of your Force Dimension haptic device (www.forcedimension.com).\n");
    }

    // get the number ID of the device we wish to communicate with
    m_deviceNumber = a_deviceNumber;

    // get the number of Force Dimension devices connected to this computer
    int numDevices = dhdGetDeviceCount();

    // check if such device is available
    if ((a_deviceNumber + 1) > (unsigned int)numDevices)
    {
        // no, such ID does not lead to an existing device
        m_deviceAvailable = false;
    }
    else
    {
        // tentatively enable device
        m_deviceAvailable = true;

    	// open the device to read all the technical specifications about it
        if (0 > open())
        {
            // no, this ID does not lead to a valid device
            m_deviceAvailable = false;
        }
        else
        {
            // everything is ok, close the device
            close();
        }
    }

    // init code to handle user switches
    for (int i=0; i<8; i++)
    {
        m_userSwitchCount[i] = 0;
        m_userSwitchStatus[i] = 0;
        m_userSwitchClock[i].reset();
        m_userSwitchClock[i].start();
    }

	//init  virtual gripper user switch
	m_gripperUserSwitchEnabled			= false;
	m_gripperUserSwitchAngleStart		= 10;
	m_gripperUserSwitchAngleClick		= 7;
	m_gripperUserSwitchForceClick		= 3;
	m_gripperUserSwitchForceEngaged		= 2;
}


//==============================================================================
/*!
    Destructor of cDeltaDevice.
*/
//==============================================================================
cDeltaDevice::~cDeltaDevice()
{
    // close connection to device
    if (m_deviceReady)
    {
        close();
    }

    m_activeDeltaDevices--;

#if defined(WIN32) | defined(WIN64)
    if (m_activeDeltaDevices == 0 && fdDLL)
    {
      FreeLibrary(fdDLL);
      fdDLL = 0;
    }
#endif
}


//==============================================================================
/*!
    Open connection to the omega.x, delta.x or sigma.x device.
*/
//==============================================================================
int cDeltaDevice::open()
{
    // check if the system is available
    if (!m_deviceAvailable) return (-1);

    // if system is already opened then return
    if (m_deviceReady) return (0);

    // check if DHD-API call is available
    if (!sdhdOpenID) return (-1);

    // try to open the device
    int result;
    if (sdrdOpenID) result = drdOpenID(m_deviceNumber);
    else            result = dhdOpenID(m_deviceNumber);

    // update device status
    if (result >= 0)
    {
        m_deviceReady = true;
        m_deviceID    = result;
    }
    else
    {
        m_deviceReady = false;
        m_deviceID    = 0;
        return (-1);
    }

    // init force status
    statusEnableForcesFirstTime = true;

    // read the device type
    m_deviceType = DHD_DEVICE_OMEGA;
    if (sdhdGetSystemType)
    {
        m_deviceType = dhdGetSystemType(m_deviceID);
    }

    // left/right hand
    bool leftHandOnly = false;
    if (sdhdIsLeftHanded)
    {
        leftHandOnly = dhdIsLeftHanded(m_deviceID);
    }

    // default information
    m_specifications.m_model                         = C_HAPTIC_DEVICE_OMEGA_3;
    m_specifications.m_manufacturerName              = "Force Dimension";
    m_specifications.m_modelName                     = "omega";
    m_specifications.m_maxLinearForce                =   12.0;   // [N]
    m_specifications.m_maxAngularTorque              =    0.0;   // [N*m]
    m_specifications.m_maxGripperForce               =    0.0;   // [N]
    m_specifications.m_maxLinearStiffness            = 5000.0;   // [N/m]
    m_specifications.m_maxAngularStiffness           =    0.0;   // [N*m/Rad]
    m_specifications.m_maxGripperLinearStiffness     =    0.0;   // [N*m/Rad]
    m_specifications.m_maxLinearDamping              =   30.0;   // [N/(m/s)]
    m_specifications.m_maxAngularDamping             =    0.0;   // [N*m/(Rad/s)]
    m_specifications.m_maxGripperAngularDamping      =    0.0;   // [N*m/(Rad/s)]
    m_specifications.m_workspaceRadius               =    0.075; // [m]
    m_specifications.m_sensedPosition                = true;
    m_specifications.m_sensedRotation                = false;
    m_specifications.m_sensedGripper                 = false;
    m_specifications.m_actuatedPosition              = true;
    m_specifications.m_actuatedRotation              = false;
    m_specifications.m_actuatedGripper               = false;
    m_specifications.m_leftHand                      = true;
    m_specifications.m_rightHand                     = true;

    // setup information regarding device
    switch (m_deviceType)
    {
        //------------------------------------------------------------------
        // delta.x devices
        //------------------------------------------------------------------
        case (DHD_DEVICE_3DOF):
        case (DHD_DEVICE_DELTA3):
        {
            m_specifications.m_model                         = C_HAPTIC_DEVICE_DELTA_3;
            m_specifications.m_manufacturerName              = "Force Dimension";
            m_specifications.m_modelName                     = "delta.3";
            m_specifications.m_maxLinearForce                =   20.0;   // [N]
            m_specifications.m_maxAngularTorque              =    0.0;   // [N*m]
            m_specifications.m_maxGripperForce               =    0.0;   // [N]
            m_specifications.m_maxLinearStiffness            = 5000.0;   // [N/m]
            m_specifications.m_maxAngularStiffness           =    0.0;   // [N*m/Rad]
            m_specifications.m_maxGripperLinearStiffness     =    0.0;   // [N*m/Rad]
            m_specifications.m_maxLinearDamping              =   30.0;   // [N/(m/s)]
            m_specifications.m_maxAngularDamping             =    0.0;   // [N*m/(Rad/s)]
            m_specifications.m_maxGripperAngularDamping      =    0.0;   // [N*m/(Rad/s)]
            m_specifications.m_workspaceRadius               =    0.15;  // [m]
            m_specifications.m_sensedPosition                = true;
            m_specifications.m_sensedRotation                = false;
            m_specifications.m_sensedGripper                 = false;
            m_specifications.m_actuatedPosition              = true;
            m_specifications.m_actuatedRotation              = false;
            m_specifications.m_actuatedGripper               = false;
            m_specifications.m_leftHand                      = true;
            m_specifications.m_rightHand                     = true;
        }
        break;

        case (DHD_DEVICE_6DOF):
        case (DHD_DEVICE_6DOF_500):
        case (DHD_DEVICE_DELTA6):
        {
            m_specifications.m_model                         = C_HAPTIC_DEVICE_DELTA_6;
            m_specifications.m_manufacturerName              = "Force Dimension";
            m_specifications.m_modelName                     = "delta.6";
            m_specifications.m_maxLinearForce                =   20.0;   // [N]
            m_specifications.m_maxAngularTorque              =    0.2;   // [N*m]
            m_specifications.m_maxGripperForce               =    0.0;   // [N]
            m_specifications.m_maxLinearStiffness            = 5000.0;   // [N/m]
            m_specifications.m_maxAngularStiffness           =    1.0;   // [N*m/Rad]
            m_specifications.m_maxGripperLinearStiffness     =    0.0;   // [N*m/Rad]
            m_specifications.m_maxLinearDamping              =   30.0;   // [N/(m/s)]
            m_specifications.m_maxAngularDamping             =    0.0;   // [N*m/(Rad/s)]
            m_specifications.m_maxGripperAngularDamping      =    0.0;   // [N*m/(Rad/s)]
            m_specifications.m_workspaceRadius               =    0.15;  // [m]
            m_specifications.m_sensedPosition                = true;
            m_specifications.m_sensedRotation                = true;
            m_specifications.m_sensedGripper                 = false;
            m_specifications.m_actuatedPosition              = true;
            m_specifications.m_actuatedRotation              = true;
            m_specifications.m_actuatedGripper               = false;
            m_specifications.m_leftHand                      = true;
            m_specifications.m_rightHand                     = true;
        }
        break;

        //------------------------------------------------------------------
        // omega.x devices
        //------------------------------------------------------------------
        case (DHD_DEVICE_OMEGA):
        case (DHD_DEVICE_OMEGA3):
        {
            m_specifications.m_model                         = C_HAPTIC_DEVICE_OMEGA_3;
            m_specifications.m_manufacturerName              = "Force Dimension";
            m_specifications.m_modelName                     = "omega.3";
            m_specifications.m_maxLinearForce                =   12.0;   // [N]
            m_specifications.m_maxAngularTorque              =    0.0;   // [N*m]
            m_specifications.m_maxGripperForce               =    0.0;   // [N]
            m_specifications.m_maxLinearStiffness            = 5000.0;   // [N/m]
            m_specifications.m_maxAngularStiffness           =    0.0;   // [N*m/Rad]
            m_specifications.m_maxGripperLinearStiffness     =    0.0;   // [N*m/Rad]
            m_specifications.m_maxLinearDamping              =   30.0;   // [N/(m/s)]
            m_specifications.m_maxAngularDamping             =    0.0;   // [N*m/(Rad/s)]
            m_specifications.m_maxGripperAngularDamping      =    0.0;   // [N*m/(Rad/s)]
            m_specifications.m_workspaceRadius               =    0.075; // [m]
            m_specifications.m_sensedPosition                = true;
            m_specifications.m_sensedRotation                = false;
            m_specifications.m_sensedGripper                 = false;
            m_specifications.m_actuatedPosition              = true;
            m_specifications.m_actuatedRotation              = false;
            m_specifications.m_actuatedGripper               = false;
            m_specifications.m_leftHand                      = true;
            m_specifications.m_rightHand                     = true;
        }
        break;

        case (DHD_DEVICE_OMEGA33):
        case (DHD_DEVICE_OMEGA33_LEFT):
        {
            m_specifications.m_model                         = C_HAPTIC_DEVICE_OMEGA_6;
            m_specifications.m_manufacturerName              = "Force Dimension";
            m_specifications.m_modelName                     = "omega.6";
            m_specifications.m_maxLinearForce                =   12.0;   // [N]
            m_specifications.m_maxAngularTorque              =    0.0;   // [N*m]
            m_specifications.m_maxGripperForce               =    0.0;   // [N]
            m_specifications.m_maxLinearStiffness            = 5000.0;   // [N/m]
            m_specifications.m_maxAngularStiffness           =    0.0;   // [N*m/Rad]
            m_specifications.m_maxGripperLinearStiffness     =    0.0;   // [N*m/Rad]
            m_specifications.m_maxLinearDamping              =   30.0;   // [N/(m/s)]
            m_specifications.m_maxAngularDamping             =    0.0;   // [N*m/(Rad/s)]
            m_specifications.m_maxGripperAngularDamping      =    0.0;   // [N*m/(Rad/s)]
            m_specifications.m_workspaceRadius               =    0.075; // [m]
            m_specifications.m_sensedPosition                = true;
            m_specifications.m_sensedRotation                = true;
            m_specifications.m_sensedGripper                 = false;
            m_specifications.m_actuatedPosition              = true;
            m_specifications.m_actuatedRotation              = false;
            m_specifications.m_actuatedGripper               = false;
            m_specifications.m_leftHand                      = leftHandOnly;
            m_specifications.m_rightHand                     = !leftHandOnly;
        }
        break;

        case (DHD_DEVICE_OMEGA331):
        case (DHD_DEVICE_OMEGA331_LEFT):
        {
            m_specifications.m_model                         = C_HAPTIC_DEVICE_OMEGA_7;
            m_specifications.m_manufacturerName              = "Force Dimension";
            m_specifications.m_modelName                     = "omega.7";
            m_specifications.m_maxLinearForce                =   12.0;   // [N]
            m_specifications.m_maxAngularTorque              =    0.0;   // [N*m]
            m_specifications.m_maxGripperForce               =    8.0;   // [N]
            m_specifications.m_maxLinearStiffness            = 5000.0;   // [N/m]
            m_specifications.m_maxAngularStiffness           =    0.0;   // [N*m/Rad]
            m_specifications.m_maxGripperLinearStiffness     = 2000.0;   // [N*m/Rad]
            m_specifications.m_maxLinearDamping              =   30.0;   // [N/(m/s)]
            m_specifications.m_maxAngularDamping             =    0.0;   // [N*m/(Rad/s)]
            m_specifications.m_maxGripperAngularDamping      =  0.500;   // [N/(Rad/s)]
            m_specifications.m_workspaceRadius               =  0.075; // [m]
            m_specifications.m_sensedPosition                = true;
            m_specifications.m_sensedRotation                = true;
            m_specifications.m_sensedGripper                 = true;
            m_specifications.m_actuatedPosition              = true;
            m_specifications.m_actuatedRotation              = false;
            m_specifications.m_actuatedGripper               = true;
            m_specifications.m_leftHand                      = leftHandOnly;
            m_specifications.m_rightHand                     = !leftHandOnly;
        }
        break;

        //------------------------------------------------------------------
        // falcon device
        //------------------------------------------------------------------
        case(DHD_DEVICE_FALCON):
        {
            m_specifications.m_model                         = C_HAPTIC_DEVICE_FALCON;
            m_specifications.m_manufacturerName              = "Novint Technologies";
            m_specifications.m_modelName                     = "Falcon";
            m_specifications.m_maxLinearForce                =    8.0;   // [N]
            m_specifications.m_maxAngularTorque              =    0.0;   // [N*m]
            m_specifications.m_maxGripperForce               =    0.0;   // [N]
            m_specifications.m_maxLinearStiffness            = 3000.0;   // [N/m]
            m_specifications.m_maxAngularStiffness           =    0.0;   // [N*m/Rad]
            m_specifications.m_maxGripperLinearStiffness     =    0.0;   // [N*m/Rad]
            m_specifications.m_maxLinearDamping              =   20.0;   // [N/(m/s)]
            m_specifications.m_maxAngularDamping             =    0.0;   // [N*m/(Rad/s)]
            m_specifications.m_maxGripperAngularDamping      =    0.0;   // [N*m/(Rad/s)]
            m_specifications.m_workspaceRadius               =    0.04;  // [m]
            m_specifications.m_sensedPosition                = true;
            m_specifications.m_sensedRotation                = false;
            m_specifications.m_sensedGripper                 = false;
            m_specifications.m_actuatedPosition              = true;
            m_specifications.m_actuatedRotation              = false;
            m_specifications.m_actuatedGripper               = false;
            m_specifications.m_leftHand                      = true;
            m_specifications.m_rightHand                     = true;
        }
        break;

        //------------------------------------------------------------------
        // sigma.7 device
        //------------------------------------------------------------------
        case(DHD_DEVICE_SIGMA331):
        {
            m_specifications.m_model                         = C_HAPTIC_DEVICE_SIGMA_7;
            m_specifications.m_manufacturerName              = "Force Dimension";
            m_specifications.m_modelName                     = "sigma.7";
            m_specifications.m_maxLinearForce                =   12.0;   // [N]
            m_specifications.m_maxAngularTorque              =    0.2;   // [N*m]
            m_specifications.m_maxGripperForce               =    8.0;   // [N]
            m_specifications.m_maxLinearStiffness            = 4000.0;   // [N/m]
            m_specifications.m_maxAngularStiffness           =    1.0;   // [N*m/Rad]
            m_specifications.m_maxGripperLinearStiffness     = 2000.0;   // [N*m/Rad]
            m_specifications.m_maxLinearDamping              =   30.0;   // [N/(m/s)]
            m_specifications.m_maxAngularDamping             =  0.070;   // [N*m/(Rad/s)]
            m_specifications.m_maxGripperAngularDamping      =  0.500;   // [N*m/(Rad/s)]
            m_specifications.m_workspaceRadius               =  0.090;   // [m]
            m_specifications.m_sensedPosition                = true;
            m_specifications.m_sensedRotation                = true;
            m_specifications.m_sensedGripper                 = true;
            m_specifications.m_actuatedPosition              = true;
            m_specifications.m_actuatedRotation              = true;
            m_specifications.m_actuatedGripper               = true;
            m_specifications.m_leftHand                      = false;
            m_specifications.m_rightHand                     = true;
            dhdSetBaseAngleZDeg(-30, m_deviceID);
        }
        break;

        case(DHD_DEVICE_SIGMA331_LEFT):
        {
            m_specifications.m_model                         = C_HAPTIC_DEVICE_SIGMA_7;
            m_specifications.m_manufacturerName              = "Force Dimension";
            m_specifications.m_modelName                     = "sigma.7";
            m_specifications.m_maxLinearForce                =   12.0;   // [N]
            m_specifications.m_maxAngularTorque              =    0.2;   // [N*m]
            m_specifications.m_maxGripperForce               =    8.0;   // [N]
            m_specifications.m_maxLinearStiffness            = 4000.0;   // [N/m]
            m_specifications.m_maxAngularStiffness           =    1.0;   // [N*m/Rad]
            m_specifications.m_maxGripperLinearStiffness     = 2000.0;   // [N*m/Rad]
            m_specifications.m_maxLinearDamping              =   30.0;   // [N/(m/s)]
            m_specifications.m_maxAngularDamping             =  0.070;   // [N*m/(Rad/s)]
            m_specifications.m_maxGripperAngularDamping      =  0.500;   // [N*m/(Rad/s)]
            m_specifications.m_workspaceRadius               =  0.090;   // [m]
            m_specifications.m_sensedPosition                = true;
            m_specifications.m_sensedRotation                = true;
            m_specifications.m_sensedGripper                 = true;
            m_specifications.m_actuatedPosition              = true;
            m_specifications.m_actuatedRotation              = true;
            m_specifications.m_actuatedGripper               = true;
            m_specifications.m_leftHand                      = true;
            m_specifications.m_rightHand                     = false;
            dhdSetBaseAngleZDeg(30, m_deviceID);
        }
        break;
    }

    if (sdhdSetVelocityThreshold && sdhdEnableExpertMode)
    {
        dhdEnableExpertMode();
        if (0 > dhdSetVelocityThreshold(0, m_deviceID))
        {
        	if (sdrdClose) drdClose (m_deviceID);
        	else           dhdClose (m_deviceID);
            m_deviceReady = false;
            m_deviceID    = -1;
            return (-1);
        }
    }

    // success
    return (0);
}


//==============================================================================
/*!
    Close connection to the omega.x or delta.x device.
*/
//==============================================================================
int cDeltaDevice::close()
{
    // check if the system has been opened previously
    if (!m_deviceReady) return (-1);

    // check if DHD-API call is available
    if (!sdhdClose) return (-1);

    // yes, the device is open so let's close it
    int result;
    if (sdrdClose) result = drdClose(m_deviceID);
    else           result = dhdClose(m_deviceID);

    // update status
    m_deviceReady = false;

    // reset force status
    statusEnableForcesFirstTime = true;

    return (result);
}


//==============================================================================
/*!
    Calibrate the omega.x or delta.x device.

    \return Always 0
*/
//==============================================================================
int cDeltaDevice::calibrate(bool force)
{
    // check if the system is available
    if (!m_deviceReady) return (0);

    // check if DRD-API call is available
    if (!sdrdAutoInit || !sdrdStop || !sdrdIsInitialized)
    {
        // check if DHD-API call is available
        if (!sdhdReset) return (1);

        // reset device (asking user to calibrate manually)
        if (force && 0 > dhdReset(m_deviceID)) return -1;

        return 0;
    }

    // auto-calibrate is necessary
    if (force || !drdIsInitialized(m_deviceID))
    {
    	if (0 > drdAutoInit(m_deviceID))   return -1;
    	if (0 > drdStop(true, m_deviceID)) return -1;
    }

    return 0;
}


//==============================================================================
/*!
    Returns the number of devices available from this class of device.

    \return  Returns the result
*/
//==============================================================================
unsigned int cDeltaDevice::getNumDevices()
{
    // check if the system is available
    if (!m_deviceAvailable) return (0);

    // check if DHD-API call is available
    if (!sdhdGetDeviceCount) return (1);

    int result = dhdGetDeviceCount();
    return (result);
}


//==============================================================================
/*!
    Read the position of the device. Units are meters [m].

    \param  a_position  Return value.

    \return Return 0 if no error occurred.
*/
//==============================================================================
int cDeltaDevice::getPosition(cVector3d& a_position)
{
    // check if the system is available
    if (!m_deviceReady) return (-1);

    // check if DHD-API call is available
    if (!sdhdGetPosition) return (-1);

    int error = -1;
    double x,y,z;
    error = dhdGetPosition(&x, &y, &z, m_deviceID);
    if (error >= 0)
    {
        a_position.set(x, y, z);
#if !defined(MACOSX) & !defined(LINUX)
        estimateLinearVelocity(a_position);
#endif
    }
    return (error);
}


//==============================================================================
/*!
    Read the linear velocity of the device. Units are in [m/s].

    \param  a_linearVelocity  Return value.

    \return Return 0 if no error occurred.
*/
//==============================================================================
int cDeltaDevice::getLinearVelocity(cVector3d& a_linearVelocity)
{
    // check if the system is available
    if (!m_deviceReady) return (-1);

    int error = 0;

#if defined(MACOSX) | defined(LINUX)
    double vx,vy,vz;
    error = dhdGetLinearVelocity(&vx, &vy, &vz, m_deviceID);
    if (error >= 0)
    {
        m_linearVelocity.set(vx, vy, vz);
    }
#endif

    a_linearVelocity = m_linearVelocity;

    return (error);
}


//==============================================================================
/*!
    Read the orientation frame of the device end-effector.

    \param  a_rotation  Return value.

    \return Return 0 if no error occurred.
*/
//==============================================================================
int cDeltaDevice::getRotation(cMatrix3d& a_rotation)
{
    // check if the system is available
    if (!m_deviceReady) return (-1);

    int error = 0;
    cMatrix3d frame;
    frame.identity();

    switch (m_deviceType)
    {
        // delta devices
        case (DHD_DEVICE_3DOF):
        case (DHD_DEVICE_6DOF):
        case (DHD_DEVICE_6DOF_500):
        {
            // read angles
            cVector3d angles;
            angles.set(0,0,0);

            // check if DHD-API call is available
            if (sdhdGetOrientationRad)
            {
                error = dhdGetOrientationRad(&angles(0) , &angles(1) , &angles(2) , m_deviceID);
            }

            // compute rotation matrix
            if (error >= 0)
            {
                angles.mul(1.5);
                frame.rotateAboutGlobalAxisRad(cVector3d(1,0,0), angles(0) );
                frame.rotateAboutGlobalAxisRad(cVector3d(0,1,0), angles(1) );
                frame.rotateAboutGlobalAxisRad(cVector3d(0,0,1), angles(2) );
            }
        }
        break;

        // omega devices
        case (DHD_DEVICE_OMEGA):
        case (DHD_DEVICE_OMEGA3):
        case (DHD_DEVICE_OMEGA33):
        case (DHD_DEVICE_OMEGA33_LEFT):
        case (DHD_DEVICE_OMEGA331):
        case (DHD_DEVICE_OMEGA331_LEFT):
        case (DHD_DEVICE_SIGMA331):
        case (DHD_DEVICE_SIGMA331_LEFT):
        {
            // read rotation matrix
            double rot[3][3];
            rot[0][0] = 1.0; rot[0][1] = 0.0; rot[0][2] = 0.0;
            rot[1][0] = 0.0; rot[1][1] = 1.0; rot[1][2] = 0.0;
            rot[2][0] = 0.0; rot[2][1] = 0.0; rot[2][2] = 1.0;

            if (sdhdGetOrientationFrame)
            {
                error = dhdGetOrientationFrame(rot, m_deviceID);
            }

            if (error >= 0)
            {
                frame(0,0) = rot[0][0];
                frame(0,1) = rot[0][1];
                frame(0,2) = rot[0][2];
                frame(1,0) = rot[1][0];
                frame(1,1) = rot[1][1];
                frame(1,2) = rot[1][2];
                frame(2,0) = rot[2][0];
                frame(2,1) = rot[2][1];
                frame(2,2) = rot[2][2];
            }
        }
        break;
    }

    // return result
    a_rotation = frame;
    estimateAngularVelocity(a_rotation);
    return (error);
}


//==============================================================================
/*!
    Read the gripper angle in radian.

    \param  a_angle  Return value.

    \return Return 0 if no error occurred.
*/
//==============================================================================
int cDeltaDevice::getGripperAngleRad(double& a_angle)
{
    // default value
    a_angle = 0.0;

    // check if the system is available
    if (!m_deviceReady) return (-1);
	
	// read gripper angle
	int error = 0;
	if (m_specifications.m_sensedGripper)
	{
		if (sdhdGetGripperAngleRad)
		{
			double angle = 0.0;
			error = dhdGetGripperAngleRad(&angle, m_deviceID);

			if (m_specifications.m_rightHand)
			{
				a_angle = cClamp0(angle);
			}
			else if (m_specifications.m_leftHand)
			{
				a_angle =cClamp0(-angle);
			}
		}
		else
		{
			a_angle = 0.0;
		}

		estimateGripperVelocity(a_angle);
	}
	else
	{
		cGenericHapticDevice::getGripperAngleRad(a_angle);
		error = 0;
	}

    // return result
    return (error);
}


//==============================================================================
/*!
    Send a force [N] to the haptic device.

    \param  a_force  Force command to be applied to device.

    \return Return 0 if no error occurred.
*/
//==============================================================================
int cDeltaDevice::setForce(const cVector3d& a_force)
{
    // check if the system is available
    if (!m_deviceReady) return (-1);

    // check if forces need to be enable (happens only once)
    if (statusEnableForcesFirstTime) { enableForces(true); }

    // check if DHD-API call is available
    if (!sdhdSetForce) return (-1);

    int error = dhdSetForce(a_force(0) , a_force(1) , a_force(2) , m_deviceID);
    m_prevForce = a_force;
    return (error);
}


//==============================================================================
/*!
    Send a force [N] and torque [N*m] to the haptic device.

    \param  a_force Force command to be applied to device.
    \param  a_torque Torque command to be applied to device.

    \return  Return 0 if no error occurred.
*/
//==============================================================================
int cDeltaDevice::setForceAndTorque(const cVector3d& a_force, 
                                    const cVector3d& a_torque)
{
    // check if the system is available
    if (!m_deviceReady) return (-1);


    // check if forces need to be enable (happens only once)
    if (statusEnableForcesFirstTime) { enableForces(true); }

    // proccess command
    if (m_specifications.m_actuatedRotation)
    {
        int error = 0;
        if (sdhdSetForceAndTorque)
        {
            int error = dhdSetForceAndTorque(a_force(0), a_force(1), a_force(2),
                                             a_torque(0), a_torque(1), a_torque(2),
                                             m_deviceID);
            if (error < 0) { return (error); }

            // store new commanded values
            m_prevForce  = a_force;
            m_prevTorque = a_torque;
        }
        else if ((sdhdSetForce) && (sdhdSetTorque))
        {
            int error = 0;
            error = dhdSetForce(a_force(0), a_force(1), a_force(2), m_deviceID);
            if (error < 0) { return (error); }

            error = dhdSetTorque( a_torque(0), a_torque(1), a_torque(2), m_deviceID);
            if (error < 0) { return (error); }

            // store new commanded values
            m_prevForce  = a_force;
            m_prevTorque = a_torque;        
        }
    }
    else
    {
        int error = 0;
        return (setForce(a_force));
    }

    // success
    return (0);
}


//==============================================================================
/*!
    Send a force [N] and a torque [N*m] and gripper torque [N*m] to the haptic device.

    \param  a_force  Force command.
    \param  a_torque  Torque command.
    \param  a_gripperForce  Gripper force command.

    \return Return 0 if no error occurred.
*/
//==============================================================================
int cDeltaDevice::setForceAndTorqueAndGripperForce(const cVector3d& a_force, 
                                                   const cVector3d& a_torque, 
                                                   double a_gripperForce)
{
    // check if the system is available
    if (!m_deviceReady) return (-1);

    // check if forces need to be enable (happens only once)
    if (statusEnableForcesFirstTime) { enableForces(true); }

    // apply force and gripper torque
    if (m_specifications.m_actuatedGripper)
    {
		// computer gripper user switch gripper force
		double gripperAngle;
		double gripperUserSwitchForce = 0.0;
		if (getGripperAngleDeg(gripperAngle) == 1)
		{
			gripperUserSwitchForce = computeGripperUserSwitchForce(gripperAngle, 0.0);
		}

		// adjust gripper command for left or right hand device
        double gripperForce = a_gripperForce;
        if (m_specifications.m_leftHand)
        {
            gripperForce =-a_gripperForce;
        }

        if (m_specifications.m_actuatedRotation)
        {
            if (sdhdSetForceAndTorqueAndGripperForce)
            {
                int error = dhdSetForceAndTorqueAndGripperForce(a_force(0), 
																a_force(1), 
																a_force(2), 
                                                                a_torque(0), 
																a_torque(1), 
																a_torque(2),
                                                                gripperForce + gripperUserSwitchForce, 
																m_deviceID);
                if (error < 0) { return (error); }

                // store new commanded values
                m_prevForce  = a_force;
                m_prevTorque  = a_torque;
                m_prevGripperForce = gripperForce;   
            }
            else
            {
                return (-1);
            }
        }
        else
        {
            if (sdhdSetForceAndGripperForce)
            {
                int error = dhdSetForceAndGripperForce(a_force(0), 
													   a_force(1), 
													   a_force(2), 
													   gripperForce + gripperUserSwitchForce, 
													   m_deviceID);
                if (error < 0) { return (error); }

                // store new commanded values
                m_prevForce = a_force;
                m_prevGripperForce = gripperForce;   
            }
            else
            {
                return (-1);
            }
        }
    }
    else
    {
        return (setForceAndTorque(a_force, a_torque));    
    }

    // success
    return (0);
}


//==============================================================================
/*!
    Read the status of the user switch [1 = \e ON / 0 = \e OFF].

    \param  a_switchIndex  index number of the switch.
    \param  a_status result value from reading the selected input switch.

    \return Return 0 if no error occurred.
*/
//==============================================================================
int cDeltaDevice::getUserSwitch(int a_switchIndex, 
                                bool& a_status)
{
    // check if the system is available
    if (!m_deviceReady) return (-1);

    int result = dhdGetButton (a_switchIndex, m_deviceID);
    
    if (result < 0)
    {
        a_status = false || getGripperUserSwitch();
        return (-1);
    }

    if (result > 0)
    {
        a_status = true;
    }
    else
    {
        a_status = false || getGripperUserSwitch();
    }

    return (0);
}


//==============================================================================
/*!
    Enable/Disable the motors of the omega.x device.
    This function overrides the force button located at the base of the
    device or on the controller panel.

    \param      a_value  force status.

    \return     Return 0 if no error occurred.
*/
//==============================================================================
int cDeltaDevice::enableForces(bool a_value)
{
    // check if DHD-API call is available
    if (!sdhdEnableExpertMode) return (-1);
    if (!sdhdEnableForce) return (-1);
    if (!sdhdDisableExpertMode) return (-1);

    if (a_value)
    {
        // enable forces
        dhdEnableExpertMode();
        dhdEnableForce(DHD_ON, m_deviceID);
        dhdDisableExpertMode();
        statusEnableForcesFirstTime = false;
    }
    else
    {
        // disable forces
        dhdEnableExpertMode();
        dhdEnableForce(DHD_OFF, m_deviceID);
        dhdDisableExpertMode();
    }

    return (0);
}


//------------------------------------------------------------------------------
} // namespace chai3d
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
#endif //C_ENABLE_DELTA_DEVICE_SUPPORT
//------------------------------------------------------------------------------
