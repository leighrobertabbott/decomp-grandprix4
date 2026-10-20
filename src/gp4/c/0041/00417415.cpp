#include <gp4/gp4.h>

// FUNCTION: GP4 0x00417415
// Runs 0x0041724d on the first two 0x300-byte records at 0x006380d8 (ebx = record, ecx = its index 0 and
// 1), then 0x004171b6 (which reads esi and eax), and leaves ebx = 0x006380d8.
void __cdecl FUN_00417415(gp4::Regs* r) {
    r->ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x006380d8)));
    r->ecx = 0;
    gp4::call_regs(0x0041724d, *r);

    r->ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x006383d8)));
    r->ecx = 1;
    gp4::call_regs(0x0041724d, *r);

    gp4::call_regs(0x004171b6, *r);

    r->ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x006380d8)));
}
GP4_IMPL(0x00417415, FUN_00417415, "regs(eax:u32, esi:ptr[0xed]:bytes) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=tyre_00601510+bits_004171b6+tyre_0041724d+state_00417415+gain_0062f880")
