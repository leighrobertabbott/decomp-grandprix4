#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu selection fix-up on the current menu [0x0082fd34] (header: +0 first item, +0xd / +0xf item
// counters; item: +0 next link, +0xc id, +0x1e / +0x1f flag bytes). The start index is the header
// byte +0xf (else +0xd), remembered in [0x0082fd45] and forced to at least 1; above the item count
// [0x0082fd52] the (empty) hook 0x0054e232 runs and the index becomes 1. The item is reached by
// following that many links from the header. From there the first item with flag 0x40 in +0x1e
// and neither 0x20 nor 0x10 in +0x1f is searched (links followed, wrapping to the first item at a
// null link); the search ends as well when it comes back to an item whose id equals [0x0082fd45]
// (the hook runs again and the first item is taken). [0x0082fd45] receives the id of the chosen
// item. Registers: ecx = 0, esi = the chosen item, dl = its id; the flags are those of the
// `test [esi+0x1f], 0x10` that accepted it or of the final id compare.
// FUNCTION: GP4 0x0053cb67
void __cdecl FUN_0053cb67(gp4::Regs* r) {
    const uint32_t header = GP4_GLOBAL(uint32_t, 0x0082fd34);
    uint32_t node = header;
    uint32_t count = GP4_FIELD(uint8_t, header, 0xf);
    if (count == 0u) {
        count = GP4_FIELD(uint8_t, header, 0xd);
    }
    GP4_GLOBAL(uint8_t, 0x0082fd45) = static_cast<uint8_t>(count);
    if (count < 1u) {
        count = 1u;
    }
    if (count > GP4_GLOBAL(uint32_t, 0x0082fd52)) {
        gp4::Regs hook = *r;
        hook.esi = 0x0053cb93u;
        gp4::call_regs(0x0054e232, hook);
        count = 1u;
    }
    do {
        node = GP4_FIELD(uint32_t, node, 0);
        --count;
    } while (count != 0u);

    uint32_t arith = 0u;
    for (;;) {
        bool next = true;
        if ((GP4_FIELD(uint8_t, node, 0x1e) & 0x40u) != 0u) {
            if ((GP4_FIELD(uint8_t, node, 0x1f) & 0x20u) == 0u) {
                arith = gp4::flags::logic8(GP4_FIELD(uint8_t, node, 0x1f) & 0x10u);
                if ((GP4_FIELD(uint8_t, node, 0x1f) & 0x10u) == 0u) {
                    next = false;
                }
            }
        }
        if (!next) {
            break;
        }
        node = GP4_FIELD(uint32_t, node, 0);
        if (node == 0u) {
            node = GP4_FIELD(uint32_t, GP4_GLOBAL(uint32_t, 0x0082fd34), 0);
        }
        const uint8_t id = GP4_FIELD(uint8_t, node, 0xc);
        arith = gp4::flags::sub8(id, GP4_GLOBAL(uint8_t, 0x0082fd45));
        if (id == GP4_GLOBAL(uint8_t, 0x0082fd45)) {
            gp4::Regs hook = *r;
            hook.esi = 0x0053cbe8u;
            gp4::call_regs(0x0054e232, hook);
            node = GP4_FIELD(uint32_t, GP4_GLOBAL(uint32_t, 0x0082fd34), 0);
            break;
        }
    }

    const uint8_t id = GP4_FIELD(uint8_t, node, 0xc);
    GP4_GLOBAL(uint8_t, 0x0082fd45) = id;
    r->ecx = 0u;
    r->edx = (r->edx & 0xffffff00u) | id;
    r->esi = node;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0053cb67, FUN_0053cb67, "regs(edx:u32) -> (ecx, edx, esi, cf, pf, af, zf, sf, of); globals=layout=hdr_0053cb67")
