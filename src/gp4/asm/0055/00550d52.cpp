#include <gp4/gp4.h>
#include <gp4/flags.h>

static inline int32_t signedWord(const uint8_t* record, uint32_t offset) {
    return static_cast<int16_t>(*reinterpret_cast<const uint16_t*>(record + offset));
}

static inline int32_t signedByte(const uint8_t* record, uint32_t offset) {
    return static_cast<int8_t>(record[offset]);
}

// Track-segment record at edi, scaled by the lateral/forward step table of the drawing code. The
// selector word [0x00934df8] (0x28 or 0x29 in the callers) indexes a byte of the record, which plus
// the word [0x006f3db8] (16-bit add) is the signed multiplier cx. Two products are formed as
//   (signed byte +0x13 + (word (+0xe, or +0x4e unless the selector is 0x29) >> 6)) * cx   -> ebx = low << 1
//   (signed byte +0x11 + (word (+0xc, or +0x4c unless the selector is 0x29) >> 6)) * cx   -> eax = low << 1,
// edx = high dword of the second product. The signed words +4 and +8 shifted left by 6 are stored in
// 0x00934df0 and 0x00934df4 and the flags are those of the last `shl ecx, 6` (word +8). ecx is
// preserved.
// FUNCTION: GP4 0x00550d52
void __cdecl FUN_00550d52(gp4::Regs* r) {
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->edi));

    const int32_t selector = static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x00934df8));
    const uint16_t sum = static_cast<uint16_t>(static_cast<uint32_t>(record[selector])
                                                + GP4_GLOBAL(uint16_t, 0x006f3db8));
    const int32_t factor = static_cast<int16_t>(sum);

    int32_t lateral = signedWord(record, 0xe);
    if (selector != 0x29) {
        lateral = signedWord(record, 0x4e);
    }
    const int32_t first = signedByte(record, 0x13) + (lateral >> 6);
    const int64_t firstProduct = static_cast<int64_t>(first) * factor;

    int32_t forward = signedWord(record, 0xc);
    if (selector != 0x29) {
        forward = signedWord(record, 0x4c);
    }
    const int32_t second = signedByte(record, 0x11) + (forward >> 6);
    const int64_t secondProduct = static_cast<int64_t>(second) * factor;

    GP4_GLOBAL(uint32_t, 0x00934df0) = static_cast<uint32_t>(signedWord(record, 4)) << 6;
    const uint32_t lastValue = static_cast<uint32_t>(signedWord(record, 8));
    const uint32_t lastShifted = lastValue << 6;
    GP4_GLOBAL(uint32_t, 0x00934df4) = lastShifted;

    // shl ecx, 6: CF = last bit shifted out, AF = 0, OF = bit31((value << 5) ^ result) (pinned model).
    const uint32_t shiftFlags = ((lastValue >> 26) & 1u)
        | gp4::flags::szp(lastShifted, 32)
        | ((((lastValue << 5) ^ lastShifted) & 0x80000000u) != 0u ? static_cast<uint32_t>(gp4::flags::OF) : 0u);

    r->ebx = static_cast<uint32_t>(firstProduct) << 1;
    r->eax = static_cast<uint32_t>(secondProduct) << 1;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(secondProduct) >> 32);
    r->eflags = gp4::flags::merge(r->eflags, shiftFlags);
}
GP4_IMPL(0x00550d52, FUN_00550d52, "regs(edi:rec[0x8943a0,0x160,0..33]) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=seg_00550d52")
