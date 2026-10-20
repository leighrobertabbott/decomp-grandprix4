#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0052ba29
// Inverse of 0x0052b96d: pushal/popal wrapper that stores the 22 current-preset bytes at
// 0x81d60f..0x81d620 and 0x81d62d..0x81d630 into entry eax of the 0x34-byte preset table at
// 0x7ab5b4 (entry +0 = 0x81d615, +1 = 0x81d616, +2 = 0x81d60f, ..., +0x1e/+0x1f = 0x81d62d/2e,
// +0x32/+0x33 = 0x81d62f/30). popal restores every register; the flags stay those of
// `add eax, 0x7ab5b4` (eax = index * 0x34).
void __cdecl FUN_0052ba29(gp4::Regs* r) {
    const uint32_t offset = r->eax * 0x34u;
    uint8_t* entry = reinterpret_cast<uint8_t*>(offset + 0x007ab5b4u);

    entry[0x00] = GP4_GLOBAL(uint8_t, 0x0081d615);
    entry[0x01] = GP4_GLOBAL(uint8_t, 0x0081d616);
    entry[0x02] = GP4_GLOBAL(uint8_t, 0x0081d60f);
    entry[0x03] = GP4_GLOBAL(uint8_t, 0x0081d617);
    entry[0x04] = GP4_GLOBAL(uint8_t, 0x0081d618);
    entry[0x05] = GP4_GLOBAL(uint8_t, 0x0081d619);
    entry[0x06] = GP4_GLOBAL(uint8_t, 0x0081d61a);
    entry[0x07] = GP4_GLOBAL(uint8_t, 0x0081d61b);
    entry[0x08] = GP4_GLOBAL(uint8_t, 0x0081d610);
    entry[0x09] = GP4_GLOBAL(uint8_t, 0x0081d61c);
    entry[0x0a] = GP4_GLOBAL(uint8_t, 0x0081d61d);
    entry[0x0b] = GP4_GLOBAL(uint8_t, 0x0081d611);
    entry[0x0c] = GP4_GLOBAL(uint8_t, 0x0081d61e);
    entry[0x0d] = GP4_GLOBAL(uint8_t, 0x0081d61f);
    entry[0x0e] = GP4_GLOBAL(uint8_t, 0x0081d612);
    entry[0x0f] = GP4_GLOBAL(uint8_t, 0x0081d620);
    entry[0x10] = GP4_GLOBAL(uint8_t, 0x0081d613);
    entry[0x11] = GP4_GLOBAL(uint8_t, 0x0081d614);
    entry[0x1e] = GP4_GLOBAL(uint8_t, 0x0081d62d);
    entry[0x1f] = GP4_GLOBAL(uint8_t, 0x0081d62e);
    entry[0x32] = GP4_GLOBAL(uint8_t, 0x0081d62f);
    entry[0x33] = GP4_GLOBAL(uint8_t, 0x0081d630);

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(offset, 0x007ab5b4u));
}
GP4_IMPL(0x0052ba29, FUN_0052ba29, "regs(eax:u32[0..7]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=presets_007ab5b4")
