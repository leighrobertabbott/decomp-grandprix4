#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040837c
void __cdecl FUN_0040837c(gp4::Regs* r) {
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t state = record[0x7a];
    uint32_t flags;
    if ((state & 0x80u) != 0u) {
        // test leaves SF set (result 0x80), PF clear; clc clears CF
        flags = 0x80u;
    } else if ((state & 0x40u) != 0u) {
        // test result 0x40: SF/ZF/PF clear; clc clears CF
        flags = 0u;
    } else {
        // test result 0: ZF and PF set; stc sets CF
        flags = 0x45u;
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040837c, FUN_0040837c, "regs(esi:ptr[0x82]:bytes) -> (cf, pf, af, zf, sf, of)")
