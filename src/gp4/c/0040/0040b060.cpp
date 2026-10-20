#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040b060
// Sets bits 1 and 2 of byte +0x12 in the six 0x160-byte records starting at [0x00866f08].
// pushal/popal keep every register; the flags are those of the final `sub ax, 1` (0 - 1 = 0xffff):
// CF | PF | AF | SF.
void __cdecl FUN_0040b060(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00866f08)));
    for (int remaining = 5; remaining >= 0; --remaining) {
        GP4_FIELD(uint8_t, record, 0x12) |= 1u;
        GP4_FIELD(uint8_t, record, 0x12) |= 2u;
        record += 0x160;
    }
    r->eflags = (r->eflags & ~0x8d5u) | 0x95u;
}
GP4_IMPL(0x0040b060, FUN_0040b060, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=ring_0040b060")
