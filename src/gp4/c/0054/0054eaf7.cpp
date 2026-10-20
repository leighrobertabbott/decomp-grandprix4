#include <gp4/gp4.h>
#include <gp4/flags.h>

// Walks the linked list whose head node is at 0x00838c94 (next pointer at +0). eax is consumed one bit per
// node, low bit first (`rcr eax, 1`; the CF shifted in is always clear, because the `cmp esi, 0` of every
// round clears it); a set bit sets 0x40 in the byte +0x1f of that node. Exit: eax shifted right by the node
// count, the flags are those of the final `cmp esi, 0` (esi == 0). esi is saved and restored.
// FUNCTION: GP4 0x0054eaf7
void __cdecl FUN_0054eaf7(gp4::Regs* r) {
    uint32_t bits = r->eax;
    uint8_t* node = GP4_ARRAY(uint8_t, 0x00838c94);
    while (node != nullptr) {
        const uint32_t selected = bits & 1u;
        bits = bits >> 1;
        if (selected != 0u) {
            node[0x1f] = static_cast<uint8_t>(node[0x1f] | 0x40u);
        }
        node = *reinterpret_cast<uint8_t**>(node);
    }
    r->eax = bits;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0u, 0u));
}
GP4_IMPL(0x0054eaf7, FUN_0054eaf7, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=list_0054eaf7+links_0054eaf7")
