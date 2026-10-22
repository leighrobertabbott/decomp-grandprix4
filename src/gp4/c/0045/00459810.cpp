#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00459810 { uint32_t Vtable00; uint32_t Unknown04; uint32_t Unknown08; uint8_t Unknown0c[0xf4]; };
#pragma pack(pop)

// Constructor-like routine: stores the vtable pointer 0x005f0580 at +0, clears
// +4 and +8 and returns this.
// FUNCTION: GP4 0x00459810
Unknown_00459810* __fastcall FUN_00459810(Unknown_00459810* self, void* /*edx*/) {
    self->Vtable00 = 0x005f0580;
    self->Unknown04 = 0;
    self->Unknown08 = 0;
    return self;
}
GP4_IMPL(0x00459810, FUN_00459810, "thiscall(ptr[0x100]:bytes) -> i32")
