#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sets up a 0x300-byte-block counter: remembers eax (DAT_0083082c), stores the size in ecx at both
// 0x0083083c and 0x00830840, computes the block count ceil(size / 0x300) at 0x00830838 and clears
// 0x00830834.  Leaves ecx = 0x300, edx = remainder of (size + 0x2ff) / 0x300 and the flags of the
// `add eax, 0x2ff` (DIV leaves the preceding flags in the oracle).
// FUNCTION: GP4 0x00541830
void __cdecl FUN_00541830(gp4::Regs* r) {
    const uint32_t first = r->eax;
    const uint32_t size = r->ecx;

    GP4_GLOBAL(uint32_t, 0x0083082c) = first;
    GP4_GLOBAL(uint32_t, 0x0083083c) = size;
    GP4_GLOBAL(uint32_t, 0x00830840) = size;

    const uint32_t sum = size + 0x2ffu;
    const uint32_t arith = gp4::flags::add32(size, 0x2ffu);
    uint32_t rem = 0;
    const uint32_t blocks = gp4::div64((uint64_t)sum, 0x300u, &rem);
    GP4_GLOBAL(uint32_t, 0x00830838) = blocks;
    GP4_GLOBAL(uint32_t, 0x00830834) = 0u;

    r->eax = blocks;
    r->ecx = 0x300u;
    r->edx = rem;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00541830, FUN_00541830, "regs(eax:u32, ecx:u32) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
