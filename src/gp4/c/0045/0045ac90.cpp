#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0045ac90 {
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

// Constructor of the "CGP4NullShader[]" shader object: runs the base constructor 0x0045eeb0, then
// stores the vtable 0x005f05a4, clears both flag bytes, sets the constants 0x220/2/0xc and the
// name string; returns this.
// FUNCTION: GP4 0x0045ac90
Unknown_0045ac90* __fastcall FUN_0045ac90(Unknown_0045ac90* self, void* /*edx*/) {
    gp4::thiscall_<0x0045eeb0, void>(self);
    self->Vtable00 = GP4_ARRAY(const uint32_t, 0x005f05a4);
    self->Flag04 = 0;
    self->Flag05 = 0;
    self->Unknown08 = 0x220;
    self->Unknown0c = 2;
    self->Name14 = GP4_ARRAY(const char, 0x0063f2d4);
    self->Unknown10 = 0xc;
    return self;
}
GP4_IMPL(0x0045ac90, FUN_0045ac90, "thiscall(ptr[0x100]:bytes) -> i32")
