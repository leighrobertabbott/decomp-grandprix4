#include <gp4/gp4.h>
#include <gp4/flags.h>

// Singly linked list node: +0 next; +4, +8, +0xc, +0x10 are compared between neighbours.
struct Unknown_0054a057_Node {
    Unknown_0054a057_Node* Next00;
    uint32_t Unknown04;
    uint32_t Unknown08;
    uint32_t Unknown0C;
    uint32_t Unknown10;
};

// FUNCTION: GP4 0x0054a057
// Merges neighbouring list entries. The list head cell is the (unaligned) dword at 0x00831d81;
// ebx walks the cells (ebx = [ebx]). For each cell the pair a = [ebx], b = [a] is examined (the
// walk ends when either is null): when b's +8 equals a's +0x10 + 1 and +4 and +0xc match, b is
// absorbed: a takes b's +0x10 and next link, and b is pushed onto the free list whose head is
// the dword 0x00831d85 (b's next = old head, head = b).
// Registers: eax is the last value loaded, ebx the last cell, esi/edi the last pair (edi stays the
// caller's when the first cell is empty). The flags escaping are those of the last `or reg, reg` of
// a null pointer: ZF|PF.
void __cdecl FUN_0054a057(gp4::Regs* r) {
    uint32_t eax = r->eax;
    uint32_t edi = r->edi;
    uint32_t ebx = 0x00831d81u;
    uint32_t esi;

    for (;;) {
        esi = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(ebx));
        if (esi == 0u) {
            break;
        }
        Unknown_0054a057_Node* a = reinterpret_cast<Unknown_0054a057_Node*>(static_cast<uintptr_t>(esi));
        edi = reinterpret_cast<uintptr_t>(a->Next00);
        if (edi == 0u) {
            break;
        }
        Unknown_0054a057_Node* b = reinterpret_cast<Unknown_0054a057_Node*>(static_cast<uintptr_t>(edi));
        eax = a->Unknown10 + 1u;
        if (eax == b->Unknown08) {
            eax = a->Unknown04;
            if (eax == b->Unknown04) {
                eax = a->Unknown0C;
                if (eax == b->Unknown0C) {
                    eax = b->Unknown10;
                    a->Unknown10 = eax;
                    eax = reinterpret_cast<uintptr_t>(b->Next00);
                    a->Next00 = b->Next00;
                    eax = GP4_GLOBAL(uint32_t, 0x00831d85);
                    b->Next00 = reinterpret_cast<Unknown_0054a057_Node*>(static_cast<uintptr_t>(eax));
                    GP4_GLOBAL(uint32_t, 0x00831d85) = edi;
                }
            }
        }
        ebx = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(ebx));
    }

    r->eax = eax;
    r->ebx = ebx;
    r->esi = esi;
    r->edi = edi;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(0u));
}
GP4_IMPL(0x0054a057, FUN_0054a057, "regs(eax:u32, edi:u32) -> (eax, ebx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=runs_0054a057")
