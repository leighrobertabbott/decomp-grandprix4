#include <gp4/gp4.h>
#include <gp4/flags.h>

// Singly linked list node: +0 next, +8 sort key (signed). The callers (0x00549f22/0x00549f55) pass
// ebx = the address of the cell that holds the first-node pointer (+0 of a pseudo node).
struct SortNode {
    SortNode* Next00;
    uint32_t Unknown04;
    int32_t Key08;
};

// FUNCTION: GP4 0x0054a09f
// One bubble pass over the list whose first pointer is stored at [ebx]: whenever a node's key is
// greater than its successor's, the two nodes swap places. (The original clears dl and never sets
// it, so its "repeat while swapped" jump back to the start is never taken: exactly one pass.)
// pushal/popal restore every register; the last flag writer is `or esi,esi` (esi == 0) or
// `or dl,dl` (dl == 0), so the arithmetic flags are always ZF|PF.
void __cdecl FUN_0054a09f(gp4::Regs* r) {
    SortNode* cell = (SortNode*)(uintptr_t)r->ebx;

    if (cell->Next00 != 0) {
        SortNode* prev = cell;
        for (;;) {
            SortNode* first = prev->Next00;
            if (first == 0) {
                break;
            }
            SortNode* second = first->Next00;
            if (second == 0) {
                break;
            }
            if (first->Key08 > second->Key08) {
                SortNode* after = second->Next00;
                prev->Next00 = second;
                second->Next00 = first;
                first->Next00 = after;
                first = second;
            }
            prev = first;
        }
    }

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(0u));
}
GP4_IMPL(0x0054a09f, FUN_0054a09f, "regs(ebx:ptr[0x10]:layout=head_0054a09f) -> (cf, pf, af, zf, sf, of)")
