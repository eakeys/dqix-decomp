#include "Graphics/NSBXX/RenderConfig.h"
#include "Graphics/GeometryFifo.h"
#include <globaldefs.h>

#pragma optimize_for_size off

extern "C"
{
    // write 3x3 identity
    void func_020c1180(fix32_t*);
    // write 3x4 identity
    void func_020c1840(fix32_t*);
    // write 4x4 identity
    void func_020c21dc(fix32_t*);

    // invert 3x4 matrix (returns determinant?)
    fix32_t func_020c19d4(const fix32_t* in, fix32_t* out);
    
    // sets out = diag(x, y, z) * mat
    void func_020c1948(const fix32_t* mat, fix32_t* out, fix32_t x, fix32_t y, fix32_t z);
    
    // multiply 3x4 matrices, store a * b in out
    void func_020c1d60(const fix32_t* a, const fix32_t* b, fix32_t* out);
}

#define BUILD_LIGHT_VECTOR(idx, x, y, z) (((unsigned)(idx) << 30) | ((unsigned)(z) << 20) | ((unsigned)(y) << 10) | ((unsigned)(x)))
#define RGB(r,g,b) (((unsigned)(r)) | ((unsigned)(g) << 5) | ((unsigned)(b) << 10))
#define BUILD_LIGHT_COLOR(idx, r,g,b) (((unsigned)(idx) << 30) | RGB((r),(g),(b)))

static inline void ArrayCopyVec3(void* dst, const void* src)
{
    struct ArrayVec3 { fix32_t values[3]; };
    *(ArrayVec3*)dst = *(const ArrayVec3*)src;
}

void RenderConfig::Reset()
{
    data_0210a010.magicCommands_0 = COMBINE_GXFIFO_COMMANDS4(
        GXFifoCommand_SetMatrixMode,
        GXFifoCommand_LoadMat4x4,
        GXFifoCommand_SetMatrixMode,
        GXFifoCommand_LoadMat4x3
    );

    data_0210a010.magicParam_4 = 0;
    data_0210a010.magicParam_48 = 2;

    data_0210a010.magicCommands_7c = COMBINE_GXFIFO_COMMANDS4(
        GXFifoCommand_SetLightVector,
        GXFifoCommand_SetLightVector,
        GXFifoCommand_SetLightVector,
        GXFifoCommand_SetLightVector
    );

    data_0210a010.magicCommands_90 = COMBINE_GXFIFO_COMMANDS4(
        GXFifoCommand_DiffuseAmbientReflect,
        GXFifoCommand_SpecularReflectEmit,
        GXFifoCommand_SetPolygonAttr,
        GXFifoCommand_SetViewport
    );

    data_0210a010.magicCommands_a4 = COMBINE_GXFIFO_COMMANDS4(
        GXFifoCommand_SetLightColor,
        GXFifoCommand_SetLightColor,
        GXFifoCommand_SetLightColor,
        GXFifoCommand_SetLightColor
    );

    data_0210a010.magicCommands_b8 = COMBINE_GXFIFO_COMMANDS3(
        GXFifoCommand_MultiplyMat4x3,
        GXFifoCommand_ScaleMatrix,
        GXFifoCommand_SetTexImageParams
    );

    func_020c1840(&data_0210a010.viewMatrix[0]);
    func_020c21dc(&data_0210a010.projectionMatrix[0]);

    uint32_t minusInvSqrt3 = 0x2d8;
    data_0210a010.lightVectors[0] = BUILD_LIGHT_VECTOR(0,
        minusInvSqrt3, minusInvSqrt3, minusInvSqrt3);
    data_0210a010.lightVectors[1] = BUILD_LIGHT_VECTOR(1, 0x200, 0, 0);
    data_0210a010.lightVectors[2] = BUILD_LIGHT_VECTOR(2, 0x1ff, 0, 0);
    data_0210a010.lightVectors[3] = BUILD_LIGHT_VECTOR(3, 0, 0x200, 0);

    data_0210a010.diffuseAmbientArg = (RGB(16, 16, 16) << 16) | (1 << 15) | RGB(16, 16, 16);
    data_0210a010.specularArg = (RGB(16, 16, 16) << 16) | (1 << 15) | RGB(16, 16, 16);
    // alpha = 31 | render forward-facing polygons | enable all lights
    data_0210a010.polygonAttrArg = (0x1f << 16) | (1 << 7) | 0xf;
    // left = 0 | top = 0 | right = 255 | bottom = 191
    data_0210a010.viewportArg = 0 | (0 << 8) | (255 << 16) | (191 << 24);
    data_0210a010.lightColors[0] = BUILD_LIGHT_COLOR(0, 31, 31, 31);
    data_0210a010.lightColors[1] = BUILD_LIGHT_COLOR(1, 31, 0, 0);
    data_0210a010.lightColors[2] = BUILD_LIGHT_COLOR(2, 0, 31, 0);
    data_0210a010.lightColors[3] = BUILD_LIGHT_COLOR(3, 0, 0, 31);

    data_0210a010.objectPosition[0] = 0;
    data_0210a010.objectPosition[1] = 0;
    data_0210a010.objectPosition[2] = 0;

    func_020c1180(&data_0210a010.objectRotation[0]);

    data_0210a010.objectScale[0] = 1 << 12;
    data_0210a010.objectScale[1] = 1 << 12;
    data_0210a010.objectScale[2] = 1 << 12;

    data_0210a010.texImageParams = 0;
    data_0210a010.flags = 0;

    data_0210a010.eyeVector.x = data_0210a010.eyeVector.y = data_0210a010.eyeVector.z = 0;
    data_0210a010.upVector.x = data_0210a010.upVector.z = 0;
    data_0210a010.upVector.y = 1 << 12;

    data_0210a010.targetVector.x = data_0210a010.targetVector.y = 0;
    data_0210a010.targetVector.z = -1 << 12;
}

void RenderConfig::SubmitToFifo()
{
    RenderConfig* instance = &data_0210a010;
    uint32_t* commands = (uint32_t*)0x0210a010; // i hate this
    SubmitCommandToGeometryFifo(instance->magicCommands_0, commands + 1, 0x3e);

    instance->flags &= ~(1 << RENDER_CONFIG_FLAG_0);
    instance->flags &= ~(1 << RENDER_CONFIG_FLAG_1);
}

void RenderConfig::SetObjectPosition(Vector3fix *pos)
{
    if (pos != NULL)
    {
        ArrayCopyVec3(&data_0210a010.objectPosition[0], pos);
        data_0210a010.flags &= ~((1 << RENDER_CONFIG_FLAG_WORLDVIEW_CACHE_VALID) | (1 << RENDER_CONFIG_FLAG_5) | (1 << RENDER_CONFIG_FLAG_2));
    }
}

void RenderConfig::SetObjectScale(Vector3fix *scale)
{
    if (scale != NULL)
    {
        ArrayCopyVec3(&data_0210a010.objectScale[0], scale);
        data_0210a010.flags &= ~((1 << RENDER_CONFIG_FLAG_WORLDVIEW_CACHE_VALID) | (1 << RENDER_CONFIG_FLAG_5) | (1 << RENDER_CONFIG_FLAG_2));
    }
}

void RenderConfig::SetLightVector(unsigned int light, fix32_t x, fix32_t y, fix32_t z)
{
    uint32_t formattedX = (x >> 3) & 0x3ff;
    uint32_t formattedY = ((uint32_t)(y >> 3) << 22) >> 12;
    uint32_t formattedZ = ((uint32_t)(z >> 3) << 22) >> 2;
    data_0210a010.lightVectors[light] = formattedX | formattedY | formattedZ | (light << 30);
}

void RenderConfig::SetLightColor(unsigned int light, uint32_t rgb)
{
    data_0210a010.lightColors[light] = rgb | (light << 30);
}

void RenderConfig::SetDiffuseAmbientColors(uint32_t diffuseRGB, uint32_t ambientRGB, int setVertexColor)
{
    setVertexColor = (setVertexColor ? 1 : 0);
    data_0210a010.diffuseAmbientArg = diffuseRGB | (ambientRGB << 16) | (setVertexColor << 15);
}

void RenderConfig::SetPolygonAttributes(unsigned int lights, unsigned int mode,
    unsigned int culling, unsigned int polygonID, unsigned int alpha, unsigned int miscFlags)
{
    data_0210a010.polygonAttrArg = miscFlags | 
        (lights | mode << 4 | culling << 6) | polygonID << 24 | alpha << 16;
}

const fix32_t *RenderConfig::GetInverseViewMatrix()
{
    if (!(data_0210a010.flags & (1 << RENDER_CONFIG_FLAG_INVERSE_VIEW_CACHE_VALID)))
    {
        func_020c19d4(&data_0210a010.viewMatrix[0], &data_0210a010.cachedInverseView[0]);
        data_0210a010.flags |= (1 << RENDER_CONFIG_FLAG_INVERSE_VIEW_CACHE_VALID);
    }
    return &data_0210a010.cachedInverseView[0];
}

// can be made static later
void RecalculateWorldViewAndInverse()
{
    func_020c1d60(&data_0210a010.objectRotation[0], &data_0210a010.viewMatrix[0], &data_0210a010.cachedWorldView[0]);
    func_020c1948(&data_0210a010.cachedWorldView[0], &data_0210a010.cachedWorldView[0],
        data_0210a010.objectScale[0], data_0210a010.objectScale[1], data_0210a010.objectScale[2]);
    func_020c19d4(&data_0210a010.cachedWorldView[0], &data_0210a010.cachedInverseWorldView[0]);
}

const fix32_t *RenderConfig::GetCombinedWorldViewMatrix()
{
    if (!(data_0210a010.flags & (1 << RENDER_CONFIG_FLAG_WORLDVIEW_CACHE_VALID)))
    {
        RecalculateWorldViewAndInverse();
        data_0210a010.flags |= (1 << RENDER_CONFIG_FLAG_WORLDVIEW_CACHE_VALID);
    }
    return &data_0210a010.cachedWorldView[0];
}

const fix32_t *RenderConfig::GetInverseCombinedWorldViewMatrix()
{
    if (!(data_0210a010.flags & (1 << RENDER_CONFIG_FLAG_WORLDVIEW_CACHE_VALID)))
    {
        RecalculateWorldViewAndInverse();
        data_0210a010.flags |= (1 << RENDER_CONFIG_FLAG_WORLDVIEW_CACHE_VALID);
    }
    return &data_0210a010.cachedInverseWorldView[0];
}

void RenderConfig::GetViewport(int *left, int *top, int *right, int *bottom)
{
    if (left != NULL)
        *left = data_0210a010.viewportArg & 0xff;
    if (top != NULL)
        *top = (data_0210a010.viewportArg >> 8) & 0xff;
    if (right != NULL)
        *right = (data_0210a010.viewportArg >> 16) & 0xff;
    if (bottom != NULL)
        *bottom = (data_0210a010.viewportArg >> 24) & 0xff;
}