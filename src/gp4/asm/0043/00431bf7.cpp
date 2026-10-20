#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 8-bit `cmp left, right`.
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    left &= 0xffu;
    right &= 0xffu;
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | (result & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

static inline int32_t clampInt(int32_t value, int32_t lo, int32_t hi) {
    if (value > hi) {
        value = hi;
    }
    if (value < lo) {
        value = lo;
    }
    return value;
}

// FUNCTION: GP4 0x00431bf7
// Driver-assist step of the car at esi (pushal/popal: every register is preserved, the flags are those of the
// last flag writer). eax < 0 steps down, otherwise up; edx selects what moves: 1 = the word +0x60 by +-0x40
// (up to 0x18c0, 0x431b00 then 0x431be8 and the larger of the result and the word +0x1e0 is stored back),
// 2 = the level byte +0x7f (up: max(level, 1) + 1 below 6; down: level - 1, replaced by 0x43c475's choice
// below 2) followed by the stage bytes of 0x66d936 chosen through 0x4411f7 for the car's slot, any other
// value = the 0..3 offset (+0x198 - +0x136 +- 1) via 0x431a5a.
void __cdecl FUN_00431bf7(gp4::Regs* r) {
    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs st = *r;
    st.st_in = 0;
    st.st_out = 0;

    const bool down = static_cast<int32_t>(r->eax) < 0;
    const uint32_t mode = r->edx;
    uint32_t flags = 0u;
    bool setStage = false;      // fall into the 0x66d936 update (0x431cf5)
    bool finished = false;      // jumped straight to popal

    if (mode == 2u) {
        if (!down) {
            int8_t level = static_cast<int8_t>(car[0x7f]);
            if (level < 2) {
                level = 1;
            }
            level = static_cast<int8_t>(level + 1);
            st.eax = (st.eax & ~0xffu) | static_cast<uint8_t>(level);
            if (level >= 6) {
                flags = cmpFlags8(static_cast<uint8_t>(level), 6u);
                finished = true;
            } else {
                car[0x7f] = static_cast<uint8_t>(level);
                setStage = true;
            }
        } else {
            int8_t level = static_cast<int8_t>(car[0x7f]);
            st.eax = (st.eax & ~0xffu) | static_cast<uint8_t>(level);
            if (level < 2) {
                flags = cmpFlags8(static_cast<uint8_t>(level), 2u);
                finished = true;
            } else {
                level = static_cast<int8_t>(level - 1);
                st.eax = (st.eax & ~0xffu) | static_cast<uint8_t>(level);
                if (level < 2) {
                    gp4::call_regs(0x0043c475, st);
                    level = static_cast<int8_t>(st.eax);
                }
                car[0x7f] = static_cast<uint8_t>(level);
                setStage = true;
            }
        }
    } else {
        if (mode == 1u) {
            const uint32_t word60 = GP4_FIELD(uint16_t, car, 0x60);
            if (!down) {
                int32_t value = static_cast<int32_t>(word60 + 0x40u);
                if (value > 0x18c0) {
                    value = 0x18c0;
                }
                st.eax = static_cast<uint32_t>(value);
            } else {
                int32_t value = static_cast<int32_t>(word60) - 0x40;
                if (value < 0) {
                    value = 0;
                }
                st.eax = static_cast<uint32_t>(value);
            }
            gp4::call_regs(0x00431b00, st);
        } else {
            const int32_t high = GP4_FIELD(uint8_t, car, 0x198);
            const int32_t low = GP4_FIELD(uint8_t, car, 0x136);
            st.edx = static_cast<uint32_t>(low);
            const int32_t step = down ? (high - low - 1) : (high - low + 1);
            st.eax = static_cast<uint32_t>(clampInt(step, 0, 3));
            gp4::call_regs(0x00431a5a, st);
        }
        st.edx = GP4_FIELD(uint16_t, car, 0x1e0);
        if (static_cast<int32_t>(st.eax) < static_cast<int32_t>(st.edx)) {
            st.eax = st.edx;
        }
        gp4::call_regs(0x00431be8, st);
        GP4_FIELD(uint16_t, car, 0x60) = static_cast<uint16_t>(st.eax);
        flags = st.eflags & 0x8d5u;
        finished = true;
    }

    if (setStage) {
        uint32_t slot = static_cast<uint32_t>(static_cast<uint8_t>(car[0x7c] - 1u)) & 0x3fu;
        if (static_cast<int32_t>(slot) > 0x15) {
            slot = 0x15;
        }
        st.eax = slot;
        st.edx = GP4_ARRAY(uint32_t, 0x006f2cc4)[car[0x7f]];
        gp4::call_regs(0x004411f7, st);
        GP4_ARRAY(uint8_t, 0x0066d936)[st.eax] = static_cast<uint8_t>(st.edx);
        flags = st.eflags & 0x8d5u;
    }
    (void)finished;

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00431bf7, FUN_00431bf7, "regs(eax:i32[-3..3], edx:u32[0..3], esi:ptr[0x1e8]:layout=lvl_00431d1f) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=counter_0040f0dd+tbl_0066d94c+levels_006f2cc4+trk_007ad894+circ_007adb50")
