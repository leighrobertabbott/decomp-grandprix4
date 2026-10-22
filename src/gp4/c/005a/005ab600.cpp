#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005ab600 { uint8_t Unknown00[0x10]; int32_t Value10; int32_t Value14; uint8_t Unknown18[0xe8]; };
#pragma pack(pop)

// Sets the dword at +0x14 to 0 and the dword at +0x10 to 2 (ecx = object); eax is left as the
// caller passed it (callers store its low byte afterwards).
// FUNCTION: GP4 0x005ab600
void __cdecl FUN_005ab600(gp4::Regs* r) {
    Unknown_005ab600* self = (Unknown_005ab600*)r->ecx;
    self->Value14 = 0;
    self->Value10 = 2;
}
GP4_IMPL(0x005ab600, FUN_005ab600, "regs(ecx:ptr[0x100]:bytes, eax:u32) -> (eax)")
