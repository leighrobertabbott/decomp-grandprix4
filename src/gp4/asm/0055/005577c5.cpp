#include <gp4/gp4.h>
#include <gp4/flags.h>

static void ApplyBit(uint32_t ax, uint32_t bit, uint32_t value, uint32_t& edx, uint32_t& arith) {
    arith = gp4::flags::logic16(ax & bit);
    if ((ax & bit) != 0u) {
        edx |= value;
        arith = gp4::flags::logic32(edx);
    }
}

// Converts a 10-bit selector mask in ax to the status bits in edx: bit n of ax sets edx bit 11 + n for
// n = 0..7 (0x800 .. 0x40000) and bits 8 and 9 set edx bits 27 and 28 (0x8000000, 0x10000000). eax is
// not changed. The flags are those of the last flag writer: the `or edx, K` of the last set bit, else
// the `test ax, 0x200`.
// FUNCTION: GP4 0x005577c5
void __cdecl FUN_005577c5(gp4::Regs* r) {
    const uint32_t ax = r->eax & 0xffffu;
    uint32_t edx = 0u;
    uint32_t arith = 0u;

    ApplyBit(ax, 0x001u, 0x00000800u, edx, arith);
    ApplyBit(ax, 0x002u, 0x00001000u, edx, arith);
    ApplyBit(ax, 0x004u, 0x00002000u, edx, arith);
    ApplyBit(ax, 0x008u, 0x00004000u, edx, arith);
    ApplyBit(ax, 0x010u, 0x00008000u, edx, arith);
    ApplyBit(ax, 0x020u, 0x00010000u, edx, arith);
    ApplyBit(ax, 0x040u, 0x00020000u, edx, arith);
    ApplyBit(ax, 0x080u, 0x00040000u, edx, arith);
    ApplyBit(ax, 0x100u, 0x08000000u, edx, arith);
    ApplyBit(ax, 0x200u, 0x10000000u, edx, arith);

    r->edx = edx;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005577c5, FUN_005577c5, "regs(eax:u32) -> (edx, cf, pf, af, zf, sf, of)")
