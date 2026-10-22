#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005a7360 { uint32_t Vtable00; uint8_t Unknown04[0xfc]; };
#pragma pack(pop)

// Base-class constructor stub: stores the vtable pointer 0x005f9a04 at +0 and
// returns this (`mov eax, ecx; mov [eax], 0x5f9a04; ret`).
// FUNCTION: GP4 0x005a7360
Unknown_005a7360* __fastcall FUN_005a7360(Unknown_005a7360* self, void* /*edx*/) {
    self->Vtable00 = 0x005f9a04;
    return self;
}
GP4_IMPL(0x005a7360, FUN_005a7360, "thiscall(ptr[0x100]:bytes) -> i32")
