#include "Graphics/ClipWindow.h"
#include "System/Memory.h"
#include "System/Graphics.h"

#if defined(jpn)
#define data_020f2bdc data_020f2d38
#define data_020f2be4 data_020f2d40
#define data_020f2bec data_020f2d48
#define data_020f2bf4 data_020f2d50
#define data_020f2c04 data_020f2d60
#endif

static void SetSubWindow1Bounds(int left, int top, int right, int bottom)
{
    WIN1XSUB = ((left << 8) & 0xff00) | (right & 0xff);
    WIN1YSUB = ((top << 8) & 0xff00) | (bottom & 0xff);
}

static void SetSubWindow0Bounds(int left, int top, int right, int bottom)
{
    WIN0XSUB = ((left << 8) & 0xff00) | (right & 0xff);
    WIN0YSUB = ((top << 8) & 0xff00) | (bottom & 0xff);
}

static void SetMainWindow1Bounds(int left, int top, int right, int bottom)
{
    WIN1X = ((left << 8) & 0xff00) | (right & 0xff);
    WIN1Y = ((top << 8) & 0xff00) | (bottom & 0xff);
}

static void SetMainWindow0Bounds(int left, int top, int right, int bottom)
{
    WIN0X = ((left << 8) & 0xff00) | (right & 0xff);
    WIN0Y = ((top << 8) & 0xff00) | (bottom & 0xff);
}

static void SetSubWindow1In(int layers, int colorSpecial)
{
    int newRegVal = (WININSUB & ~0x3f00) | (layers << 8);
    if (colorSpecial)
        newRegVal |= 0x2000;
    WININSUB = newRegVal;
}

static void SetSubWindow0In(int layers, int colorSpecial)
{
    int newRegVal = (WININSUB & ~0x3f) | layers;
    if (colorSpecial)
        newRegVal |= 0x20;
    WININSUB = newRegVal;
}

static void SetMainWindow1In(int layers, int colorSpecial)
{
    int newRegVal = (WININ & ~0x3f00) | (layers << 8);
    if (colorSpecial)
        newRegVal |= 0x2000;
    WININ = newRegVal;
}

static void SetMainWindow0In(int layers, int colorSpecial)
{
    int newRegVal = (WININ & ~0x3f) | layers;
    if (colorSpecial)
        newRegVal |= 0x20;
    WININ = newRegVal;
}

static void SetSubWindowOut(int layers, int colorSpecial)
{
    int newRegVal = (WINOUTSUB & ~0x3f) | layers;
    if (colorSpecial)
        newRegVal |= 0x20;
    WINOUTSUB = newRegVal;
}

static void SetMainWindowOut(int layers, int colorSpecial)
{
    int newRegVal = (WINOUT & ~0x3f) | layers;
    if (colorSpecial)
        newRegVal |= 0x20;
    WINOUT = newRegVal;
}

static void SetSubEnabledWindows(int to)
{
    DISPCNTSUB = (DISPCNTSUB & ~0xe000) | (to << 13);
}

static void SetMainEnabledWindows(int to)
{
    DISPCNT = (DISPCNT & ~0xe000) | (to << 13);
}

static int GetSubEnabledWindows()
{
    return (DISPCNTSUB & 0xe000) >> 13;
}

static int GetMainEnabledWindows()
{
    return (DISPCNT & 0xe000) >> 13;
}

static int (*s_getEnabledWindows[2])() = {
    &GetMainEnabledWindows, &GetSubEnabledWindows
};

static void (*s_setWindowOut[2])(int, int) = {
    &SetMainWindowOut, &SetSubWindowOut
};

static void (*s_setEnabledWindows[2])(int) = {
    &SetMainEnabledWindows, &SetSubEnabledWindows
};

static void (*s_setWindowBounds[2][2])(int, int, int, int) = {
    { &SetMainWindow0Bounds, &SetMainWindow1Bounds },
    { &SetSubWindow0Bounds, &SetSubWindow1Bounds }
};

static void (*s_setWindowIn[2][2])(int, int) = {
    { &SetMainWindow0In, &SetMainWindow1In },
    { &SetSubWindow0In, &SetSubWindow1In }
};

void ClipWindow::Initialize(int screen, int windowID)
{
    enabled_ = false;
    screen_ = screen;
    windowID_ = windowID;
    VectorizedMemset(&bounds_, 0, sizeof(Bounds));
    innerLayers_ = (1 << 0) | (1 << 2);
    innerColorEffects_ = true;
    outerLayers_ = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4);
    outerColorEffects_ = true;
    SetEnabled(false);
    UpdateLayers();
}

void ClipWindow::Apply()
{
    if (!enabled_)
        return;

    UpdateLayers();
    s_setWindowBounds[screen_][windowID_](bounds_.left, bounds_.top, bounds_.right, bounds_.bottom);
}

void ClipWindow::UpdateLayers()
{
    s_setWindowIn[screen_][windowID_](innerLayers_, innerColorEffects_);
    s_setWindowOut[screen_](outerLayers_, outerColorEffects_);
}

void ClipWindow::SetEnabled(bool to)
{
    if (enabled_ == to)
        return;

    enabled_ = to;
    int windowMask = s_getEnabledWindows[screen_]();
    if (to)
        windowMask |= (1 << windowID_);
    else
        windowMask &= ~(1 << windowID_);
    s_setEnabledWindows[screen_](windowMask);
}

void ClipWindow::ConfigureInterior(int layers, int enableColorEffects)
{
    innerLayers_ = layers;
    innerColorEffects_ = enableColorEffects;
}

void ClipWindow::SetBoundsXYWH(int x, int y, int width, int height)
{
    bounds_.left = x;
    bounds_.top = y;
    bounds_.right = x + width;
    bounds_.bottom = y + height;
}