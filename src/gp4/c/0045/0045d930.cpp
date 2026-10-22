#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0045d930 {
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
    uint8_t Unknown61[3];
    uint32_t Unknown64;
    uint8_t Unknown68[0x98];
};
#pragma pack(pop)

// Constructor of the "CGP4ReflectionShader[Reflection Shader]" shader object: runs the base
// constructor 0x0045eeb0, then stores the vtable 0x005f05d8, sets the flag bytes +4 and +5 to 1,
// clears the byte +0x60, sets +0x64 to 0xff000000 and stores the constants 0x220/0x152/0x24 and the
// name string; returns this.
// FUNCTION: GP4 0x0045d930
Unknown_0045d930* __fastcall FUN_0045d930(Unknown_0045d930* self, void* /*edx*/) {
    gp4::thiscall_<0x0045eeb0, void>(self);
    self->Vtable00 = GP4_ARRAY(const uint32_t, 0x005f05d8);
    self->Flag04 = 1;
    self->Flag05 = 1;
    self->Flag60 = 0;
    self->Unknown64 = 0xff000000u;
    self->Unknown08 = 0x220;
    self->Unknown0c = 0x152;
    self->Name14 = GP4_ARRAY(const char, 0x0063f2fc);
    self->Unknown10 = 0x24;
    return self;
}
GP4_IMPL(0x0045d930, FUN_0045d930, "thiscall(ptr[0x100]:bytes) -> i32")
