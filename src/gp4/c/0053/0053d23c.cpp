#include <gp4/gp4.h>
#include <gp4/flags.h>

// Every time the tick counter [0x7a9478] is more than 4 ahead of the stamp [0x82fe80] (signed), the
// stamp is refreshed and the node list headed by [0x82fd34] (each node's +0 is the next link, 0 ends
// the list) is walked: a node with flag bit 0x10 in byte +0x1e whose cursor pointer at +0x26 has a
// nonzero dword at +4 advances the cursor by 4 (plus the dword it then points at when that is
// negative) and gets bit 0x40 set in byte +0x1f. eax and esi are saved/restored; the flags are those
// of `cmp eax, 4` when the tick gap is small, else of the final `cmp dword [esi], 0` (equal).
// FUNCTION: GP4 0x0053d23c
void __cdecl FUN_0053d23c(gp4::Regs* r) {
    const int32_t gap = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x007a9478) - GP4_GLOBAL(uint32_t, 0x0082fe80));
    uint32_t arith = gp4::flags::sub32(static_cast<uint32_t>(gap), 4u);
    if (gap > 4) {
        GP4_GLOBAL(uint32_t, 0x0082fe80) = GP4_GLOBAL(uint32_t, 0x007a9478);
        uint32_t node = GP4_GLOBAL(uint32_t, 0x0082fd34);
        for (;;) {
            const uint32_t next = GP4_FIELD(uint32_t, node, 0);
            arith = gp4::flags::sub32(next, 0u);
            if (next == 0u) {
                break;
            }
            node = next;
            if ((GP4_FIELD(uint8_t, node, 0x1e) & 0x10u) == 0u) {
                continue;
            }
            uint32_t cursor = GP4_FIELD(uint32_t, node, 0x26);
            if (GP4_FIELD(uint32_t, cursor, 4) == 0u) {
                continue;
            }
            cursor += 4u;
            GP4_FIELD(uint32_t, node, 0x26) = cursor;
            const int32_t step = GP4_FIELD(int32_t, cursor, 0);
            if (step < 0) {
                GP4_FIELD(uint32_t, node, 0x26) += static_cast<uint32_t>(step);
            }
            GP4_FIELD(uint8_t, node, 0x1f) |= 0x40u;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0053d23c, FUN_0053d23c, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mlist_0053d23c")
