#include <gp4/gp4.h>

// FUNCTION: GP4 0x0042e968
void __cdecl FUN_0042e968(gp4::Regs* r) {
    const uint8_t* first = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t* second = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint32_t flags;
    if (((first[0x179] & 0x80u) == 0u) && ((second[0x179] & 0x80u) == 0u)) {
        // both tests give zero: ZF and PF set, then stc sets CF
        flags = 0x45u;
    } else {
        // the last test left 0x80 (SF set, ZF/PF clear), then clc cleared CF
        flags = 0x80u;
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042e968, FUN_0042e968, "regs(esi:ptr[0x181]:bytes, ebx:ptr[0x181]:bytes) -> (cf, pf, af, zf, sf, of)")
