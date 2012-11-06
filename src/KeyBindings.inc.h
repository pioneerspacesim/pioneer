// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef KEY_BINDING
#define KEY_BINDING(name, config_name, ui_name, default_value)
#endif
#ifndef AXIS_BINDING
#define AXIS_BINDING(name, config_name, ui_name, default_value)
#endif
#ifndef BINDING_GROUP
#define BINDING_GROUP(ui_name)
#endif
#ifndef BINDING_PAGE
#define BINDING_PAGE(ui_name)
#endif
#ifndef BINDING_PAGE_END
#define BINDING_PAGE_END()
#endif

BINDING_PAGE(CONTROLS)

BINDING_GROUP(Lang::WEAPONS)
KEY_BINDING(targetObject, "BindTargetObject", Lang::TARGET_OBJECT_IN_SIGHTS, SDLK_t)
KEY_BINDING(fireLaser, "BindFireLaser", Lang::FIRE_LASER, SDLK_SPACE)

BINDING_GROUP(Lang::SHIP_ORIENTATION)
KEY_BINDING(pitchUp, "BindPitchUp", Lang::PITCH_UP, SDLK_s)
KEY_BINDING(pitchDown, "BindPitchDown", Lang::PITCH_DOWN, SDLK_w)
KEY_BINDING(yawLeft, "BindYawLeft", Lang::YAW_LEFT, SDLK_a)
KEY_BINDING(yawRight, "BindYawRight", Lang::YAW_RIGHT, SDLK_d)
KEY_BINDING(rollLeft, "BindRollLeft", Lang::ROLL_LEFT, SDLK_q)
KEY_BINDING(rollRight, "BindRollRight", Lang::ROLL_RIGHT, SDLK_e)

BINDING_GROUP(Lang::MANUAL_CONTROL_MODE)
KEY_BINDING(thrustForward, "BindThrustForward", Lang::THRUSTER_MAIN, SDLK_i)
KEY_BINDING(thrustBackwards, "BindThrustBackwards", Lang::THRUSTER_RETRO, SDLK_k)
KEY_BINDING(thrustUp, "BindThrustUp", Lang::THRUSTER_VENTRAL, SDLK_u)
KEY_BINDING(thrustDown, "BindThrustDown", Lang::THRUSTER_DORSAL, SDLK_o)
KEY_BINDING(thrustLeft, "BindThrustLeft", Lang::THRUSTER_PORT, SDLK_j)
KEY_BINDING(thrustRight, "BindThrustRight", Lang::THRUSTER_STARBOARD, SDLK_l)
KEY_BINDING(thrustLowPower, "BindThrustLowPower", Lang::USE_LOW_THRUST, SDLK_LSHIFT)

BINDING_GROUP(Lang::SPEED_CONTROL_MODE)
KEY_BINDING(increaseSpeed, "BindIncreaseSpeed", Lang::INCREASE_SET_SPEED, SDLK_RETURN)
KEY_BINDING(decreaseSpeed, "BindDecreaseSpeed", Lang::DECREASE_SET_SPEED, SDLK_RSHIFT)

BINDING_GROUP(Lang::SCANNER_CONTROL)
KEY_BINDING(toggleScanMode, "BindToggleScanMode", Lang::TOGGLE_SCAN_MODE, SDLK_BACKSLASH)
KEY_BINDING(increaseScanRange, "BindIncreaseScanRange", Lang::INCREASE_SCAN_RANGE, SDLK_RIGHTBRACKET)
KEY_BINDING(decreaseScanRange, "BindDecreaseScanRange", Lang::DECREASE_SCAN_RANGE, SDLK_LEFTBRACKET)

BINDING_GROUP(Lang::MISCELLANEOUS)
KEY_BINDING(toggleHudMode, "BindToggleHudMode", Lang::TOGGLE_HUD_MODE, SDLK_TAB)
KEY_BINDING(toggleLuaConsole, "BindToggleLuaConsole", Lang::TOGGLE_LUA_CONSOLE, SDLK_BACKQUOTE)
KEY_BINDING(toggleManualRotation, "BindToggleManualRotation", Lang::TOGGLE_MANUAL_ROTATION, SDLK_r)

BINDING_GROUP(Lang::JOYSTICK_INPUT)
AXIS_BINDING(pitchAxis, "BindAxisPitch", Lang::PITCH, "-Joy0Axis1")
AXIS_BINDING(rollAxis, "BindAxisRoll", Lang::ROLL, "Joy0Axis2")
AXIS_BINDING(yawAxis, "BindAxisYaw", Lang::YAW, "Joy0Axis0")

BINDING_PAGE_END()

// not yet implemented/used
// AXIS_BINDING(thrustRightAxis, "BindAxisThrustRight", , DEFAULT_BINDING)
// AXIS_BINDING(thrustUpAxis, "BindAxisThrustUp", , DEFAULT_BINDING)
// AXIS_BINDING(thrustForwardAxis, "BindAxisThrustForward", , DEFAULT_BINDING)

BINDING_PAGE(VIEW)

BINDING_GROUP(Lang::INTERNAL_VIEW)
KEY_BINDING(frontCamera, "BindFrontCamera", Lang::CAMERA_FRONT_VIEW, SDLK_KP8)
KEY_BINDING(rearCamera, "BindRearCamera", Lang::CAMERA_REAR_VIEW, SDLK_KP2)
KEY_BINDING(leftCamera, "BindLeftCamera", Lang::CAMERA_LEFT_VIEW, SDLK_KP4)
KEY_BINDING(rightCamera, "BindRightCamera", Lang::CAMERA_RIGHT_VIEW, SDLK_KP6)
KEY_BINDING(topCamera, "BindTopCamera", Lang::CAMERA_TOP_VIEW, SDLK_KP9)
KEY_BINDING(bottomCamera, "BindBottomCamera", Lang::CAMERA_BOTTOM_VIEW, SDLK_KP3)

BINDING_GROUP(Lang::EXTERNAL_VIEW)
KEY_BINDING(cameraRollLeft, "BindCameraRollLeft", Lang::ROLL_LEFT, SDLK_KP1)
KEY_BINDING(cameraRollRight, "BindCameraRollRight", Lang::ROLL_RIGHT, SDLK_KP3)
KEY_BINDING(cameraRotateDown, "BindCameraRotateDown", Lang::ROTATE_DOWN, SDLK_KP2)
KEY_BINDING(cameraRotateUp, "BindCameraRotateUp", Lang::ROTATE_UP, SDLK_KP8)
KEY_BINDING(cameraRotateLeft, "BindCameraRotateLeft", Lang::ROTATE_LEFT, SDLK_KP4)
KEY_BINDING(cameraRotateRight, "BindCameraRotateRight", Lang::ROTATE_RIGHT, SDLK_KP6)
KEY_BINDING(cameraZoomIn, "BindCameraZoomIn", Lang::ZOOM_IN, SDLK_KP_PLUS)
KEY_BINDING(cameraZoomOut, "BindCameraZoomOut", Lang::ZOOM_OUT, SDLK_KP_MINUS)
KEY_BINDING(resetCamera, "BindResetCamera", Lang::RESET, SDLK_HOME)

BINDING_PAGE_END()

#undef KEY_BINDING
#undef AXIS_BINDING
#undef BINDING_GROUP
#undef BINDING_PAGE
#undef BINDING_PAGE_END
