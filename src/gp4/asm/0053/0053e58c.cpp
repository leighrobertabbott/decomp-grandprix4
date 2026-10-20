#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal; walks the singly linked list whose first node is the dword at [esi] (dword +0 = next).
// The walk ends at a null link or at the static list head 0x00838c94. The 96-bit value ebx:edx:eax
// is consumed one bit per node, lowest bit first (shr ebx / rcr edx / rcr eax): a set bit sets 0x10
// in the flag byte +0x1f of the node reached, a clear bit clears it. popal restores every register;
// the walk always ends on a compare of equal values (`or esi, esi` with esi = 0, or `cmp esi,
// 0x838c94`), so the flags are those of 0 - 0 (ZF and PF set).
// FUNCTION: GP4 0x0053e58c
void __cdecl FUN_0053e58c(gp4::Regs* r) {
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t ebx = r->ebx;
    uint32_t node = GP4_FIELD(uint32_t, r->esi, 0);
    while (node != 0u && node != 0x00838c94u) {
        const uint32_t from_ebx = ebx & 1u;
        ebx >>= 1;
        const uint32_t from_edx = edx & 1u;
        edx = (from_ebx << 31) | (edx >> 1);
        const uint32_t selected = eax & 1u;
        eax = (from_edx << 31) | (eax >> 1);
        if (selected != 0u) {
            GP4_FIELD(uint8_t, node, 0x1f) |= 0x10u;
        } else {
            GP4_FIELD(uint8_t, node, 0x1f) &= 0xefu;
        }
        node = GP4_FIELD(uint32_t, node, 0);
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0u, 0u));
}
GP4_IMPL(0x0053e58c, FUN_0053e58c, "regs(eax:u32, edx:u32, ebx:u32, esi:ptr[0x28]:layout=snode1_0053e56a) -> (cf, pf, af, zf, sf, of)")
