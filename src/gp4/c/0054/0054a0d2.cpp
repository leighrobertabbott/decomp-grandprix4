#include <gp4/gp4.h>
#include <gp4/flags.h>

#pragma pack(push, 1)
// Singly linked list node walked from the (unaligned) head pointer at 0x00831d81.
struct ListNode_0054a0d2 {
    uint32_t Next00;
    uint32_t Unknown04;
    uint32_t Unknown08;
    uint32_t Unknown0C;
    uint32_t Unknown10;
};
#pragma pack(pop)

// Clears the screen with colour 0, then waits for the space key; and does the same once for each
// node of the list at [0x00831d81], passing the node's 1-based index (wrapping 15 -> 1) to 0x00531af4
// first. Leaves eax = the next index, esi = 0 and the flags of the final `or esi, esi`; the other
// registers are whatever the last callee returned.
// FUNCTION: GP4 0x0054a0d2
void __cdecl FUN_0054a0d2(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.eax = 0u;
    gp4::call_regs(0x00549d26, c);
    gp4::call_regs(0x00549d12, c);
    gp4::call_regs(0x0052d75a, c);

    uint32_t node = GP4_GLOBAL(uint32_t, 0x00831d81);
    uint32_t index = 1u;
    while (node != 0u) {
        const ListNode_0054a0d2* n = reinterpret_cast<const ListNode_0054a0d2*>(static_cast<uintptr_t>(node));
        c.esi = node;
        c.eax = index;
        gp4::call_regs(0x00531af4, c);
        c.ebx = n->Unknown04;
        c.eax = n->Unknown08;
        c.ebp = n->Unknown0C;
        c.ecx = n->Unknown10;
        gp4::call_regs(0x00531b3d, c);
        gp4::call_regs(0x00549d12, c);
        gp4::call_regs(0x0052d75a, c);
        index += 1u;
        if (index == 0x10u) {
            index = 1u;
        }
        node = n->Next00;
    }

    c.eax = index;
    c.esi = node;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(node));
    *r = c;
}
GP4_IMPL(0x0054a0d2, FUN_0054a0d2, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=surfcopy_00549d55+keyq_008245c0; cases=40; wall=3600; emu=skip:win32 PeekMessageA")
