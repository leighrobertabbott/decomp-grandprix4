#include <gp4/gp4.h>

enum : uint32_t { kArithFlags = 0x8d5u };

// FUNCTION: GP4 0x0040bce6
// Track-surface warm-up (wrapped by the pushal / popal shell 0x0040bd57). Clears [0x0063b81c] / [0x0063b814],
// forms the step scale [0x00622828] = ((word 0x868558 + word 0x86855a) * [0x00622820]) >> 14 and its rate
// product [0x0062282c] = [0x00622828] * [0x00603854] (low 32 bits; edx keeps the high half), draws two
// positions from the generator 0x0040142d into [0x0063b828] / [0x0063b82c], then runs the warm-up routines
// 0x0040c3c8, 0x0040b652, 0x0040c204, 0x0040b68c, 0x0040c566 and the empty hook 0x0051fc60. One register
// block is threaded through every callee and handed back.
void __cdecl FUN_0040bce6(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x0063b81c) = 0u;
    GP4_GLOBAL(uint32_t, 0x0063b814) = 0u;

    const uint32_t sum = static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x00868558))
        + static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x0086855a));
    const int64_t first = static_cast<int64_t>(static_cast<int32_t>(sum)) * GP4_GLOBAL(int32_t, 0x00622820);
    const uint32_t scaled = static_cast<uint32_t>(static_cast<uint64_t>(first) >> 14);
    GP4_GLOBAL(uint32_t, 0x00622828) = scaled;
    const int64_t second = static_cast<int64_t>(static_cast<int32_t>(scaled)) * GP4_GLOBAL(int32_t, 0x00603854);
    const uint32_t low = static_cast<uint32_t>(second);
    GP4_GLOBAL(uint32_t, 0x0062282c) = low;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.eax = low;
    c.edx = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 32);

    gp4::call_regs(0x0040142d, c);
    GP4_GLOBAL(uint32_t, 0x0063b828) = c.eax;
    gp4::call_regs(0x0040142d, c);
    GP4_GLOBAL(uint32_t, 0x0063b82c) = c.eax;
    gp4::call_regs(0x0040c3c8, c);
    gp4::call_regs(0x0040b652, c);
    gp4::call_regs(0x0040c204, c);
    gp4::call_regs(0x0040b68c, c);
    gp4::call_regs(0x0040c566, c);
    gp4::call_regs(0x0051fc60, c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~kArithFlags) | (c.eflags & kArithFlags);
}
GP4_IMPL(0x0040bce6, FUN_0040bce6, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0+map_0040c43b+ring_0040c0ae+state_0040bec7+state_0040bce6")
