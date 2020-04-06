// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
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
BINDING_GROUP(Lang::MISCELLANEOUS)
KEY_BINDING(toggleLuaConsole, "BindToggleLuaConsole", Lang::TOGGLE_LUA_CONSOLE, SDLK_BACKQUOTE, 0)

BINDING_GROUP(Lang::RADAR_CONTROL)
KEY_BINDING(toggleScanMode, "BindToggleScanMode", Lang::TOGGLE_RADAR_MODE, SDLK_BACKSLASH, 0)
KEY_BINDING(increaseScanRange, "BindIncreaseScanRange", Lang::INCREASE_RADAR_RANGE, SDLK_RIGHTBRACKET, 0)
KEY_BINDING(decreaseScanRange, "BindDecreaseScanRange", Lang::DECREASE_RADAR_RANGE, SDLK_LEFTBRACKET, 0)

BINDING_PAGE_END()

BINDING_PAGE(VIEW)

BINDING_GROUP(Lang::INTERNAL_VIEW)
KEY_BINDING(frontCamera, "BindFrontCamera", Lang::CAMERA_FRONT_VIEW, SDLK_KP_8, SDLK_UP)
KEY_BINDING(rearCamera, "BindRearCamera", Lang::CAMERA_REAR_VIEW, SDLK_KP_2, SDLK_DOWN)
KEY_BINDING(leftCamera, "BindLeftCamera", Lang::CAMERA_LEFT_VIEW, SDLK_KP_4, SDLK_LEFT)
KEY_BINDING(rightCamera, "BindRightCamera", Lang::CAMERA_RIGHT_VIEW, SDLK_KP_6, SDLK_RIGHT)
KEY_BINDING(topCamera, "BindTopCamera", Lang::CAMERA_TOP_VIEW, SDLK_KP_9, 0)
KEY_BINDING(bottomCamera, "BindBottomCamera", Lang::CAMERA_BOTTOM_VIEW, SDLK_KP_3, 0)

BINDING_GROUP(Lang::EXTERNAL_VIEW)
KEY_BINDING(cameraRollLeft, "BindCameraRollLeft", Lang::ROLL_LEFT, SDLK_KP_1, 0)
KEY_BINDING(cameraRollRight, "BindCameraRollRight", Lang::ROLL_RIGHT, SDLK_KP_3, 0)
KEY_BINDING(cameraRotateDown, "BindCameraRotateDown", Lang::ROTATE_DOWN, SDLK_KP_2, SDLK_DOWN)
KEY_BINDING(cameraRotateUp, "BindCameraRotateUp", Lang::ROTATE_UP, SDLK_KP_8, SDLK_UP)
KEY_BINDING(cameraRotateLeft, "BindCameraRotateLeft", Lang::ROTATE_LEFT, SDLK_KP_4, SDLK_LEFT)
KEY_BINDING(cameraRotateRight, "BindCameraRotateRight", Lang::ROTATE_RIGHT, SDLK_KP_6, SDLK_RIGHT)
KEY_BINDING(resetCamera, "BindResetCamera", Lang::RESET, SDLK_HOME, 0)

BINDING_PAGE_END()

#undef KEY_BINDING
#undef AXIS_BINDING
#undef BINDING_GROUP
#undef BINDING_PAGE
#undef BINDING_PAGE_END
