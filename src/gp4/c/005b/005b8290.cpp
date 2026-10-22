#include <gp4/gp4.h>
#include <gp4/d3d8.h>

#pragma pack(push, 1)
struct Unknown_005b8290 { int32_t Mode00; };
#pragma pack(pop)

// Switches the device's blend state when the requested mode differs from the current one (+0):
// 0 = alpha blending and alpha test off; 1 = SRCALPHA/INVSRCALPHA blending; 2 = the same with the
// diffuse/ambient/specular material sources (0x91/0x93/0x92) set to the material. Modes 1 and 2 enable
// the alpha test (ref 8, GREATEREQUAL) when the byte 0x00b2e5a0 is set; modes 0..2 turn z-buffer,
// z-writes and fog off. Any other mode is only recorded.
// FUNCTION: GP4 0x005b8290
void __fastcall FUN_005b8290(Unknown_005b8290* self, void* /*edx*/, int32_t mode) {
    if (mode == self->Mode00) return;
    switch (mode) {
    case 0:
        GP4_D3D_DEVICE->SetRenderState(0x1b, 0);
        GP4_D3D_DEVICE->SetRenderState(0xf, 0);
        GP4_D3D_DEVICE->SetRenderState(7, 0);
        GP4_D3D_DEVICE->SetRenderState(0xe, 0);
        GP4_D3D_DEVICE->SetRenderState(0x1c, 0);
        break;
    case 1:
        GP4_D3D_DEVICE->SetRenderState(0x1b, 1);
        GP4_D3D_DEVICE->SetRenderState(0x13, 5);
        GP4_D3D_DEVICE->SetRenderState(0x14, 6);
        if (GP4_GLOBAL(uint8_t, 0x00b2e5a0) != 0) {
            GP4_D3D_DEVICE->SetRenderState(0xf, 1);
            GP4_D3D_DEVICE->SetRenderState(0x18, 8);
            GP4_D3D_DEVICE->SetRenderState(0x19, 7);
        }
        GP4_D3D_DEVICE->SetRenderState(7, 0);
        GP4_D3D_DEVICE->SetRenderState(0xe, 0);
        GP4_D3D_DEVICE->SetRenderState(0x1c, 0);
        break;
    case 2:
        GP4_D3D_DEVICE->SetRenderState(0x1b, 1);
        GP4_D3D_DEVICE->SetRenderState(0x13, 5);
        GP4_D3D_DEVICE->SetRenderState(0x14, 6);
        GP4_D3D_DEVICE->SetRenderState(0x91, 0);
        GP4_D3D_DEVICE->SetRenderState(0x93, 0);
        GP4_D3D_DEVICE->SetRenderState(0x92, 0);
        if (GP4_GLOBAL(uint8_t, 0x00b2e5a0) != 0) {
            GP4_D3D_DEVICE->SetRenderState(0xf, 1);
            GP4_D3D_DEVICE->SetRenderState(0x18, 8);
            GP4_D3D_DEVICE->SetRenderState(0x19, 7);
        }
        GP4_D3D_DEVICE->SetRenderState(7, 0);
        GP4_D3D_DEVICE->SetRenderState(0xe, 0);
        GP4_D3D_DEVICE->SetRenderState(0x1c, 0);
        break;
    }
    self->Mode00 = mode;
}
GP4_IMPL(0x005b8290, FUN_005b8290,
         "thiscall(ptr[8]:{0:i32{=0,1,2,3}}, i32{=0,1,2,3}) -> void; globals=layout=d3ddev_00b2e808; gfields={0x00b2e5a0:u8{=0,1}}")
