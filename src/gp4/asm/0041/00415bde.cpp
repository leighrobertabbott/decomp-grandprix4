#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `add left, right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

// imul (edx:eax = a * b) followed by shrd eax, edx, 14: the low dword of the product >> 14.
static inline uint32_t mulShift14(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// One draw of the random byte generator 0x00401462 (low byte of its eax).
static inline uint32_t drawByte(const gp4::Regs* r) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x00401462, call);
    return call.eax & 0xffu;
}

// Randomised steering value stored as a word at [esi+0x264] (clamped to 4000..DAT_00601314) and a
// randomised dword at [esi+0x260] (DAT_0066d814 plus a scaled random byte). When bit 6 of the byte
// [esi+0x232] is clear the first value blends DAT_00603a90/DAT_00603a94/DAT_00603a98 terms (ebp is
// left as 0x4000 - DAT_00603a98); otherwise it is DAT_00603ab0 * random + DAT_00603ab4. eax is the
// second scaled value, edx the high dword of its 64-bit product, the flags those of the final
// `add [esi+0x260], eax`.
// FUNCTION: GP4 0x00415bde
void __cdecl FUN_00415bde(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t value;

    if ((GP4_FIELD(uint8_t, record, 0x232) & 0x40u) != 0u) {
        const int32_t centred = static_cast<int32_t>((drawByte(r) << 6) - 0x2000u);
        value = mulShift14(centred, GP4_GLOBAL(int32_t, 0x00603ab0)) + GP4_GLOBAL(uint32_t, 0x00603ab4);
    } else {
        const int32_t centred = static_cast<int32_t>((drawByte(r) << 6) - 0x2000u);
        const uint32_t first = mulShift14(centred, GP4_GLOBAL(int32_t, 0x00603a94));
        const int32_t complement = static_cast<int32_t>(0x4000u - GP4_GLOBAL(uint32_t, 0x00603a98));
        r->ebp = static_cast<uint32_t>(complement);
        const int32_t mean = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00603a04) + GP4_GLOBAL(uint32_t, 0x00603a00)) >> 1;
        const uint32_t second = mulShift14(mean, GP4_GLOBAL(int32_t, 0x00603a98));
        const uint32_t third = mulShift14(GP4_GLOBAL(int32_t, 0x00603a90), complement);
        value = third + second + first;
    }

    int32_t clamped = static_cast<int32_t>(value);
    if (clamped < 4000) {
        clamped = 4000;
    }
    if (clamped > GP4_GLOBAL(int32_t, 0x00601314)) {
        clamped = GP4_GLOBAL(int32_t, 0x00601314);
    }
    GP4_FIELD(uint16_t, record, 0x264) = static_cast<uint16_t>(clamped);

    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(drawByte(r) << 6)) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603a88));
    const uint32_t scaled = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14) + GP4_GLOBAL(uint32_t, 0x00603a8c);
    const uint32_t base = GP4_GLOBAL(uint32_t, 0x0066d814);
    GP4_FIELD(uint32_t, record, 0x260) = base + scaled;

    r->eax = scaled;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(base, scaled);
}
GP4_IMPL(0x00415bde, FUN_00415bde, "regs(ebp:u32, esi:ptr[0x26c]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0066d8c0")
