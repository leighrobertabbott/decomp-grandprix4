#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of `test word, 0xffff`: CF = OF = AF = 0.
static inline uint32_t testFlags16(uint32_t value) {
    value &= 0xffffu;
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value >> 8) & 0x80u);
}

// Flags of SHL r32, count (count > 1) in the pinned oracle model: CF = last bit out, AF = 0,
// OF = bit31(value << (count - 1) ^ result).
static inline uint32_t shlFlags32(uint32_t value, uint32_t count) {
    const uint32_t result = value << count;
    const uint32_t previous = value << (count - 1);
    return (previous >> 31)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | (((previous ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// cmp eax, 0 ; jge ; mov eax, 0
static inline int32_t clampNonNegative(int32_t value) {
    return value < 0 ? 0 : value;
}

// The 0x300-byte wheel records at 0x006380d8 (ebx), reached by displacement.
static __declspec(noinline) void setFrontSlots(uint8_t* records, int32_t scaled) {
    GP4_FIELD(int32_t, records, 0x8c4) = scaled;
    GP4_FIELD(int32_t, records, 0xbc4) = scaled;
    GP4_FIELD(int32_t, records, 0x8c8) = scaled;
    GP4_FIELD(int32_t, records, 0xbc8) = scaled;
}

static __declspec(noinline) void setRearSlots(uint8_t* records, int32_t scaled) {
    GP4_FIELD(int32_t, records, 0x2c4) = scaled;
    GP4_FIELD(int32_t, records, 0x5c4) = scaled;
    GP4_FIELD(int32_t, records, 0x2c8) = scaled;
    GP4_FIELD(int32_t, records, 0x5c8) = scaled;
}

// FUNCTION: GP4 0x0041062a
// esi = wheel body. After 0x00405893 the four dwords [0x0060309c..0x006030a8] are clamped at zero and
// stored at +0x2ec/+0x2f0/+0x2f4/+0x2f8 and +0x2bc/+0x2c0/+0x2c4/+0x2c8. When the word +0x27e is
// nonzero its value << 10 is copied to the four dwords at 0x6380d8 + 0x8c4/0xbc4/0x8c8/0xbc8 (with
// [0x00622450] = 1, and +0x2f4/+0x2f8 cleared), then to 0x6380d8 + 0x2c4/0x5c4/0x2c8/0x5c8 (with
// [0x0062244c] = 1, and +0x2ec/+0x2bc/+0x2f0/+0x2c0 cleared). ebx = 0x6380d8, eax = 0 when the word is
// nonzero (else the last clamped value). The flags are those of the final `test word` (word zero) or
// of the second `shl eax, 10`.
void __cdecl FUN_0041062a(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00405893, c);

    int32_t value = clampNonNegative(GP4_GLOBAL(int32_t, 0x0060309c));
    GP4_FIELD(int32_t, body, 0x2ec) = value;
    GP4_FIELD(int32_t, body, 0x2bc) = value;
    value = clampNonNegative(GP4_GLOBAL(int32_t, 0x006030a0));
    GP4_FIELD(int32_t, body, 0x2f0) = value;
    GP4_FIELD(int32_t, body, 0x2c0) = value;
    value = clampNonNegative(GP4_GLOBAL(int32_t, 0x006030a4));
    GP4_FIELD(int32_t, body, 0x2f4) = value;
    GP4_FIELD(int32_t, body, 0x2c4) = value;
    value = clampNonNegative(GP4_GLOBAL(int32_t, 0x006030a8));
    GP4_FIELD(int32_t, body, 0x2f8) = value;
    GP4_FIELD(int32_t, body, 0x2c8) = value;

    uint8_t* records = GP4_ARRAY(uint8_t, 0x006380d8);
    r->ebx = 0x006380d8u;

    if (GP4_FIELD(uint16_t, body, 0x27e) != 0u) {
        const uint32_t word = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, body, 0x27e)));
        setFrontSlots(records, static_cast<int32_t>(word << 10));
        GP4_GLOBAL(int32_t, 0x00622450) = 1;
        value = 0;
        GP4_FIELD(int32_t, body, 0x2f4) = value;
        GP4_FIELD(int32_t, body, 0x2f8) = value;
    }
    uint32_t flags = testFlags16(GP4_FIELD(uint16_t, body, 0x27e));
    if (GP4_FIELD(uint16_t, body, 0x27e) != 0u) {
        const uint32_t word = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, body, 0x27e)));
        setRearSlots(records, static_cast<int32_t>(word << 10));
        flags = shlFlags32(word, 10);
        GP4_GLOBAL(int32_t, 0x0062244c) = 1;
        value = 0;
        GP4_FIELD(int32_t, body, 0x2ec) = value;
        GP4_FIELD(int32_t, body, 0x2bc) = value;
        GP4_FIELD(int32_t, body, 0x2f0) = value;
        GP4_FIELD(int32_t, body, 0x2c0) = value;
    }

    r->eax = static_cast<uint32_t>(value);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041062a, FUN_0041062a, "regs(esi:ptr[0x300]:i16[-1..1], edi:rec[0x8943a0,0x160,0..25]) -> (eax, ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00405489+engine_00413606+ring_008943a0+bank_004279ab+state_0041062a")
