#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal; walks the singly linked list that starts at esi (dword +0 = next). The walk ends when the
// next link is 0 or the sentinel address 0x00838c94. For each hop eax is shifted right by one and the
// flag byte +0x1f of the node just reached gets bit 4 set when the bit shifted out is 1, cleared
// otherwise. popal restores every register; the walk ends on a compare of equal dwords, so the flags
// are those of x - x (ZF and PF set).
// FUNCTION: GP4 0x0053e56a
void __cdecl FUN_0053e56a(gp4::Regs* r) {
    uint32_t node = r->esi;
    uint32_t bits = r->eax;
    for (;;) {
        const uint32_t next = GP4_FIELD(uint32_t, node, 0);
        if (next == 0u || next == 0x00838c94u) {
            break;
        }
        node = next;
        const uint32_t carry = bits & 1u;
        bits >>= 1;
        if (carry != 0u) {
            GP4_FIELD(uint8_t, node, 0x1f) |= 0x10u;
        } else {
            GP4_FIELD(uint8_t, node, 0x1f) &= 0xefu;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0u, 0u));
}
GP4_IMPL(0x0053e56a, FUN_0053e56a, "regs(eax:u32, esi:ptr[0x28]:layout=snode1_0053e56a) -> (cf, pf, af, zf, sf, of)")
