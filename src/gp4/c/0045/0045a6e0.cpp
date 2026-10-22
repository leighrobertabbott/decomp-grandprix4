#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0045a6e0 {
    const void* Vtable00;
    uint8_t Flag04;
    uint8_t Flag05;
    uint8_t Unknown06[2];
    uint32_t Unknown08;
    uint32_t Unknown0c;
    uint32_t Unknown10;
    const char* Name14;
    uint8_t Unknown18[0xe8];
};
#pragma pack(pop)

// Constructor of the "CGP4MultiTextureShader[Shaded multi texture]" shader object: runs the base
// constructor 0x0045eeb0, then stores the vtable 0x005f058c, sets both flag bytes to 2, the constants
// 0x220/0x252/0x2c and the name string; returns this.
// FUNCTION: GP4 0x0045a6e0
Unknown_0045a6e0* __fastcall FUN_0045a6e0(Unknown_0045a6e0* self, void* /*edx*/) {
    gp4::thiscall_<0x0045eeb0, void>(self);
    self->Vtable00 = GP4_ARRAY(const uint32_t, 0x005f058c);
    self->Flag04 = 2;
    self->Flag05 = 2;
    self->Unknown08 = 0x220;
    self->Unknown0c = 0x252;
    self->Name14 = GP4_ARRAY(const char, 0x0063f2a4);
    self->Unknown10 = 0x2c;
    return self;
}
GP4_IMPL(0x0045a6e0, FUN_0045a6e0, "thiscall(ptr[0x100]:bytes) -> i32")
