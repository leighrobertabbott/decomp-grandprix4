#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00446a80 {
    const void* Vtable00;
    uint8_t Flag04;
    uint8_t Flag05;
    uint8_t Unknown06[2];
    uint32_t Unknown08;
    uint32_t Unknown0c;
    uint32_t Unknown10;
    const char* Name14;
    uint8_t Unknown18[0x5c - 0x18];
    uint32_t Unknown5c;
    uint8_t Flag60;
    uint8_t Flag61;
    uint8_t Unknown62[2];
    int32_t Unknown64;
    int32_t Unknown68;
    uint8_t Unknown6c[0x94];
};
#pragma pack(pop)

// Constructor of the "CGP4AdvancedCarShader[Specialised tri-texture (glossmapped ...)]" shader
// object: runs the base constructor 0x0045eeb0, then stores the vtable 0x005f0430, the two flag
// bytes 3, the constants 0x220/0x352/0x34, the name string, +0x5c = 1, the float 1.0 in the global
// at 0x0063e198, flag bytes +0x61 = 1 and +0x60 = 0 and the two -1 words at +0x64/+0x68.
// Returns this.
// FUNCTION: GP4 0x00446a80
Unknown_00446a80* __fastcall FUN_00446a80(Unknown_00446a80* self, void* /*edx*/) {
    gp4::thiscall_<0x0045eeb0, int>(self);
    self->Vtable00 = GP4_ARRAY(const uint32_t, 0x005f0430);
    self->Flag04 = 3;
    self->Flag05 = 3;
    self->Unknown08 = 0x220;
    self->Unknown0c = 0x352;
    self->Name14 = GP4_ARRAY(const char, 0x0063e1a0);
    self->Unknown10 = 0x34;
    self->Unknown5c = 1;
    GP4_GLOBAL(uint32_t, 0x0063e198) = 0x3f800000;
    self->Flag61 = 1;
    self->Flag60 = 0;
    self->Unknown68 = -1;
    self->Unknown64 = -1;
    return self;
}
GP4_IMPL(0x00446a80, FUN_00446a80, "thiscall(ptr[0x100]:bytes) -> i32")
