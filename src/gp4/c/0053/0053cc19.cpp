#include <gp4/gp4.h>
#include <gp4/flags.h>

// Selects the next selectable item of the menu list headed by [0x0082fd34] (items chained by +0
// next pointers, id byte at +0xc, flag bytes +0x1e / +0x1f). Starting point: the item reached by
// hopping max(1, [0x0082fd45]) links from the header (a hop count above [0x0082fd52] reports
// through 0x0054e232 and uses 1 instead). From there on the first item with +0x1e bits 0x40 and
// 0x80 set and +0x1f bits 0x20 and 0x10 clear wins; otherwise the walk steps to the next item
// (the list wraps through the header's first item) and stops, after reporting, when it meets the
// current item id again (then it takes the first item). The winner's id goes to [0x0082fd45].
// pushal/popal around the reports; ecx ends 0, esi = the winner, dl = its id, the arithmetic flags
// are always ZF | PF (the `test` that found it, or the equal `cmp` of the id).
// FUNCTION: GP4 0x0053cc19
void __cdecl FUN_0053cc19(gp4::Regs* r) {
    const uint32_t header = GP4_GLOBAL(uint32_t, 0x0082fd34);
    uint32_t hops = GP4_GLOBAL(uint8_t, 0x0082fd45);
    if (hops < 1u) {
        hops = 1u;
    }
    if (hops > GP4_GLOBAL(uint32_t, 0x0082fd52)) {
        gp4::cdecl_<0x0054e232, void>();
        hops = 1u;
    }

    uint32_t node = header;
    do {
        node = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(node));
        --hops;
    } while (hops != 0u);

    uint32_t arith = 0u;
    for (;;) {
        const uint8_t* item = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(node));
        if ((item[0x1e] & 0x40u) != 0u && (item[0x1f] & 0x20u) == 0u && (item[0x1e] & 0x80u) != 0u
            && (item[0x1f] & 0x10u) == 0u) {
            arith = gp4::flags::logic8(item[0x1f] & 0x10u);
            break;
        }
        node = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(node));
        if (node == 0u) {
            node = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0082fd34)));
        }
        const uint8_t id = *(reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(node)) + 0xc);
        if (id == GP4_GLOBAL(uint8_t, 0x0082fd45)) {
            arith = gp4::flags::sub8(id, id);
            gp4::cdecl_<0x0054e232, void>();
            node = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0082fd34)));
            break;
        }
    }

    const uint8_t id = *(reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(node)) + 0xc);
    GP4_GLOBAL(uint8_t, 0x0082fd45) = id;
    r->ecx = 0u;
    r->edx = (r->edx & 0xffffff00u) | id;
    r->esi = node;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0053cc19, FUN_0053cc19, "regs(edx:u32) -> (ecx, edx, esi, cf, pf, af, zf, sf, of); globals=layout=menu_0084300a+sel_0053cc19")
