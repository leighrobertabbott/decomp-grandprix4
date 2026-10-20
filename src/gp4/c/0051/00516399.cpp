#include <gp4/gp4.h>
#include <gp4/flags.h>

// Latches a timing readout of the selected car record [0x679d30] into
// 0xa6987c (kind 0..3), 0xa69878 (value) and 0xa69880 (difference) for the
// caller 0x4c4740. Record byte +0x82 bit 3 enables it (and the mode byte
// 0x7c2ea9 must not be 0x80); bit 4 selects the split indexed by byte +0x74
// (dwords +0x20[i] against +0x2c[i], 0x20000000 / 0x10000000 = unset), else the
// +0x28 / +0x38 pair. pushal/popal keep every register; popal does not restore
// flags, so the flags are those of the last flag writer on each path.

// FUNCTION: GP4 0x00516399
void __cdecl FUN_00516399(gp4::Regs* r) {
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    uint32_t flags = gp4::flags::sub8(mode, 0x80u);
    if (mode == 0x80) {
        GP4_GLOBAL(uint32_t, 0x00a6987c) = 0;
        r->eflags = gp4::flags::merge(r->eflags, flags);
        return;
    }
    const uint8_t* rec = (const uint8_t*)GP4_GLOBAL(uint32_t, 0x00679d30);
    const uint8_t b82 = rec[0x82];
    flags = gp4::flags::logic8(b82 & 8u);
    if ((b82 & 8u) == 0) {
        GP4_GLOBAL(uint32_t, 0x00a6987c) = 0;
        r->eflags = gp4::flags::merge(r->eflags, flags);
        return;
    }
    flags = gp4::flags::logic8(b82 & 0x10u);
    if ((b82 & 0x10u) == 0) {
        GP4_GLOBAL(uint32_t, 0x00a6987c) = 1;
        const uint32_t v28 = *(const uint32_t*)(rec + 0x28);
        GP4_GLOBAL(uint32_t, 0x00a69878) = v28;
        flags = gp4::flags::sub8(mode, 0u);
        if (mode == 0) {
            const uint32_t v38 = *(const uint32_t*)(rec + 0x38);
            flags = gp4::flags::sub32(v38, 0x10000000u);
            if (v38 != 0x10000000u) {
                GP4_GLOBAL(uint32_t, 0x00a6987c) = 3;
                flags = gp4::flags::sub32(v28, v38);
                GP4_GLOBAL(uint32_t, 0x00a69880) = v28 - v38;
            }
        }
    } else {
        GP4_GLOBAL(uint32_t, 0x00a6987c) = 2;
        const uint32_t idx = rec[0x74];
        const uint32_t* cur = (const uint32_t*)(rec + 0x20);
        const uint32_t* best = (const uint32_t*)(rec + 0x2c);
        GP4_GLOBAL(uint32_t, 0x00a69878) = cur[idx];
        const uint32_t b = best[idx];
        flags = gp4::flags::sub32(b, 0x20000000u);
        if (b != 0x20000000u) {
            GP4_GLOBAL(uint32_t, 0x00a6987c) = 3;
            GP4_GLOBAL(uint32_t, 0x00893cec) = b;
            const uint32_t c = cur[idx];
            flags = gp4::flags::sub32(c, 0x10000000u);
            if (c == 0x10000000u) {
                GP4_GLOBAL(uint32_t, 0x00a69880) = b;
            } else {
                flags = gp4::flags::sub32(c, b);
                GP4_GLOBAL(uint32_t, 0x00a69880) = c - b;
            }
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00516399, FUN_00516399, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0044334d+cars_0042504d+sel_0051634b+lap_00516399")
