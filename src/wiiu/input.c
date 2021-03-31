#include "wiiu.h"

#include <vpad/input.h>
#include <padscore/kpad.h>
#include <padscore/wpad.h>

int disable_gamepad = 0;

void wiiu_input_init(void)
{
	KPADInit();
	WPADEnableURCC(1);
}

void wiiu_input_update(void) {
  short controllerNumber = 0;
  short gamepad_mask = 0;

  for (int i = 0; i < wiiu_input_num_controllers(); i++)
    gamepad_mask |= 1 << i;

  VPADStatus vpad;
  VPADReadError err;
  VPADRead(VPAD_CHAN_0, &vpad, 1, &err);
  if (err == VPAD_READ_SUCCESS && !disable_gamepad) {
    uint32_t btns = vpad.hold;
    short buttonFlags = 0;
#define CHECKBTN(v, f) if (btns & v) buttonFlags |= f;
    CHECKBTN(VPAD_BUTTON_A,       A_FLAG);
    CHECKBTN(VPAD_BUTTON_B,       B_FLAG);
    CHECKBTN(VPAD_BUTTON_X,       X_FLAG);
    CHECKBTN(VPAD_BUTTON_Y,       Y_FLAG);
    CHECKBTN(VPAD_BUTTON_UP,      UP_FLAG);
    CHECKBTN(VPAD_BUTTON_DOWN,    DOWN_FLAG);
    CHECKBTN(VPAD_BUTTON_LEFT,    LEFT_FLAG);
    CHECKBTN(VPAD_BUTTON_RIGHT,   RIGHT_FLAG);
    CHECKBTN(VPAD_BUTTON_L,       LB_FLAG);
    CHECKBTN(VPAD_BUTTON_R,       RB_FLAG);
    CHECKBTN(VPAD_BUTTON_STICK_L, LS_CLK_FLAG);
    CHECKBTN(VPAD_BUTTON_STICK_R, RS_CLK_FLAG);
    CHECKBTN(VPAD_BUTTON_PLUS,    PLAY_FLAG);
    CHECKBTN(VPAD_BUTTON_MINUS,   BACK_FLAG);
#undef CHECKBTN

    LiSendMultiControllerEvent(controllerNumber++, gamepad_mask, buttonFlags,
      (vpad.hold & VPAD_BUTTON_ZL) ? 0xFF : 0,
      (vpad.hold & VPAD_BUTTON_ZR) ? 0xFF : 0,
      vpad.leftStick.x * INT16_MAX, vpad.leftStick.y * INT16_MAX,
      vpad.rightStick.x * INT16_MAX, vpad.rightStick.y * INT16_MAX);
  }

  KPADStatus kpad_data = {0};
	int32_t kpad_err = -1;
	for (int i = 0; i < 4; i++) {
		KPADReadEx((KPADChan) i, &kpad_data, 1, &kpad_err);
		if (kpad_err == KPAD_ERROR_OK && controllerNumber < 4) {
      if (kpad_data.extensionType == WPAD_EXT_PRO_CONTROLLER) {
        uint32_t btns = kpad_data.pro.hold;
        short buttonFlags = 0;
#define CHECKBTN(v, f) if (btns & v) buttonFlags |= f;
        CHECKBTN(WPAD_PRO_BUTTON_A,       A_FLAG);
        CHECKBTN(WPAD_PRO_BUTTON_B,       B_FLAG);
        CHECKBTN(WPAD_PRO_BUTTON_X,       X_FLAG);
        CHECKBTN(WPAD_PRO_BUTTON_Y,       Y_FLAG);
        CHECKBTN(WPAD_PRO_BUTTON_UP,      UP_FLAG);
        CHECKBTN(WPAD_PRO_BUTTON_DOWN,    DOWN_FLAG);
        CHECKBTN(WPAD_PRO_BUTTON_LEFT,    LEFT_FLAG);
        CHECKBTN(WPAD_PRO_BUTTON_RIGHT,   RIGHT_FLAG);
        CHECKBTN(WPAD_PRO_TRIGGER_L,      LB_FLAG);
        CHECKBTN(WPAD_PRO_TRIGGER_R,      RB_FLAG);
        CHECKBTN(WPAD_PRO_BUTTON_STICK_L, LS_CLK_FLAG);
        CHECKBTN(WPAD_PRO_BUTTON_STICK_R, RS_CLK_FLAG);
        CHECKBTN(WPAD_PRO_BUTTON_PLUS,    PLAY_FLAG);
        CHECKBTN(WPAD_PRO_BUTTON_MINUS,   BACK_FLAG);
#undef CHECKBTN

        LiSendMultiControllerEvent(controllerNumber++, gamepad_mask, buttonFlags,
          (kpad_data.pro.hold & WPAD_PRO_TRIGGER_ZL) ? 0xFF : 0,
          (kpad_data.pro.hold & WPAD_PRO_TRIGGER_ZR) ? 0xFF : 0,
          kpad_data.pro.leftStick.x * INT16_MAX, kpad_data.pro.leftStick.y * INT16_MAX,
          kpad_data.pro.rightStick.x * INT16_MAX, kpad_data.pro.rightStick.y * INT16_MAX);
      }
      else if (kpad_data.extensionType == WPAD_EXT_CLASSIC || kpad_data.extensionType == WPAD_EXT_MPLUS_CLASSIC) {
        uint32_t btns = kpad_data.classic.hold;
        short buttonFlags = 0;
#define CHECKBTN(v, f) if (btns & v) buttonFlags |= f;
        CHECKBTN(WPAD_CLASSIC_BUTTON_A,       A_FLAG);
        CHECKBTN(WPAD_CLASSIC_BUTTON_B,       B_FLAG);
        CHECKBTN(WPAD_CLASSIC_BUTTON_X,       X_FLAG);
        CHECKBTN(WPAD_CLASSIC_BUTTON_Y,       Y_FLAG);
        CHECKBTN(WPAD_CLASSIC_BUTTON_UP,      UP_FLAG);
        CHECKBTN(WPAD_CLASSIC_BUTTON_DOWN,    DOWN_FLAG);
        CHECKBTN(WPAD_CLASSIC_BUTTON_LEFT,    LEFT_FLAG);
        CHECKBTN(WPAD_CLASSIC_BUTTON_RIGHT,   RIGHT_FLAG);
        CHECKBTN(WPAD_CLASSIC_BUTTON_ZL,      LB_FLAG);
        CHECKBTN(WPAD_CLASSIC_BUTTON_ZR,      RB_FLAG);
        // don't have stick buttons on a classic controller
        // CHECKBTN(WPAD_CLASSIC_BUTTON_STICK_L, LS_CLK_FLAG);
        // CHECKBTN(WPAD_CLASSIC_BUTTON_STICK_R, RS_CLK_FLAG);
        CHECKBTN(WPAD_CLASSIC_BUTTON_PLUS,    PLAY_FLAG);
        CHECKBTN(WPAD_CLASSIC_BUTTON_MINUS,   BACK_FLAG);
#undef CHECKBTN

        LiSendMultiControllerEvent(controllerNumber++, gamepad_mask, buttonFlags,
          kpad_data.classic.leftTrigger * 0xFF,
          kpad_data.classic.rightTrigger * 0xFF,
          kpad_data.classic.leftStick.x * INT16_MAX, kpad_data.classic.leftStick.y * INT16_MAX,
          kpad_data.classic.rightStick.x * INT16_MAX, kpad_data.classic.rightStick.y * INT16_MAX);
      }
    }
  }
}

uint32_t wiiu_input_num_controllers(void)
{
  uint32_t numControllers = !disable_gamepad;

  WPADExtensionType type;
  if(WPADProbe(0, &type) == 0) {
    if (type == WPAD_EXT_PRO_CONTROLLER || type == WPAD_EXT_CLASSIC || type == WPAD_EXT_MPLUS_CLASSIC) {
      numControllers++;
    }
  }

  if (numControllers > 4) {
    numControllers = 4;
  }

  return numControllers;
}

uint32_t wiiu_input_buttons_triggered(void)
{
  uint32_t btns = 0;

  VPADStatus vpad;
  VPADReadError vpad_err;
  VPADRead(VPAD_CHAN_0, &vpad, 1, &vpad_err);
  if (vpad_err == VPAD_READ_SUCCESS) {
    btns |= vpad.trigger;
  }

  KPADStatus kpad_data = {0};
	int32_t kpad_err = -1;
	for (int i = 0; i < 4; i++) {
		KPADReadEx((KPADChan) i, &kpad_data, 1, &kpad_err);
		if (kpad_err == KPAD_ERROR_OK) {
      if (kpad_data.extensionType == WPAD_EXT_PRO_CONTROLLER) {
#define MAPBTNS(b, v) if (kpad_data.pro.trigger & b) btns |= v;
        MAPBTNS(WPAD_PRO_BUTTON_UP,       VPAD_BUTTON_UP);
        MAPBTNS(WPAD_PRO_BUTTON_LEFT,     VPAD_BUTTON_LEFT);
        MAPBTNS(WPAD_PRO_TRIGGER_ZR,      VPAD_BUTTON_ZR);
        MAPBTNS(WPAD_PRO_BUTTON_X,        VPAD_BUTTON_X);
        MAPBTNS(WPAD_PRO_BUTTON_A,        VPAD_BUTTON_A);
        MAPBTNS(WPAD_PRO_BUTTON_Y,        VPAD_BUTTON_Y);
        MAPBTNS(WPAD_PRO_BUTTON_B,        VPAD_BUTTON_B);
        MAPBTNS(WPAD_PRO_TRIGGER_ZL,      VPAD_BUTTON_ZL);
        MAPBTNS(WPAD_PRO_TRIGGER_R,       VPAD_BUTTON_R);
        MAPBTNS(WPAD_PRO_BUTTON_PLUS,     VPAD_BUTTON_PLUS);
        MAPBTNS(WPAD_PRO_BUTTON_HOME,     VPAD_BUTTON_HOME);
        MAPBTNS(WPAD_PRO_BUTTON_MINUS,    VPAD_BUTTON_MINUS);
        MAPBTNS(WPAD_PRO_TRIGGER_L,       VPAD_BUTTON_L);
        MAPBTNS(WPAD_PRO_BUTTON_DOWN,     VPAD_BUTTON_DOWN);
        MAPBTNS(WPAD_PRO_BUTTON_RIGHT,    VPAD_BUTTON_RIGHT);
        MAPBTNS(WPAD_PRO_BUTTON_STICK_R,  VPAD_BUTTON_STICK_R);
        MAPBTNS(WPAD_PRO_BUTTON_STICK_L,  VPAD_BUTTON_STICK_L);
#undef MAPBTNS
      }
      else if (kpad_data.extensionType == WPAD_EXT_CLASSIC || kpad_data.extensionType == WPAD_EXT_MPLUS_CLASSIC) {
#define MAPBTNS(b, v) if (kpad_data.classic.trigger & b) btns |= v;
        MAPBTNS(WPAD_CLASSIC_BUTTON_UP,     VPAD_BUTTON_UP);
        MAPBTNS(WPAD_CLASSIC_BUTTON_LEFT,   VPAD_BUTTON_LEFT);
        MAPBTNS(WPAD_CLASSIC_BUTTON_ZR,     VPAD_BUTTON_ZR);
        MAPBTNS(WPAD_CLASSIC_BUTTON_X,      VPAD_BUTTON_X);
        MAPBTNS(WPAD_CLASSIC_BUTTON_A,      VPAD_BUTTON_A);
        MAPBTNS(WPAD_CLASSIC_BUTTON_Y,      VPAD_BUTTON_Y);
        MAPBTNS(WPAD_CLASSIC_BUTTON_B,      VPAD_BUTTON_B);
        MAPBTNS(WPAD_CLASSIC_BUTTON_ZL,     VPAD_BUTTON_ZL);
        MAPBTNS(WPAD_CLASSIC_BUTTON_R,      VPAD_BUTTON_R);
        MAPBTNS(WPAD_CLASSIC_BUTTON_PLUS,   VPAD_BUTTON_PLUS);
        MAPBTNS(WPAD_CLASSIC_BUTTON_HOME,   VPAD_BUTTON_HOME);
        MAPBTNS(WPAD_CLASSIC_BUTTON_MINUS,  VPAD_BUTTON_MINUS);
        MAPBTNS(WPAD_CLASSIC_BUTTON_L,      VPAD_BUTTON_L);
        MAPBTNS(WPAD_CLASSIC_BUTTON_DOWN,   VPAD_BUTTON_DOWN);
        MAPBTNS(WPAD_CLASSIC_BUTTON_RIGHT,  VPAD_BUTTON_RIGHT);
#undef MAPBTNS
      }
      else {
        // meh we can't really map a wiimote to gamepad
      }
    }
  }

  return btns;
}
