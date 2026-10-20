#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043a05a
void __cdecl FUN_0043a05a(gp4::Regs* r) {
    uint8_t* table = GP4_ARRAY(uint8_t, 0x0063db7c);

    // Claim the first free slot (1..15): mark it 0xff and report it with CF set.
    uint32_t index = 1;
    do {
        if (table[index] == 0u) {
            table[index] = 0xff;
            r->eax = index;
            // cmp byte [..], 0 on a zero byte (ZF, PF), then stc
            r->eflags = (r->eflags & ~0x8d5u) | 0x45u;
            return;
        }
        index++;
    } while (index != 0x10u);

    // table full: the last cmp eax, 0x10 was equal (ZF, PF), then clc
    r->eax = index;
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0043a05a, FUN_0043a05a, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=slots_0043a05a")
