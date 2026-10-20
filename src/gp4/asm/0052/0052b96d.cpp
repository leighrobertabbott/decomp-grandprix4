#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0052b96d
// pushal/popal wrapper: selects entry eax of the 0x34-byte preset table at 0x7ab5b4 and scatters
// 22 of its bytes (+0x00..+0x11, +0x1e, +0x1f, +0x32, +0x33) to the current-preset bytes at
// 0x81d60f..0x81d620 and 0x81d62d..0x81d630 (the order is not linear: entry +2 goes to 0x81d60f,
// +8 to 0x81d610, +0xb to 0x81d611, +0xe to 0x81d612, +0x10 to 0x81d613, +0x11 to 0x81d614, the
// rest to 0x81d615..0x81d620). popal restores every register; the flags stay those of
// `add eax, 0x7ab5b4` (eax = index * 0x34).
void __cdecl FUN_0052b96d(gp4::Regs* r) {
    const uint32_t offset = r->eax * 0x34u;
    const uint8_t* entry = reinterpret_cast<const uint8_t*>(offset + 0x007ab5b4u);

    GP4_GLOBAL(uint8_t, 0x0081d615) = entry[0x00];
    GP4_GLOBAL(uint8_t, 0x0081d616) = entry[0x01];
    GP4_GLOBAL(uint8_t, 0x0081d60f) = entry[0x02];
    GP4_GLOBAL(uint8_t, 0x0081d617) = entry[0x03];
    GP4_GLOBAL(uint8_t, 0x0081d618) = entry[0x04];
    GP4_GLOBAL(uint8_t, 0x0081d619) = entry[0x05];
    GP4_GLOBAL(uint8_t, 0x0081d61a) = entry[0x06];
    GP4_GLOBAL(uint8_t, 0x0081d61b) = entry[0x07];
    GP4_GLOBAL(uint8_t, 0x0081d610) = entry[0x08];
    GP4_GLOBAL(uint8_t, 0x0081d61c) = entry[0x09];
    GP4_GLOBAL(uint8_t, 0x0081d61d) = entry[0x0a];
    GP4_GLOBAL(uint8_t, 0x0081d611) = entry[0x0b];
    GP4_GLOBAL(uint8_t, 0x0081d61e) = entry[0x0c];
    GP4_GLOBAL(uint8_t, 0x0081d61f) = entry[0x0d];
    GP4_GLOBAL(uint8_t, 0x0081d612) = entry[0x0e];
    GP4_GLOBAL(uint8_t, 0x0081d620) = entry[0x0f];
    GP4_GLOBAL(uint8_t, 0x0081d613) = entry[0x10];
    GP4_GLOBAL(uint8_t, 0x0081d614) = entry[0x11];
    GP4_GLOBAL(uint8_t, 0x0081d62d) = entry[0x1e];
    GP4_GLOBAL(uint8_t, 0x0081d62e) = entry[0x1f];
    GP4_GLOBAL(uint8_t, 0x0081d62f) = entry[0x32];
    GP4_GLOBAL(uint8_t, 0x0081d630) = entry[0x33];

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(offset, 0x007ab5b4u));
}
GP4_IMPL(0x0052b96d, FUN_0052b96d, "regs(eax:u32[0..7]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=presets_007ab5b4")
