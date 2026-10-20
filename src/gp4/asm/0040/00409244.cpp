#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags left by `test word, 0xffff`: CF = OF = AF = 0, PF/ZF/SF from the word.
static inline uint32_t logicFlags16(uint16_t value) {
    return parityFlag(value)
        | (value == 0u ? 0x40u : 0u)
        | ((value & 0x8000u) != 0u ? 0x80u : 0u);
}

// Derives the values stored in the esi record from the parameter block at edi and the word
// [esi+0x90] (x): [esi+0x96] = (y * x >> 15) + [esi+0x92] with y = word [edi+0x14] (0 when y is 0),
// [esi+0x146] = max(0x400 - (x * y >> 14), 0x10), [esi+0x66] = ((word [esi+0x96] - 0x400) << 14) /
// max(word [esi+0x146], 0x10) + 0x4000 clamped to +-0x6000, [esi+0x11c]/[esi+0x11e] = words
// [edi+0x60]/[edi+0x62] - 0x100 and [esi+0xb0] = [esi+0x11e], or [esi+0x11c] when x is negative.
// Leaves eax = (high word of the clamped +0x66 value : word stored at +0xb0), ecx = the divisor,
// edx = the remainder; the flags are those of `test word [esi+0x90], 0xffff`.
// FUNCTION: GP4 0x00409244
void __cdecl FUN_00409244(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* block = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    const int16_t y = GP4_FIELD(int16_t, block, 0x14);
    const int16_t x = GP4_FIELD(int16_t, record, 0x90);

    uint16_t scaled = 0;
    if (y != 0) {
        const int32_t product = static_cast<int32_t>(y) * static_cast<int32_t>(x);
        scaled = static_cast<uint16_t>(static_cast<uint32_t>(product) >> 15);
    }
    GP4_FIELD(uint16_t, record, 0x96) = static_cast<uint16_t>(scaled + GP4_FIELD(uint16_t, record, 0x92));

    const int64_t wide = static_cast<int64_t>(x) * static_cast<int64_t>(y);
    int32_t reduction = static_cast<int32_t>(0x400u - static_cast<uint32_t>(static_cast<uint64_t>(wide) >> 14));
    if (reduction < 0x10) {
        reduction = 0x10;
    }
    GP4_FIELD(uint16_t, record, 0x146) = static_cast<uint16_t>(reduction);

    const int32_t numerator = static_cast<int32_t>(
        static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, record, 0x96)) - 0x400) << 14);
    int32_t divisor = static_cast<int32_t>(GP4_FIELD(uint16_t, record, 0x146));
    if (divisor < 0x10) {
        divisor = 0x10;
    }
    const int32_t quotient = numerator / divisor;
    const int32_t remainder = numerator % divisor;

    uint32_t result = static_cast<uint32_t>(quotient) + 0x4000u;
    if (static_cast<int32_t>(result) < 0) {
        uint32_t magnitude = 0u - result;
        if (magnitude >= 0x6000u) {
            magnitude = 0x6000u;
        }
        result = 0u - magnitude;
    } else if (result >= 0x6000u) {
        result = 0x6000u;
    }
    GP4_FIELD(uint16_t, record, 0x66) = static_cast<uint16_t>(result);

    GP4_FIELD(uint16_t, record, 0x11c) = static_cast<uint16_t>(GP4_FIELD(uint16_t, block, 0x60) - 0x100u);
    uint16_t last = static_cast<uint16_t>(GP4_FIELD(uint16_t, block, 0x62) - 0x100u);
    GP4_FIELD(uint16_t, record, 0x11e) = last;
    if (GP4_FIELD(int16_t, record, 0x90) < 0) {
        last = GP4_FIELD(uint16_t, record, 0x11c);
    }
    GP4_FIELD(uint16_t, record, 0xb0) = last;

    r->eax = (result & 0xffff0000u) | last;
    r->ecx = static_cast<uint32_t>(divisor);
    r->edx = static_cast<uint32_t>(remainder);
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags16(static_cast<uint16_t>(GP4_FIELD(int16_t, record, 0x90)));
}
GP4_IMPL(0x00409244, FUN_00409244, "regs(edi:ptr[0x6a]:bytes, esi:ptr[0x150]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of)")
