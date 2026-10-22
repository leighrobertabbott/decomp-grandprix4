#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0046a610 {
    const void* Vtable00;
    uint8_t Flag04;
    uint8_t Flag05;
    uint8_t Unknown06[2];
    uint32_t Unknown08;
    uint32_t Unknown0c;
    uint32_t Unknown10;
    const char* Name14;
    uint8_t Unknown18[0x40];
    uint32_t Unknown58;
    uint32_t Unknown5c;
    uint8_t Unknown60;
    uint8_t Flag61;
    uint8_t Flag62;
    uint8_t Unknown63[5];
    uint32_t Unknown68;
    uint8_t Unknown6c[0x94];
};
#pragma pack(pop)

// Constructor of the "CGP4WorldShader[Shaded single texture]" shader object: runs the base constructor
// 0x0045eeb0, then sets the flag bytes at +4/+5/+0x61/+0x62 and the dword +0x5c to 1, stores the vtable
// 0x005f0740, the constants 0x220/0x152/0x24, the name string, and clears +0x58 and +0x68; returns this.
// FUNCTION: GP4 0x0046a610
Unknown_0046a610* __fastcall FUN_0046a610(Unknown_0046a610* self, void* /*edx*/) {
    gp4::thiscall_<0x0045eeb0, void>(self);
    self->Flag04 = 1;
    self->Flag05 = 1;
    self->Unknown5c = 1;
    self->Flag61 = 1;
    self->Flag62 = 1;
    self->Vtable00 = GP4_ARRAY(const uint32_t, 0x005f0740);
    self->Unknown08 = 0x220;
    self->Unknown0c = 0x152;
    self->Name14 = GP4_ARRAY(const char, 0x0063f718);
    self->Unknown10 = 0x24;
    self->Unknown58 = 0;
    self->Unknown68 = 0;
    return self;
}
GP4_IMPL(0x0046a610, FUN_0046a610, "thiscall(ptr[0x100]:bytes) -> i32")
