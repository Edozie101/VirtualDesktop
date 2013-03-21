//
//  sixense_controller.m
//  IbexMac
//
//  Created by Hesham Wahba on 3/14/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#include "sixense_controller.h"

#include <sixense.h>
#include <sixense_math.hpp>
#ifdef WIN32
#include <sixense_utils/mouse_pointer.hpp>
#endif
#include <sixense_utils/derivatives.hpp>
#include <sixense_utils/button_states.hpp>
#include <sixense_utils/event_triggers.hpp>
#include <sixense_utils/controller_manager/controller_manager.hpp>

#include <iostream>
#include <deque>


#include <OpenGL/OpenGL.h>
#include "ibex.h"

double sixenseStrafeRight = 0;
double sixenseWalkForward = 0;

// whether or not we are currently logging position data to a file, and the file pointer to which to log
static int is_logging = 0;
static FILE *log_file = 0;

// whether or not to write the current controller positions on the screen.
static int display_pos_enabled = 0;

// Zoom factor for the camera, press [ and ] to zoom in or out
static float camera_dist = 1.0f;

// The current mode of the real-time graph display
static int graph_mode = 0; // 0 == off, 1 == pos, 2 == vel, 3 == accel
static bool graph_paused = false;
static bool auto_graph_bounds = false;
static float graph_bounds[2] = {-750, 750};

// flags that the controller manager system can set to tell the graphics system to draw the instructions
// for the player
static bool controller_manager_screen_visible = true;
std::string controller_manager_text_string;

// these are used by the graphics to highlight one of the controller 3d objects for a number of frames
static int flash_left_controller_frames=0, flash_right_controller_frames=0;

// pressing 'm' turns on drawing of 2d mouse cursors controlled by each controller
static bool draw_mouse_pointers_enabled = false;
static float left_mouse_pos[2]={0,0}, right_mouse_pos[2]={0,0};
static float left_mouse_roll=0.0f, right_mouse_roll=0.0f;

// Log a number of samples for graphing
const int log_history_size = 1000;
std::deque<sixenseMath::Vector3> pos_hist, vel_hist, accel_hist;


// This is the callback that gets registered with the sixenseUtils::controller_manager. It will get called each time the user completes
// one of the setup steps so that the game can update the instructions to the user. If the engine supports texture mapping, the
// controller_manager can prove a pathname to a image file that contains the instructions in graphic form.
// The controller_manager serves the following functions:
//  1) Makes sure the appropriate number of controllers are connected to the system. The number of required controllers is designaged by the
//     game type (ie two player two controller game requires 4 controllers, one player one controller game requires one)
//  2) Makes the player designate which controllers are held in which hand.
//  3) Enables hemisphere tracking by calling the Sixense API call sixenseAutoEnableHemisphereTracking. After this is completed full 360 degree
//     tracking is possible.
void controller_manager_setup_callback( sixenseUtils::ControllerManager::setup_step step ) {
    
	if( sixenseUtils::getTheControllerManager()->isMenuVisible() ) {
        
		// Turn on the flag that tells the graphics system to draw the instruction screen instead of the controller information. The game
		// should be paused at this time.
		controller_manager_screen_visible = true;
        
		// Ask the controller manager what the next instruction string should be.
		controller_manager_text_string = sixenseUtils::getTheControllerManager()->getStepString();
        
		// We could also load the supplied controllermanager textures using the filename: sixenseUtils::getTheControllerManager()->getTextureFileName();
        
	} else {
        
		// We're done with the setup, so hide the instruction screen.
		controller_manager_screen_visible = false;
        
	}
    
}
// This function causes the 3D objects to flash when the buttons are pressed. It does so using two different techniques
// available using sixenseUtils
void check_for_button_presses( sixenseAllControllerData *acd ) {
    
	// Ask the controller manager which controller is in the left hand and which is in the right
	int left_index = sixenseUtils::getTheControllerManager()->getIndex( sixenseUtils::ControllerManager::P1L );
	int right_index = sixenseUtils::getTheControllerManager()->getIndex( sixenseUtils::ControllerManager::P1R );
    
    
    
	// First use the 'ButtonStates' class to flash the object when the 1 button is pressed, or the trigger is pulled.
	// ButtonStates is a simple class that reports when a button's state just transitioned from released to pressed
	// or vice versa. It also detects when the trigger crosses a programmable threshold.
	static sixenseUtils::ButtonStates left_states, right_states;
    
	left_states.update( &acd->controllers[left_index] );
	right_states.update( &acd->controllers[right_index] );
    
	// Do something if the button was pressed
	if( left_states.buttonJustPressed( SIXENSE_BUTTON_1 ) ) {
		flash_left_controller_frames = 20;
	}
    
	if( right_states.buttonJustPressed( SIXENSE_BUTTON_1 ) ) {
		flash_right_controller_frames = 20;
	}
    
	// Or if the trigger was pulled
	if( left_states.triggerJustPressed() ) {
		flash_left_controller_frames = 20;
	}
    
	if( right_states.triggerJustPressed() ) {
		flash_right_controller_frames = 20;
	}
    
    relativeMouseX += acd->controllers[left_index].joystick_x*30;
//    relativeMouseY += acd->controllers[left_index].joystick_y;
    
    relativeMouseX += acd->controllers[right_index].joystick_x*10;
//    if(fabs(acd->controllers[right_index].joystick_x) > 0.0000001) {
//        sixenseStrafeRight = acd->controllers[right_index].joystick_x > 0 ? 1 : -1;
//    } else {
//        sixenseStrafeRight = 0;
//    }
    if(fabs(acd->controllers[right_index].joystick_y) > 0.0000001) {
        sixenseWalkForward = acd->controllers[right_index].joystick_y > 0 ? 1 : -1;
    } else {
        sixenseWalkForward = 0;
    }
    
    
    
	// Now do the same thing but use event triggers to flash the object when a button is pressed, or when the
	// controller moves to a certain height.
	// EventTriggers are very flexible objects that can be used to check for transitions of controller state including buttons being pressed, controllers moving a certain distance,
	// or exceeding a certain velocity.
	class FlashObjectTrigger : public sixenseUtils::EventTriggerBase {
		int &enable_for_frames;
	public:
		FlashObjectTrigger( int &i ) : enable_for_frames( i ) {}
		virtual void trigger() const {
			enable_for_frames = 20;
		}
	};
    
	// First make a couple of BinaryEventSwitch that flash the object when the test parameter changes from false to true. Use a null trigger for when it transitions
	// from true to false.
	static sixenseUtils::EventSwitchBase *left_button_switch = new sixenseUtils::BinaryEventSwitch( new FlashObjectTrigger( flash_left_controller_frames ), new sixenseUtils::NullEventTrigger );
	static sixenseUtils::EventSwitchBase *right_button_switch = new sixenseUtils::BinaryEventSwitch( new FlashObjectTrigger( flash_right_controller_frames ), new sixenseUtils::NullEventTrigger );
	left_button_switch->test( ((acd->controllers)[left_index].buttons & SIXENSE_BUTTON_4) ? 1.0f : 0.0f ); // test against the current state of the 4 button
	right_button_switch->test( ((acd->controllers)[right_index].buttons & SIXENSE_BUTTON_4) ? 1.0f : 0.0f );
    
	// First make a couple of BinaryEventSwitch that flash the object when the controller moves above a 200mm. Do nothing when it transitions back down.
	// ValuatorEventSwitches can be used to test against any floating point value, including position, velocity, trigger positions, joystick positions, rotation angles, etc.
	static sixenseUtils::EventSwitchBase *left_height_switch = new sixenseUtils::ValuatorEventSwitch( 200.0f, new FlashObjectTrigger( flash_left_controller_frames ), new sixenseUtils::NullEventTrigger );
	static sixenseUtils::EventSwitchBase *right_height_switch = new sixenseUtils::ValuatorEventSwitch( 200.0f, new FlashObjectTrigger( flash_right_controller_frames ), new sixenseUtils::NullEventTrigger );
	left_button_switch->test( (acd->controllers)[left_index].pos[1] ); // test the y position (height)
	right_button_switch->test( (acd->controllers)[right_index].pos[1] );
}

void mySixenseRefresh() {
    // update the controller manager with the latest controller data here
	sixenseSetActiveBase(0);
	sixenseAllControllerData acd;
	sixenseGetAllNewestData( &acd );
	sixenseUtils::getTheControllerManager()->update( &acd );
    
	check_for_button_presses( &acd );
    
#ifdef WIN32
	update_mouse_pointers( &acd );
#endif
    
	// Either draw the controller manager instruction screen, or display the controller information
	if( controller_manager_screen_visible ) {
        //		draw_controller_manager_screen();
	} else {
        //		draw_controller_info();
	}
}

void myInitSixense() {
    // Init sixense
	sixenseInit();
    
	// Init the controller manager. This makes sure the controllers are present, assigned to left and right hands, and that
	// the hemisphere calibration is complete.
	sixenseUtils::getTheControllerManager()->setGameType( sixenseUtils::ControllerManager::ONE_PLAYER_TWO_CONTROLLER );
	sixenseUtils::getTheControllerManager()->registerSetupCallback( controller_manager_setup_callback );
}
