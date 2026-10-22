#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00460100 {
    const void* Vtable00;
    uint8_t Flag04;
    uint8_t Flag05;
    uint8_t Unknown06[2];
    uint32_t Unknown08;
    uint32_t Unknown0c;
    uint32_t Unknown10;
    const char* Name14;
    uint8_t Unknown18[0x48];
    uint8_t Flag60;
    uint8_t Unknown61[0x9f];
};
#pragma pack(pop)

// Constructor of the "CGP4SingleTextureShader[Shaded single texture]" shader object: runs the base
// constructor 0x0045eeb0, then stores the vtable 0x005f0664, sets the flag bytes +4, +5 and +0x60 to
// 1, the constants 0x220/0x152/0x24 and the name string; returns this.
// FUNCTION: GP4 0x00460100
Unknown_00460100* __fastcall FUN_00460100(Unknown_00460100* self, void* /*edx*/) {
    gp4::thiscall_<0x0045eeb0, void>(self);
    self->Vtable00 = GP4_ARRAY(const uint32_t, 0x005f0664);
    self->Flag04 = 1;
    self->Flag05 = 1;
    self->Flag60 = 1;
    self->Unknown08 = 0x220;
    self->Unknown0c = 0x152;
    self->Name14 = GP4_ARRAY(const char, 0x0063f448);
    self->Unknown10 = 0x24;
    return self;
}
GP4_IMPL(0x00460100, FUN_00460100, "thiscall(ptr[0x100]:bytes) -> i32")
