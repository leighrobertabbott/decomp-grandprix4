#include <gp4/gp4.h>
#include <gp4/flags.h>

// Pops one 16-bit entry from a circular word queue: the read cursor lives at
// 0x8245c4, the write cursor at 0x8245c0, the ring is 0x8245c8..0x824648.
// Returns the word in ax (upper half of eax kept), or eax = 0 when the queue is
// empty. Leaves the flags of the last compare; edi is preserved.

// FUNCTION: GP4 0x0052d7e2
void __cdecl FUN_0052d7e2(gp4::Regs* r) {
    uint32_t rd = GP4_GLOBAL(uint32_t, 0x008245c4);
    const uint32_t wr = GP4_GLOBAL(uint32_t, 0x008245c0);
    uint32_t arith;
    if (rd == wr) {
        arith = gp4::flags::sub32(rd, wr);
        r->eax = 0;
    } else {
        r->eax = (r->eax & 0xffff0000u) | *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(rd));
        rd += 2;
        arith = gp4::flags::sub32(rd, 0x00824648u);
        if (rd >= 0x00824648u) rd = 0x008245c8u;
        GP4_GLOBAL(uint32_t, 0x008245c4) = rd;
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0052d7e2, FUN_0052d7e2, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=keyq_008245c0")
