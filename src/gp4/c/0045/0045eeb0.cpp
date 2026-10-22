#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0045eeb0 {
    const void* Vtable00;
    uint8_t Flag04;
    uint8_t Flag05;
    uint8_t Unknown06[2];
    uint32_t Unknown08;
    uint32_t Unknown0c;
    uint32_t Unknown10;
    const char* Name14;
    uint32_t Slots18[16];
    uint32_t Unknown58;
    uint32_t Unknown5c;
    uint8_t Unknown60[0x9c];
};
#pragma pack(pop)

// Constructor of the "CGP4Shader[Shaded, untextured]" shader object: clears the two flag bytes and
// +0x58, stores the vtable 0x005f05f4, the constants 0x220/0x52/0x1c, the name string, sets +0x5c to 1
// and fills the 16 slots at +0x18 with 0xffffffff; returns this.
// FUNCTION: GP4 0x0045eeb0
Unknown_0045eeb0* __fastcall FUN_0045eeb0(Unknown_0045eeb0* self, void* /*edx*/) {
    self->Flag04 = 0;
    self->Flag05 = 0;
    self->Unknown58 = 0;
    self->Vtable00 = GP4_ARRAY(const uint32_t, 0x005f05f4);
    self->Unknown08 = 0x220;
    self->Unknown0c = 0x52;
    self->Name14 = GP4_ARRAY(const char, 0x0063f328);
    self->Unknown10 = 0x1c;
    self->Unknown5c = 1;
    for (int i = 0; i < 16; i++) {
        self->Slots18[i] = 0xffffffffu;
    }
    return self;
}
GP4_IMPL(0x0045eeb0, FUN_0045eeb0, "thiscall(ptr[0x100]:bytes) -> i32")
