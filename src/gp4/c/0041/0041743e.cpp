#include <gp4/gp4.h>

// FUNCTION: GP4 0x0041743e
// Runs 0x0041724d on the third and fourth 0x300-byte records at 0x006380d8 (ebx = record, ecx = its index 2
// and 3), then 0x004171f9 (which reads esi and eax), and leaves ebx = 0x006380d8.
void __cdecl FUN_0041743e(gp4::Regs* r) {
    r->ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x006386d8)));
    r->ecx = 2;
    gp4::call_regs(0x0041724d, *r);

    r->ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x006389d8)));
    r->ecx = 3;
    gp4::call_regs(0x0041724d, *r);

    gp4::call_regs(0x004171f9, *r);

    r->ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x006380d8)));
}
GP4_IMPL(0x0041743e, FUN_0041743e, "regs(eax:u32, esi:ptr[0x300]:bytes) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=tyre_00601510+bits_004171f9")
