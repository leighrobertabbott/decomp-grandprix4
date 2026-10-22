#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00463ae0 {
    const void* Vtable00;
    uint8_t Flag04;
    uint8_t Flag05;
    uint8_t Unknown06[2];
    uint32_t Unknown08;
    uint32_t Unknown0c;
    uint32_t Unknown10;
    const char* Name14;
    uint8_t Unknown18[0x48];
    uint32_t Unknown60;
    uint8_t Unknown64[0x9c];
};
#pragma pack(pop)

// Constructor of the "CGP4TrackShader[Shaded single texture with blend]" shader object: runs the
// base constructor 0x0045eeb0, then stores the vtable 0x005f06c0, sets the flag bytes +4 and +5 to
// 1, the constants 0x220/0x152/0x24, the name string and clears +0x60; returns this.
// FUNCTION: GP4 0x00463ae0
Unknown_00463ae0* __fastcall FUN_00463ae0(Unknown_00463ae0* self, void* /*edx*/) {
    gp4::thiscall_<0x0045eeb0, void>(self);
    self->Vtable00 = GP4_ARRAY(const uint32_t, 0x005f06c0);
    self->Flag04 = 1;
    self->Flag05 = 1;
    self->Unknown08 = 0x220;
    self->Unknown0c = 0x152;
    self->Name14 = GP4_ARRAY(const char, 0x0063f5dc);
    self->Unknown10 = 0x24;
    self->Unknown60 = 0;
    return self;
}
GP4_IMPL(0x00463ae0, FUN_00463ae0, "thiscall(ptr[0x100]:bytes) -> i32")
