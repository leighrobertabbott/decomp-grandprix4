#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0045fa70 {
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
    uint32_t Unknown64;
    uint8_t Unknown68[0x98];
};
#pragma pack(pop)

// Constructor of the "CGP4ShadowShader[Shadow (single texture, unlit, no colour)]" shader object: runs
// the base constructor 0x0045eeb0, then stores the vtable 0x005f0644, sets both flag bytes to 1, the
// constants 0x220/0x102/0x14, the name string, clears +0x60 and stores 1.0f (0x3f800000) at +0x64;
// returns this.
// FUNCTION: GP4 0x0045fa70
Unknown_0045fa70* __fastcall FUN_0045fa70(Unknown_0045fa70* self, void* /*edx*/) {
    gp4::thiscall_<0x0045eeb0, void>(self);
    self->Vtable00 = GP4_ARRAY(const uint32_t, 0x005f0644);
    self->Flag04 = 1;
    self->Flag05 = 1;
    self->Unknown08 = 0x220;
    self->Unknown0c = 0x102;
    self->Name14 = GP4_ARRAY(const char, 0x0063f40c);
    self->Unknown10 = 0x14;
    self->Unknown60 = 0;
    self->Unknown64 = 0x3f800000;
    return self;
}
GP4_IMPL(0x0045fa70, FUN_0045fa70, "thiscall(ptr[0x100]:bytes) -> i32")
