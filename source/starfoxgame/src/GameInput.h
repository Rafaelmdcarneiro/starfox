#ifndef _GAME_INPUT_H_
#define _GAME_INPUT_H_

#include "gs/Input/KeyboardMgr.h"

const VKEY vkeyPause = 'P';
const VKEY vkeyStepFrame = VK_OEM_4; // [{
const VKEY vkeyStepFrameRepeatModifier = VK_OEM_6; // ]}
const VKEY vkeyTimeScaleInc = VK_OEM_PLUS; // +=
const VKEY vkeyTimeScaleDec = VK_OEM_MINUS; // -_
const VKEY vkeyTimeScaleReset = '0';
const VKEY vkeyCamUp = VK_UP;
const VKEY vkeyCamDown = VK_DOWN;
const VKEY vkeyCamLeft = VK_LEFT;
const VKEY vkeyCamRight = VK_RIGHT;
const VKEY vkeyCamZoomIn = VK_PRIOR; // Page up
const VKEY vkeyCamZoomOut = VK_NEXT; // Page down
const VKEY vkeyCamReset = VK_HOME;
const VKEY vkeyShipLeft = 'A';
const VKEY vkeyShipRight = 'D';
const VKEY vkeyShipUp = 'W';
const VKEY vkeyShipDown = 'S';

#endif // _GAME_INPUT_H_
