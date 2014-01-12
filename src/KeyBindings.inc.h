// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef KEY_BINDING
#define KEY_BINDING(name, config_name, ui_name, default_value_1, default_value_2)
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
KEY_BINDING(targetObject, "BindTargetObject", Lang::TARGET_OBJECT_IN_SIGHTS, SDLK_t, 0)
KEY_BINDING(fireLaser, "BindFireLaser", Lang::FIRE_LASER, SDLK_SPACE, 0)
KEY_BINDING(fireMissile, "BindFireMissile", Lang::FIRE_MISSILE, SDLK_m, 0)

BINDING_GROUP(Lang::SHIP_ORIENTATION)
KEY_BINDING(pitchUp, "BindPitchUp", Lang::PITCH_UP, SDLK_s, 0)
KEY_BINDING(pitchDown, "BindPitchDown", Lang::PITCH_DOWN, SDLK_w, 0)
KEY_BINDING(yawLeft, "BindYawLeft", Lang::YAW_LEFT, SDLK_a, 0)
KEY_BINDING(yawRight, "BindYawRight", Lang::YAW_RIGHT, SDLK_d, 0)
KEY_BINDING(rollLeft, "BindRollLeft", Lang::ROLL_LEFT, SDLK_q, 0)
KEY_BINDING(rollRight, "BindRollRight", Lang::ROLL_RIGHT, SDLK_e, 0)

BINDING_GROUP(Lang::MANUAL_CONTROL_MODE)
KEY_BINDING(thrustForward, "BindThrustForward", Lang::THRUSTER_MAIN, SDLK_i, 0)
KEY_BINDING(thrustBackwards, "BindThrustBackwards", Lang::THRUSTER_RETRO, SDLK_k, 0)
KEY_BINDING(thrustUp, "BindThrustUp", Lang::THRUSTER_VENTRAL, SDLK_u, 0)
KEY_BINDING(thrustDown, "BindThrustDown", Lang::THRUSTER_DORSAL, SDLK_o, 0)
KEY_BINDING(thrustLeft, "BindThrustLeft", Lang::THRUSTER_PORT, SDLK_j, 0)
KEY_BINDING(thrustRight, "BindThrustRight", Lang::THRUSTER_STARBOARD, SDLK_l, 0)
KEY_BINDING(thrustLowPower, "BindThrustLowPower", Lang::USE_LOW_THRUST, SDLK_LSHIFT, 0)

BINDING_GROUP(Lang::SPEED_CONTROL_MODE)
KEY_BINDING(increaseSpeed, "BindIncreaseSpeed", Lang::INCREASE_SET_SPEED, SDLK_RETURN, 0)
KEY_BINDING(decreaseSpeed, "BindDecreaseSpeed", Lang::DECREASE_SET_SPEED, SDLK_RSHIFT, 0)

BINDING_GROUP(Lang::SCANNER_CONTROL)
KEY_BINDING(toggleScanMode, "BindToggleScanMode", Lang::TOGGLE_SCAN_MODE, SDLK_BACKSLASH, 0)
KEY_BINDING(increaseScanRange, "BindIncreaseScanRange", Lang::INCREASE_SCAN_RANGE, SDLK_RIGHTBRACKET, 0)
KEY_BINDING(decreaseScanRange, "BindDecreaseScanRange", Lang::DECREASE_SCAN_RANGE, SDLK_LEFTBRACKET, 0)

BINDING_GROUP(Lang::MISCELLANEOUS)
KEY_BINDING(toggleHudMode, "BindToggleHudMode", Lang::TOGGLE_HUD_MODE, SDLK_TAB, 0)
KEY_BINDING(toggleLuaConsole, "BindToggleLuaConsole", Lang::TOGGLE_LUA_CONSOLE, SDLK_BACKQUOTE, 0)
KEY_BINDING(toggleRotationDamping, "BindToggleRotationDamping", Lang::TOGGLE_ROTATION_DAMPING, SDLK_r, 0)

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
KEY_BINDING(frontCamera, "BindFrontCamera", Lang::CAMERA_FRONT_VIEW, SDLK_KP_8, SDLK_UP)
KEY_BINDING(rearCamera, "BindRearCamera", Lang::CAMERA_REAR_VIEW, SDLK_KP_2, SDLK_DOWN)
KEY_BINDING(leftCamera, "BindLeftCamera", Lang::CAMERA_LEFT_VIEW, SDLK_KP_4, SDLK_LEFT)
KEY_BINDING(rightCamera, "BindRightCamera", Lang::CAMERA_RIGHT_VIEW, SDLK_KP_6, SDLK_RIGHT)
KEY_BINDING(topCamera, "BindTopCamera", Lang::CAMERA_TOP_VIEW, SDLK_KP_9, SDLK_PAGEUP)
KEY_BINDING(bottomCamera, "BindBottomCamera", Lang::CAMERA_BOTTOM_VIEW, SDLK_KP_3, SDLK_PAGEDOWN)

BINDING_GROUP(Lang::EXTERNAL_VIEW)
KEY_BINDING(cameraRollLeft, "BindCameraRollLeft", Lang::ROLL_LEFT, SDLK_KP_1, 0)
KEY_BINDING(cameraRollRight, "BindCameraRollRight", Lang::ROLL_RIGHT, SDLK_KP_3, 0)
KEY_BINDING(cameraRotateDown, "BindCameraRotateDown", Lang::ROTATE_DOWN, SDLK_KP_2, SDLK_DOWN)
KEY_BINDING(cameraRotateUp, "BindCameraRotateUp", Lang::ROTATE_UP, SDLK_KP_8, SDLK_UP)
KEY_BINDING(cameraRotateLeft, "BindCameraRotateLeft", Lang::ROTATE_LEFT, SDLK_KP_4, SDLK_LEFT)
KEY_BINDING(cameraRotateRight, "BindCameraRotateRight", Lang::ROTATE_RIGHT, SDLK_KP_6, SDLK_RIGHT)
KEY_BINDING(cameraZoomIn, "BindCameraZoomIn", Lang::ZOOM_IN, SDLK_KP_PLUS, SDLK_EQUALS)
KEY_BINDING(cameraZoomOut, "BindCameraZoomOut", Lang::ZOOM_OUT, SDLK_KP_MINUS, SDLK_MINUS)
KEY_BINDING(resetCamera, "BindResetCamera", Lang::RESET, SDLK_HOME, 0)

BINDING_PAGE_END()

#undef KEY_BINDING
#undef AXIS_BINDING
#undef BINDING_GROUP
#undef BINDING_PAGE
#undef BINDING_PAGE_END
