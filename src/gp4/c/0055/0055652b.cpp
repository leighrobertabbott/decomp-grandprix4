#include <gp4/gp4.h>
#include <gp4/flags.h>

// Runs the per-record step 0x00556457 over the ring records from 0x8943a0 up to the lap end [0x866ecc], merges the
// bits 0x1800300 of the last lap record (end - 0x160, dword +0x2c) into the five dwords +0x30..+0x40 of the first
// record, then runs the step over the pit records from [0x891500] up to the pit end [0x866ef8]. Both walks are
// do-while loops that stop only on equality. The step's state (ebp, ecx, ebx, eax) is carried through every call; ebp
// and ecx are cleared before each walk. pushal/popal restore every register; the flags are those of the final
// `cmp edi, [0x866ef8]` (equal).
// FUNCTION: GP4 0x0055652b
void __cdecl FUN_0055652b(gp4::Regs* r) {
    gp4::Regs c = *r;
    uint32_t end;

    c.ebp = 0u;
    c.ecx = 0u;
    c.edi = 0x008943a0u;
    do {
        gp4::call_regs(0x00556457, c);
        c.edi += 0x160u;
        end = GP4_GLOBAL(uint32_t, 0x00866ecc);
    } while (c.edi != end);

    c.edi = 0x008943a0u;
    c.esi = GP4_GLOBAL(uint32_t, 0x00866ecc) - 0x160u;
    c.eax = GP4_FIELD(uint32_t, c.esi, 0x2c) & 0x01800300u;
    GP4_FIELD(uint32_t, c.edi, 0x30) |= c.eax;
    GP4_FIELD(uint32_t, c.edi, 0x38) |= c.eax;
    GP4_FIELD(uint32_t, c.edi, 0x34) |= c.eax;
    GP4_FIELD(uint32_t, c.edi, 0x40) |= c.eax;
    GP4_FIELD(uint32_t, c.edi, 0x3c) |= c.eax;

    c.ebp = 0u;
    c.ecx = 0u;
    c.edi = GP4_GLOBAL(uint32_t, 0x00891500);
    do {
        gp4::call_regs(0x00556457, c);
        c.edi += 0x160u;
        end = GP4_GLOBAL(uint32_t, 0x00866ef8);
    } while (c.edi != end);

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(c.edi, end));
}
GP4_IMPL(0x0055652b, FUN_0055652b, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=bits_0055616f+ring_00555b28")
