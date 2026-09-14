#pragma once

#include "std_library_functions.h"

// sizeof == 0x24 == 36.
// Uses the NDS's windowing feature to alter visibility of layers within
// a rectangular region.
class ClipWindow
{
public:
    bool enabled_;
    unsigned char screen_; // 0 = main, 1 = sub
    unsigned char windowID_; // 0 or 1
    char pad_3[1];
    int innerLayers_;
    int innerColorEffects_; // not sure what these are? this is a bool
    int outerLayers_;
    int outerColorEffects_; // not sure what these are? this is a bool
    struct Bounds
    {
        int left;
        int top;
        int right;
        int bottom;
    } bounds_;

public:
    void Initialize(int screen, int windowID);
    void Apply();
    void UpdateLayers();

    void SetEnabled(bool);
    void ConfigureInterior(int layers, int enableColorEffects);
    void SetBoundsXYWH(int x, int y, int width, int height);
};