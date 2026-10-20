#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0053ccc7
// Marks the cl-th node of the list headed by [0x82fd34] (cl = byte at 0x82fd45; 0xff = none
// selected): clears bit 0x80 and sets bit 0x40 of the node's byte +0x1f. A null link met on the
// way calls the routine 0x54e232 (a bare `ret`; the original passes esi = 0x53ccee, which that
// routine ignores) between pushal/popal and leaves.
// Register effects: cl is loaded from 0x82fd45 (and counted down to 0 on success); esi ends as the
// marked node (success), 0 (null link) or is untouched (cl == 0xff). Flags: cmp cl,0xff when none
// selected; or esi,esi (esi == 0 -> ZF = PF = 1) on the null path, where popal restores the rest;
// the final `or byte [esi+0x1f], 0x40` on success.
void __cdecl FUN_0053ccc7(gp4::Regs* r) {
    uint32_t cl = GP4_GLOBAL(uint8_t, 0x0082fd45);
    uint32_t flags = gp4::flags::sub8(cl, 0xffu);
    if (cl != 0xffu) {
        uint32_t node = GP4_GLOBAL(uint32_t, 0x0082fd34);
        for (;;) {
            flags = gp4::flags::logic32(node);
            if (node == 0u) {
                gp4::cdecl_<0x0054e232, void>();
                r->esi = 0u;
                break;
            }
            node = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(node));
            cl = (cl - 1u) & 0xffu;
            if (cl == 0u) {
                uint8_t* flagByte = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(node)) + 0x1f;
                *flagByte = static_cast<uint8_t>(*flagByte & 0x7fu);
                *flagByte = static_cast<uint8_t>(*flagByte | 0x40u);
                flags = gp4::flags::logic8(*flagByte);
                r->esi = node;
                break;
            }
        }
    }
    r->ecx = (r->ecx & 0xffffff00u) | cl;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0053ccc7, FUN_0053ccc7, "regs(ecx:u32, esi:u32) -> (ecx, esi, cf, pf, af, zf, sf, of); globals=layout=menu_0084300a+sel_0053ccc7")
