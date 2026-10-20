#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal; walks the singly linked list that starts at esi (dword +0 = next, 0 ends the list). For each
// hop eax is shifted right by one and, when the bit shifted out is set, bit 6 of the flag byte +0x1f of
// the node just reached is set. popal restores every register; the walk always ends on
// `cmp dword [esi], 0` with a zero dword, so the flags are those of 0 - 0 (ZF and PF set).
// FUNCTION: GP4 0x0053e612
void __cdecl FUN_0053e612(gp4::Regs* r) {
    uint32_t node = r->esi;
    uint32_t bits = r->eax;
    while (GP4_FIELD(uint32_t, node, 0) != 0u) {
        node = GP4_FIELD(uint32_t, node, 0);
        const uint32_t carry = bits & 1u;
        bits >>= 1;
        if (carry != 0u) {
            GP4_FIELD(uint8_t, node, 0x1f) |= 0x40u;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0u, 0u));
}
GP4_IMPL(0x0053e612, FUN_0053e612, "regs(eax:u32, esi:ptr[0x28]:layout=lnode1_0053e612) -> (cf, pf, af, zf, sf, of)")
