#include <gp4/gp4.h>

// Stores the byte in AL into the first free (0xff) slot of the four-byte table at esi+0x74.
// Returns CF clear when stored, CF set when the table is full. The last flag writer on both
// paths is a compare that ended equal (ZF and PF set, AF/SF/OF clear); clc/stc then sets CF.
// FUNCTION: GP4 0x0043a81e
void __cdecl FUN_0043a81e(gp4::Regs* r) {
    uint8_t* slots = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi)) + 0x74;
    const uint8_t value = static_cast<uint8_t>(r->eax);
    uint32_t full = 1u;
    for (uint32_t i = 0; i < 4u; i++) {
        if (slots[i] == 0xffu) {
            slots[i] = value;
            full = 0u;
            break;
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u | full;
}
GP4_IMPL(0x0043a81e, FUN_0043a81e, "regs(esi:ptr[0x80]:bytes, eax:u32) -> (cf, pf, af, zf, sf, of)")
