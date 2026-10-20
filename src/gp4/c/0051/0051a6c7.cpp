#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0051a6c7
// Snapshots game state into the block at 0x00b1d3ec.. (inside a pushal/popal pair, so every register
// is kept): [0xb1d3ec] = [0x875274], [0xb1d3fc] = 0x16, [0xb1d400] = [0x7ad894], word [0xb1d40c] =
// word [0x679b4e], [0xb1d404] = 1 when bit 1 of the byte +0x70 of the ring record [0x679d20] is
// set (else 0), [0xb1d408] = the byte [0x7c2ea9]. popal does not restore the flags: they are those
// of `test byte [eax+0x70], 2`.
void __cdecl FUN_0051a6c7(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x00b1d3ec) = GP4_GLOBAL(uint32_t, 0x00875274);
    GP4_GLOBAL(uint32_t, 0x00b1d3fc) = 0x16u;
    GP4_GLOBAL(uint32_t, 0x00b1d400) = GP4_GLOBAL(uint32_t, 0x007ad894);
    GP4_GLOBAL(uint16_t, 0x00b1d40c) = GP4_GLOBAL(uint16_t, 0x00679b4e);

    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00679d20)));
    const uint32_t bit = GP4_FIELD(uint8_t, record, 0x70) & 2u;
    GP4_GLOBAL(uint32_t, 0x00b1d404) = bit != 0u ? 1u : 0u;
    GP4_GLOBAL(uint32_t, 0x00b1d408) = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(bit));
}
GP4_IMPL(0x0051a6c7, FUN_0051a6c7, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=box_00550798+segflag_00510b48")
