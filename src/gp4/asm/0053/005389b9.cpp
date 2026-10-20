#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x005389b9
// Allocates eax bytes from the top of the block pool: the request size is stored in [0x82fcb8]
// and the free end [0x82fcb0] is lowered by it, provided the new end does not cross below the used
// end [0x82fcac]. Otherwise the slot picked by 0x005388ed is evicted with 0x00538921 and the test
// is repeated; when no slot is left the routine fails. On success eax = the new free end, CF = 0
// and the other flags are those of `cmp eax, [0x82fcac]`; on failure eax = 0, CF = 1 and ZF = PF = 1
// (the flags of `cmp eax, 0`). ebx, ecx, edx, esi and edi are saved and restored.
void __cdecl FUN_005389b9(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x0082fcb8) = r->eax;

    for (;;) {
        const uint32_t freeEnd = GP4_GLOBAL(uint32_t, 0x0082fcb0);
        const uint32_t request = GP4_GLOBAL(uint32_t, 0x0082fcb8);
        const uint32_t newEnd = freeEnd - request;
        if (freeEnd >= request) {
            const uint32_t usedEnd = GP4_GLOBAL(uint32_t, 0x0082fcac);
            if (newEnd >= usedEnd) {
                GP4_GLOBAL(uint32_t, 0x0082fcb0) = newEnd;
                r->eax = newEnd;
                r->eflags = gp4::flags::merge(
                    r->eflags, gp4::flags::sub32(newEnd, usedEnd) & ~static_cast<uint32_t>(gp4::flags::CF));
                return;
            }
        }

        gp4::Regs pick = {};
        gp4::call_regs(0x005388ed, pick);
        if (pick.eax == 0u) {
            gp4::cdecl_<0x0054e232, void>();
            r->eax = 0u;
            r->eflags = gp4::flags::merge(
                r->eflags, gp4::flags::sub32(0u, 0u) | static_cast<uint32_t>(gp4::flags::CF));
            return;
        }

        gp4::Regs evict = {};
        evict.eax = pick.eax;
        gp4::call_regs(0x00538921, evict);
    }
}
GP4_IMPL(0x005389b9, FUN_005389b9, "regs(eax:u32{0,1,0x10,0x20,0x3f,0x40,0x41,0x80,0xc0,0x100,0x140,0x180,0x200,0x280,0x300,0xa06100,0xa06200,0xa06301,0xffffffff,0x1000000}) -> (eax, cf, pf, af, zf, sf, of); globals=layout=pool_005389b9")
