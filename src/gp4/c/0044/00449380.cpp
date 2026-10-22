#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00449380 {
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

// Constructor of the "CGP4CockpitShader[Special cockpit]" shader object: runs the base constructor
// 0x0045eeb0, then stores the vtable 0x005f04a0, sets the flag bytes +4, +5 and +0x60 to 1, the
// constants 0x220/0x152/0x24 and the name string; returns this.
// FUNCTION: GP4 0x00449380
Unknown_00449380* __fastcall FUN_00449380(Unknown_00449380* self, void* /*edx*/) {
    gp4::thiscall_<0x0045eeb0, void>(self);
    self->Vtable00 = GP4_ARRAY(const uint32_t, 0x005f04a0);
    self->Flag04 = 1;
    self->Flag05 = 1;
    self->Flag60 = 1;
    self->Unknown08 = 0x220;
    self->Unknown0c = 0x152;
    self->Name14 = GP4_ARRAY(const char, 0x0063e2f8);
    self->Unknown10 = 0x24;
    return self;
}
GP4_IMPL(0x00449380, FUN_00449380, "thiscall(ptr[0x100]:bytes) -> i32")
