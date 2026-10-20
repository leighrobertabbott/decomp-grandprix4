#include <gp4/gp4.h>

// Keeps the compiler from folding state-relative offsets into absolute globals.
static __declspec(noinline) uint8_t* StateBase() { return GP4_ARRAY(uint8_t, 0x006380d8); }

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `add left, right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// The signed 64-bit product of two dwords: low half shifted right by n in eax (imul + shrd eax,
// edx, n), high half in edx.
struct Product {
    uint32_t shifted;
    uint32_t high;
};
static inline Product multiply(uint32_t a, uint32_t b, int n) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(a)) * static_cast<int64_t>(static_cast<int32_t>(b));
    Product result;
    result.shifted = static_cast<uint32_t>(static_cast<uint64_t>(product) >> n);
    result.high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    return result;
}

// After 0x00416dec (which refreshes DAT_0062fba8/ac), stores into the four wheel records of the state
// at 0x006380d8 (+0x48, +0x348, +0x648, +0x948) the dwords +0x2ec..+0x2f8 of the esi buffer times
// the record's +0xe4 / +0x3e4 / +0x6e4 / +0x9e4 gains (>> 14) plus half of DAT_0062fba8 (first two) or
// DAT_0062fbac (last two), negated when the record's +0x1c / +0x31c / +0x61c / +0x91c is not negative.
// With DAT_00622424 set it then leaves DAT_00637fd4 = the four values + DAT_0062fbac + DAT_0062fba8;
// otherwise with DAT_00622485 set it adds the scaled word +0xb2 (zero for the flagged cases) times the
// +0x198 / +0x798 gains to those four records. ebx ends as 0x006380d8, ebp as the last half or
// scaled value, eax / edx as the last value and product high half; the flags are the last add (or
// the compare of DAT_00622485 with zero).
// FUNCTION: GP4 0x00417066
void __cdecl FUN_00417066(gp4::Regs* r) {
    uint8_t* const buffer = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* const state = StateBase();

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00416dec, c);

    uint32_t half = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x0062fba8) >> 1);
    Product value = multiply(GP4_FIELD(uint32_t, buffer, 0x2ec), GP4_FIELD(uint32_t, state, 0xe4), 14);
    uint32_t result = value.shifted + half;
    if (GP4_FIELD(int32_t, state, 0x1c) >= 0) {
        result = 0u - result;
    }
    GP4_FIELD(uint32_t, state, 0x48) = result;

    value = multiply(GP4_FIELD(uint32_t, buffer, 0x2f0), GP4_FIELD(uint32_t, state, 0x3e4), 14);
    result = value.shifted + half;
    if (GP4_FIELD(int32_t, state, 0x31c) >= 0) {
        result = 0u - result;
    }
    GP4_FIELD(uint32_t, state, 0x348) = result;

    half = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x0062fbac) >> 1);
    value = multiply(GP4_FIELD(uint32_t, buffer, 0x2f4), GP4_FIELD(uint32_t, state, 0x6e4), 14);
    result = value.shifted + half;
    if (GP4_FIELD(int32_t, state, 0x61c) >= 0) {
        result = 0u - result;
    }
    GP4_FIELD(uint32_t, state, 0x648) = result;

    value = multiply(GP4_FIELD(uint32_t, buffer, 0x2f8), GP4_FIELD(uint32_t, state, 0x9e4), 14);
    result = value.shifted + half;
    if (GP4_FIELD(int32_t, state, 0x91c) >= 0) {
        result = 0u - result;
    }
    GP4_FIELD(uint32_t, state, 0x948) = result;

    uint32_t flags;
    if (GP4_GLOBAL(uint8_t, 0x00622424) != 0) {
        uint32_t sum = GP4_FIELD(uint32_t, state, 0x48);
        sum += GP4_FIELD(uint32_t, state, 0x348);
        sum += GP4_FIELD(uint32_t, state, 0x648);
        sum += GP4_FIELD(uint32_t, state, 0x948);
        sum += GP4_GLOBAL(uint32_t, 0x0062fbac);
        flags = addFlags32(sum, GP4_GLOBAL(uint32_t, 0x0062fba8));
        sum += GP4_GLOBAL(uint32_t, 0x0062fba8);
        GP4_GLOBAL(uint32_t, 0x00637fd4) = sum;
        result = sum;
    } else if (GP4_GLOBAL(uint8_t, 0x00622485) == 0) {
        flags = 0x44u;
    } else {
        value = multiply(GP4_FIELD(uint16_t, buffer, 0xb2), GP4_GLOBAL(uint32_t, 0x00637fa0), 11);
        result = value.shifted;
        if ((GP4_FIELD(uint8_t, buffer, 0x273) & 2u) != 0 || (GP4_FIELD(uint8_t, buffer, 0x162) & 1u) != 0 ||
            ((GP4_FIELD(uint32_t, buffer, 0x40) & 0x14280u) != 0 && GP4_FIELD(int16_t, buffer, 0x9a) < 0x1d5)) {
            result = 0;
        }
        half = result;
        value = multiply(result, GP4_FIELD(uint32_t, state, 0x198), 14);
        GP4_FIELD(uint32_t, state, 0x48) += value.shifted;
        GP4_FIELD(uint32_t, state, 0x348) += value.shifted;
        value = multiply(half, GP4_FIELD(uint32_t, state, 0x798), 14);
        GP4_FIELD(uint32_t, state, 0x648) += value.shifted;
        flags = addFlags32(GP4_FIELD(uint32_t, state, 0x948), value.shifted);
        GP4_FIELD(uint32_t, state, 0x948) += value.shifted;
        result = value.shifted;
    }

    r->eax = result;
    r->ebx = 0x006380d8u;
    r->edx = value.high;
    r->ebp = half;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00417066, FUN_00417066, "regs(esi:ptr[0x300]:bytes, ebx:u32, ebp:u32) -> (eax, ebx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00416dec+state_00416fce")
