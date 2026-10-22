#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00468e60 {
    const void* Vtable00;
    uint8_t Flag04;
    uint8_t Flag05;
    uint8_t Unknown06[2];
    uint32_t Unknown08;
    uint32_t Unknown0c;
    uint32_t Unknown10;
    const char* Name14;
    uint8_t Unknown18[0x44];
    uint32_t Unknown5c;
    uint8_t Unknown60[0xa0];
};
#pragma pack(pop)

// Constructor of the "CGP4VisorShader[Tex gen shifted specular]" shader object: runs the base
// constructor 0x0045eeb0, then stores the vtable 0x005f0710, sets the flag bytes +4 and +5 to 1, the
// constants 0x220/0x112/0x20, the name string and +0x5c to 2; returns this.
// FUNCTION: GP4 0x00468e60
Unknown_00468e60* __fastcall FUN_00468e60(Unknown_00468e60* self, void* /*edx*/) {
    gp4::thiscall_<0x0045eeb0, void>(self);
    self->Vtable00 = GP4_ARRAY(const uint32_t, 0x005f0710);
    self->Flag04 = 1;
    self->Flag05 = 1;
    self->Unknown08 = 0x220;
    self->Unknown0c = 0x112;
    self->Name14 = GP4_ARRAY(const char, 0x0063f6c4);
    self->Unknown10 = 0x20;
    self->Unknown5c = 2;
    return self;
}
GP4_IMPL(0x00468e60, FUN_00468e60, "thiscall(ptr[0x100]:bytes) -> i32")
