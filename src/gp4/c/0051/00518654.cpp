#include <gp4/gp4.h>
#include <gp4/flags.h>

// Hand-written routine (pushal ... popal). Takes the channel k = dword 0x0088f998[dword 0x0088f6cc]
// (the sample -> channel map). When bit k of the mask 0x0088f700 is set, it starts the three ramps of
// channel k towards the new targets 0x0088f6d0 / 0x0088f6d4 / 0x0088f6d8 (stored as the targets
// 0x0088f7c8[k] / 0x0088f848[k] / 0x0088f808[k]); the step of each ramp (0x0088f888[k] /
// 0x0088f908[k] / 0x0088f8c8[k]) is ((target - current) * 4) / dword 0x0088f6dc, or 0 when the
// target equals the current value (0x0088f708[k] / 0x0088f788[k] / 0x0088f748[k]). Bit k of the
// active mask 0x0088f704 is set (bts) when some step was computed, else cleared (btr) - the mask
// 0x005188f5 walks to run the ramps. Every register is restored; the flags are those of `bt`
// (CF = 0) when the channel bit is clear, else of `cmp ebp, -1` with CF = the old bit of 0x0088f704.

static inline uint32_t* MaskWord(uint32_t* base, uint32_t bit) {
    return base + (static_cast<int32_t>(bit) >> 5);
}

static inline int32_t Step(int32_t delta) {
    return static_cast<int32_t>(static_cast<uint32_t>(delta) << 2) / GP4_GLOBAL(int32_t, 0x0088f6dc);
}

// FUNCTION: GP4 0x00518654
void __cdecl FUN_00518654(gp4::Regs* r) {
    const uint32_t k = GP4_ARRAY(uint32_t, 0x0088f998)[GP4_GLOBAL(uint32_t, 0x0088f6cc)];
    const uint32_t bit = 1u << (k & 31u);

    if ((*MaskWord(&GP4_ARRAY(uint32_t, 0x0088f700)[0], k) & bit) == 0u) {
        r->eflags &= ~static_cast<uint32_t>(gp4::flags::CF);
        return;
    }

    uint32_t started = 0u;
    int32_t target;
    int32_t delta;

    GP4_ARRAY(int32_t, 0x0088f888)[k] = 0;
    target = GP4_GLOBAL(int32_t, 0x0088f6d0);
    GP4_ARRAY(int32_t, 0x0088f7c8)[k] = target;
    delta = target - GP4_ARRAY(int32_t, 0x0088f708)[k];
    if (delta != 0) {
        started = 0xffffffffu;
        GP4_ARRAY(int32_t, 0x0088f888)[k] = Step(delta);
    }

    GP4_ARRAY(int32_t, 0x0088f908)[k] = 0;
    target = GP4_GLOBAL(int32_t, 0x0088f6d4);
    GP4_ARRAY(int32_t, 0x0088f848)[k] = target;
    delta = target - GP4_ARRAY(int32_t, 0x0088f788)[k];
    if (delta != 0) {
        started = 0xffffffffu;
        GP4_ARRAY(int32_t, 0x0088f908)[k] = Step(delta);
    }

    GP4_ARRAY(int32_t, 0x0088f8c8)[k] = 0;
    target = GP4_GLOBAL(int32_t, 0x0088f6d8);
    GP4_ARRAY(int32_t, 0x0088f808)[k] = target;
    delta = target - GP4_ARRAY(int32_t, 0x0088f748)[k];
    if (delta != 0) {
        started = 0xffffffffu;
        GP4_ARRAY(int32_t, 0x0088f8c8)[k] = Step(delta);
    }

    uint32_t* active = MaskWord(&GP4_ARRAY(uint32_t, 0x0088f704)[0], k);
    const uint32_t old_bit = (*active & bit) != 0u ? 1u : 0u;
    if (started == 0xffffffffu) {
        *active |= bit;
    } else {
        *active &= ~bit;
    }
    const uint32_t flags = (gp4::flags::sub32(started, 0xffffffffu) & ~static_cast<uint32_t>(gp4::flags::CF)) | old_bit;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00518654, FUN_00518654, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=chan_005188f5+chan_00518654")
