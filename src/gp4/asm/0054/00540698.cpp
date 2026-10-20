#include <gp4/gp4.h>
#include <gp4/flags.h>

// Releases a driver slot key: al is compared with each of the 22 key bytes at 0x007c2a1c; every
// match resets that key to 0xff and clears bits 6 and 7 of the matching driver flag byte at
// 0x007c2a48. eax, ecx, esi and edi are saved and restored; the flags are those of the final
// `dec ecx` (1 -> 0) with CF kept from the last iteration (al < the last key byte, or 0 after a
// match since `and` clears it).
// FUNCTION: GP4 0x00540698
void __cdecl FUN_00540698(gp4::Regs* r) {
    const uint8_t key = static_cast<uint8_t>(r->eax);
    uint8_t* const slotKeys = GP4_ARRAY(uint8_t, 0x007c2a1c);
    uint8_t* const driverFlags = GP4_ARRAY(uint8_t, 0x007c2a48);
    uint32_t carry = 0u;
    for (uint32_t i = 0u; i < 0x16u; ++i) {
        carry = key < slotKeys[i] ? 1u : 0u;
        if (key == slotKeys[i]) {
            slotKeys[i] = 0xffu;
            driverFlags[i] &= 0x3fu;
            carry = 0u;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(1u, carry));
}
GP4_IMPL(0x00540698, FUN_00540698, "regs(eax:u32[0..255]) -> (cf, pf, af, zf, sf, of); globals=layout=blocks_0054045b+drv22_0052d402")
