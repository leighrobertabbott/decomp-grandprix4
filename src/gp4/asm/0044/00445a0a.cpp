#include <gp4/gp4.h>

// Looks up the byte key al in the 22-entry byte table at 0x0062f916. Found: eax = the index (whole register) and
// CF clear; key 0xff or not found: eax unchanged and CF set. Every exit follows an equal compare (ZF and PF set,
// the other arithmetic flags clear) with only CF replaced by clc / stc. ecx is saved and restored.
// FUNCTION: GP4 0x00445a0a
void __cdecl FUN_00445a0a(gp4::Regs* r) {
    const uint8_t key = static_cast<uint8_t>(r->eax);
    uint32_t flags = 0x45u;
    if (key != 0xffu) {
        const uint8_t* const table = GP4_ARRAY(const uint8_t, 0x0062f916);
        for (uint32_t index = 0u; index != 0x16u; ++index) {
            if (table[index] == key) {
                r->eax = index;
                flags = 0x44u;
                break;
            }
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00445a0a, FUN_00445a0a, "regs(eax:u32{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,0xff,0x1ff05}) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slots_0062f916")
