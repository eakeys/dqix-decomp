#include "Graphics/NSBXX/NSBXX.h"
#include <globaldefs.h>

int NSBXX_Tex_GetBlock1Length(NSBXXTex* tex)
{
    return (tex != NULL) ? tex->block1LengthShr3_ << 3 : 0;
}

int NSBXX_Tex_GetBlock2Length(NSBXXTex* tex)
{
    return (tex != NULL) ? tex->block2LengthShr3_ << 3 : 0;
}

int NSBXX_Tex_GetBlock4Length(NSBXXTex* tex)
{
    return (tex != NULL) ? tex->block4LengthShr3_ << 3 : 0;
}