#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of "sar value, 8" in the oracle model: CF = last bit shifted out, AF = 0, OF = 0.
static inline uint32_t sarBy8Flags(int32_t value) {
    const uint32_t result = static_cast<uint32_t>(value >> 8);
    return ((static_cast<uint32_t>(value) >> 7) & 1u) | parityFlag(result)
        | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Steps the position DAT_00623c34 by a correction towards the target DAT_006258a8 (computed by
// 0x0041bce9), limited to DAT_00623c2c..DAT_00623c30, and stores position >> 8 as a word at +0x62
// of the record in esi. DAT_006258c8 is the pending offset (position - target); 0x0041bd28
// supplies a correction that is added with overflow protection to a non-zero offset (or plainly
// when DAT_006258dc is 0xff and the offset is zero). eax = position >> 8, edx and ebx are what the
// last callee left (or the xor tests of the overflow check); the flags are those of the final sar.
// FUNCTION: GP4 0x0041bd56
void __cdecl FUN_0041bd56(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::call_regs(0x0041bce9, *r);

    const int32_t lowLimit = GP4_GLOBAL(int32_t, 0x00623c2c);
    const uint32_t target = GP4_GLOBAL(uint32_t, 0x006258a8);

    if (GP4_GLOBAL(int32_t, 0x00623c34) < lowLimit) {
        GP4_GLOBAL(int32_t, 0x00623c34) = lowLimit;
        GP4_GLOBAL(uint32_t, 0x006258c8) = static_cast<uint32_t>(lowLimit) - target;
    }

    bool correct = false;
    if (GP4_GLOBAL(uint32_t, 0x006258c8) != 0u) {
        const uint32_t offset = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00623c34)) - target;
        GP4_GLOBAL(uint32_t, 0x006258c8) = offset;
        correct = offset != 0u;
    }

    if (correct) {
        uint32_t step = GP4_GLOBAL(uint32_t, 0x006258b4);
        if (static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x006258c8)) < 0) {
            step = 0u - step;
        }
        GP4_GLOBAL(uint32_t, 0x006258ac) = step;
        gp4::call_regs(0x0041bd28, *r);

        const uint32_t adjust = r->eax;
        if (adjust != 0u) {
            const uint32_t before = GP4_GLOBAL(uint32_t, 0x006258c8);
            r->edx = before ^ adjust;
            if (static_cast<int32_t>(r->edx) < 0) {
                // opposite signs: the sum cannot overflow, but a sign flip of the offset clears it
                const uint32_t after = before + adjust;
                GP4_GLOBAL(uint32_t, 0x006258c8) = after;
                r->edx = before ^ after;
                if (static_cast<int32_t>(r->edx) < 0) {
                    GP4_GLOBAL(uint32_t, 0x006258c8) = 0u;
                }
            } else {
                GP4_GLOBAL(uint32_t, 0x006258c8) = before + adjust;
            }
        }
    } else if (GP4_GLOBAL(uint8_t, 0x006258dc) == 0xffu) {
        gp4::call_regs(0x0041bd28, *r);
        GP4_GLOBAL(uint32_t, 0x006258c8) = GP4_GLOBAL(uint32_t, 0x006258c8) + r->eax;
    }

    int32_t position = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x006258c8) + GP4_GLOBAL(uint32_t, 0x006258a8));
    if (position < lowLimit) {
        position = lowLimit;
    }
    if (position > GP4_GLOBAL(int32_t, 0x00623c30)) {
        position = GP4_GLOBAL(int32_t, 0x00623c30);
    }
    GP4_GLOBAL(int32_t, 0x00623c34) = position;

    r->eax = static_cast<uint32_t>(position >> 8);
    GP4_FIELD(uint16_t, record, 0x62) = static_cast<uint16_t>(r->eax);
    r->eflags = (r->eflags & ~0x8d5u) | sarBy8Flags(position);
}
GP4_IMPL(0x0041bd56, FUN_0041bd56, "regs(esi:ptr[0x6a]:bytes) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=layout=state_0041bce9+state_0041bd28+state_0041bd56")
