#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00459730 {
    uint32_t Field00;
    uint32_t Field04;
    uint32_t Field08;
    uint32_t Field0c;
    uint32_t Field10;
    uint32_t Field14;
    uint32_t Field18;
    uint32_t Field1c;
};
#pragma pack(pop)

// Clears the eight dwords at +0..+0x1c and returns the object.
// FUNCTION: GP4 0x00459730
Unknown_00459730* __fastcall FUN_00459730(Unknown_00459730* self, void* /*edx*/) {
    self->Field08 = 0;
    self->Field04 = 0;
    self->Field00 = 0;
    self->Field14 = 0;
    self->Field10 = 0;
    self->Field0c = 0;
    self->Field1c = 0;
    self->Field18 = 0;
    return self;
}
GP4_IMPL(0x00459730, FUN_00459730, "thiscall(ptr[0x20]:bytes) -> u32")
