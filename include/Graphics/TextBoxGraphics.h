#pragma once

#include "SPRSprite.h"
#include "../Memory/SafeAllocator.h"
#include "../UI/TextBox.h"

// sizeof == 0x448
// Responsible for displaying a text box with the 3D engine. One instance
// can draw multiple text boxes by changing the pointer
class TextBoxGraphics
{
public:
    SPRSprite tiles_[8];
    TextBox* activeBox_;
    SafeAllocator* allocator_;

    void Reset();
    void Destroy();
    void Create();

    // the data pointer isn't used, file is loaded from the active NARC instead
    void LoadFromFile(SafeAllocator* alloc, const void* data);
    void Update(int numTicks);

    void Draw(CBool drawBorder, int unused);
    void Draw(fix32_t x, fix32_t y, fix32_t w, fix32_t h, CBool drawBorder, int unused);

    // Make the text box referenced by activeBox_ visible, Update()
    // makes it fully visible in one frame
    void BeginDisplay();
    // Should cause a collapsing fade out effect, but I don't recognise it
    // anywhere
    void EndDisplay();
};