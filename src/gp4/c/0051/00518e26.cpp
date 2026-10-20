#include <gp4/gp4.h>
#include <gp4/flags.h>

// Byte `index` of the per-sample flag table at `base` (a helper so the table base 0x88fb28 stays the
// referenced global, as in `test byte [eax+0x88fb28], 0x80` with eax = 5).
static __declspec(noinline) uint8_t ByteAt(const uint8_t* base, uint32_t index) {
    return base[index];
}

// The third ramp target: 0xa000 whichever way the mode byte (0x7ab185) compares with 0xff.
static __declspec(noinline) uint32_t ThirdTarget(const uint8_t* mode) {
    uint32_t target = 0xa000u;
    if (*mode != 0xffu) {
        target = 0xa000u;
    }
    return target;
}

// Sound level step for the record at ebx: with the mode byte 0x66d7a4 == 0x80 or a zero count
// [ebx+0x88] it switches the active byte 0x88fbbe off (nothing to do when already off) and takes
// level 0; otherwise it switches it on with level [0x88f6f0], and for a count of 3 or more the level
// is [0x88fc1c] - ([0x6f2e90] * [0x88fc00] >> 14), switched off at 0 when not positive. The level is
// stored to 0x88fc1c. Unless paused (0x852d6a), sample 5 ([0x88f6cc] = 5) is (re)started through
// 0x51f263 when it is loaded (0x51f27f) and either active or flagged (bit 7 of 0x88fb2d); when
// active its ramp targets 0x88f6d0/d4/d8 are 2 * level, 0x100000 and 0xa000. eax returns the level
// (unchanged on the idle exit), edx the high product dword when it was computed; the flags are those
// of the last compare/test or callee.
// FUNCTION: GP4 0x00518e26
void __cdecl FUN_00518e26(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint8_t& active = GP4_GLOBAL(uint8_t, 0x0088fbbe);
    uint32_t edx = r->edx;
    uint32_t level;

    if (GP4_GLOBAL(uint8_t, 0x0066d7a4) == 0x80u || GP4_FIELD(int32_t, record, 0x88) == 0) {
        if (active == 0u) {
            r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(0u, 0u));
            return;
        }
        active = 0;
        level = 0;
    } else {
        level = GP4_GLOBAL(uint32_t, 0x0088f6f0);
        active = 0xffu;
        if (GP4_FIELD(int32_t, record, 0x88) >= 3) {
            const int64_t product = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006f2e90)) *
                                    static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0088fc00));
            edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
            level = GP4_GLOBAL(uint32_t, 0x0088fc1c) - static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
            if (static_cast<int32_t>(level) <= 0) {
                level = 0;
                active = 0;
            }
        }
    }
    GP4_GLOBAL(uint32_t, 0x0088fc1c) = level;

    r->eax = level;
    r->edx = edx;

    const uint8_t paused = GP4_GLOBAL(uint8_t, 0x00852d6a);
    if (paused != 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(paused));
        return;
    }

    const uint32_t sample = 5u;
    GP4_GLOBAL(uint32_t, 0x0088f6cc) = sample;
    if (active == 0u) {
        const uint8_t bit = static_cast<uint8_t>(ByteAt(GP4_ARRAY(uint8_t, 0x0088fb28), sample) & 0x80u);
        if (bit == 0u) {
            r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(bit));
            return;
        }
    }

    gp4::Regs c = *r;
    c.eax = sample;
    gp4::call_regs(0x0051f27f, c);
    if ((c.eflags & gp4::flags::CF) == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, c.eflags);
        return;
    }

    GP4_GLOBAL(uint32_t, 0x0088fbac) = 0;
    const uint8_t state = active;
    GP4_GLOBAL(uint8_t, 0x0088f6e0) = state;
    c.eax = state;
    if (state == 0xffu) {
        GP4_GLOBAL(uint32_t, 0x0088f6d0) = GP4_GLOBAL(uint32_t, 0x0088fc1c) << 1;
        GP4_GLOBAL(uint32_t, 0x0088f6d4) = 0x100000u;
        c.eax = ThirdTarget(&GP4_GLOBAL(uint8_t, 0x007ab185));
        GP4_GLOBAL(uint32_t, 0x0088f6d8) = c.eax;
    }
    gp4::call_regs(0x0051f263, c);

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00518e26, FUN_00518e26, "regs(eax:u32, edx:u32, ebx:ptr[0x90]:i32[-1..4]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=full_00518b09+chan_005188f5+chan_00518654+snd_00518758+sndrec_0051a78d+snd_0051ace6+sndgate_0051a78d+mode_00550d0b")
