#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal: when the mode byte [0x7c2ea9] is 0x80 clears bit 3 of the flag byte at +0x83 of each
// of the 22 car records (stride 0x41c) at 0x0066fef0. Registers are restored; the flags are those of
// the last flag writer: `cmp byte [0x7c2ea9], 0x80` when skipped, else the final `add esi, 0x41c`.
// FUNCTION: GP4 0x0051734e
void __cdecl FUN_0051734e(gp4::Regs* r) {
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    uint32_t arith = gp4::flags::sub8(mode, 0x80u);
    if (mode == 0x80u) {
        uint8_t* car = GP4_ARRAY(uint8_t, 0x0066fef0);
        for (uint32_t k = 22u; k != 0u; --k) {
            car[0x83] = static_cast<uint8_t>(car[0x83] & 0xf7u);
            arith = gp4::flags::add32(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car)), 0x41cu);
            car += 0x41c;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0051734e, FUN_0051734e, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=carbg_0066fef0+bits_005174cb")
