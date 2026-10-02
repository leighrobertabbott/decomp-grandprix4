#include <gp4/gp4.h>

// FUNCTION: GP4 0x004197ec
// Accumulates six 32-bit deltas into the six accumulators at 0x00623e84..0x00623e98:
//   [623e88] += [623e9c]   [623e84] += [638ce4]   [623e8c] += [638cec]
//   [623e90] += [62290c]   [623e94] += [622910]   [623e98] += [622914]
// eax leaves holding the last addend and the flags are those of the last add.
void __cdecl FUN_004197ec(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x00623e88) += GP4_GLOBAL(uint32_t, 0x00623e9c);
    GP4_GLOBAL(uint32_t, 0x00623e84) += GP4_GLOBAL(uint32_t, 0x00638ce4);
    GP4_GLOBAL(uint32_t, 0x00623e8c) += GP4_GLOBAL(uint32_t, 0x00638cec);
    GP4_GLOBAL(uint32_t, 0x00623e90) += GP4_GLOBAL(uint32_t, 0x0062290c);
    GP4_GLOBAL(uint32_t, 0x00623e94) += GP4_GLOBAL(uint32_t, 0x00622910);

    const uint32_t b = GP4_GLOBAL(uint32_t, 0x00622914);
    const uint32_t a = GP4_GLOBAL(uint32_t, 0x00623e98);
    const uint32_t res = a + b;
    GP4_GLOBAL(uint32_t, 0x00623e98) = res;

    // flags of the final "add [0x623e98], eax"
    uint32_t p = res & 0xffu;
    p ^= p >> 4; p ^= p >> 2; p ^= p >> 1;
    uint32_t fl = r->eflags & ~0x8D5u;
    if (res < a) fl |= 0x001u;
    if (!(p & 1u)) fl |= 0x004u;
    if ((a ^ b ^ res) & 0x10u) fl |= 0x010u;
    if (res == 0) fl |= 0x040u;
    if (res & 0x80000000u) fl |= 0x080u;
    if (((a ^ res) & (b ^ res)) & 0x80000000u) fl |= 0x800u;

    r->eax = b;
    r->eflags = fl;
}
GP4_IMPL(0x004197ec, FUN_004197ec, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
