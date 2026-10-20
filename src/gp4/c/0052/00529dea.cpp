#include <gp4/gp4.h>
#include <gp4/flags.h>

// Per-driver slot flag bytes (22 bytes at 0x007c2a48): clears bits 6-7 of all 22, then sets bit 7
// of the first [0x0081d229] bytes (a do-while: a count of 0 would wrap to 2^32 iterations). ecx and
// edi are saved and restored; the flags are those of the last `dec ecx` (count 1 -> 0, CF kept from
// the last `or`, which clears it).
// FUNCTION: GP4 0x00529dea
void __cdecl FUN_00529dea(gp4::Regs* r) {
    uint8_t* const row = GP4_ARRAY(uint8_t, 0x007c2a48);
    for (uint32_t i = 0u; i < 0x16u; ++i) {
        row[i] = static_cast<uint8_t>(row[i] & 0x3fu);
    }
    uint32_t count = GP4_GLOBAL(uint8_t, 0x0081d229);
    uint8_t* slot = row;
    uint32_t flags;
    do {
        const uint8_t marked = static_cast<uint8_t>(*slot | 0x80u);
        *slot = marked;
        ++slot;
        flags = gp4::flags::dec32(count, gp4::flags::logic8(marked));
        --count;
    } while (count != 0u);
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00529dea, FUN_00529dea, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=drv22_0052d402+cnt_0081d229")
