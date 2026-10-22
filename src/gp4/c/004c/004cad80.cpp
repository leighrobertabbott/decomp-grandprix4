#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004cad80 { uint32_t Vptr00; uint32_t Field04; uint32_t Field08; };
#pragma pack(pop)

// Constructor: stores the vtable pointer 0x005f0d3c, clears +4 and +8, returns the object.
// FUNCTION: GP4 0x004cad80
int __fastcall FUN_004cad80(Unknown_004cad80* self, void* /*edx*/) {
    self->Vptr00 = 0x005f0d3c;
    self->Field08 = 0;
    self->Field04 = 0;
    return (int)self;
}
GP4_IMPL(0x004cad80, FUN_004cad80, "thiscall(ptr[0x10]:bytes) -> i32")
