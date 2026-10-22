#include <gp4/gp4.h>

struct Unknown_0046bba0 { uint32_t Vtable00; uint8_t Unknown04[0xfc]; };

// Base-class constructor stub: stores the vtable pointer 0x005f0758 at +0 and
// returns this (`mov eax, ecx; mov [eax], 0x5f0758; ret`).
// FUNCTION: GP4 0x0046bba0
Unknown_0046bba0* __fastcall FUN_0046bba0(Unknown_0046bba0* self, void* /*edx*/) {
    self->Vtable00 = 0x005f0758;
    return self;
}
GP4_IMPL(0x0046bba0, FUN_0046bba0, "thiscall(ptr[0x100]:bytes) -> i32")
