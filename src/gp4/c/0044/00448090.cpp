#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00448090 {
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
    uint8_t Flag61;
    uint8_t Unknown62[2];
    uint32_t Unknown64;
    uint8_t Unknown68[0x98];
};
#pragma pack(pop)

// Constructor of the "CGP4CarShader[Specialised tri-texture (glossmapped specmap ...)]" shader
// object: runs the base constructor 0x0045eeb0, then stores the vtable 0x005f044c, sets the flag
// bytes +4 and +5 to 3, the constants 0x220/0x352/0x34, the name string, the float global
// 0x0063e1fc to 1.0f, +0x60 to 0, +0x61 to 1 and +0x64 to 0xffffffff; returns this.
// FUNCTION: GP4 0x00448090
Unknown_00448090* __fastcall FUN_00448090(Unknown_00448090* self, void* /*edx*/) {
    gp4::thiscall_<0x0045eeb0, void>(self);
    self->Vtable00 = GP4_ARRAY(const uint32_t, 0x005f044c);
    self->Flag04 = 3;
    self->Flag05 = 3;
    self->Unknown08 = 0x220;
    self->Unknown0c = 0x352;
    self->Name14 = GP4_ARRAY(const char, 0x0063e200);
    self->Unknown10 = 0x34;
    GP4_GLOBAL(uint32_t, 0x0063e1fc) = 0x3f800000u;
    self->Flag60 = 0;
    self->Flag61 = 1;
    self->Unknown64 = 0xffffffffu;
    return self;
}
GP4_IMPL(0x00448090, FUN_00448090, "thiscall(ptr[0x100]:bytes) -> i32")
