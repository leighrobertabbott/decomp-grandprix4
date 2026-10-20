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

// Flags of the 8-bit `cmp left, right`.
static inline uint32_t subFlags8(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | (result & 0x80u)
        | (((left ^ right) & (left ^ result) & 0x80u) != 0u ? 0x800u : 0u);
}

// imul + shrd eax, edx, n: the low 32 bits of the signed 64-bit product shifted right by n.
static inline uint32_t mulShr(int32_t a, int32_t b, int n) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> n);
}

// FUNCTION: GP4 0x004205a0
// Sums the four wheel-record dwords +0x38 of the 0x300-byte records at 0x006380d8 with four
// scalars; when DAT_00601ea8 is set, each record's +0x14 becomes ([+0xbc] * [0x00601e34]) >> 14
// (and +0x178 is reduced by it times the car's scaled word +0x9a, floored at 0), minus the
// [0x00601de4]-scaled +0xc4 when +0x2c4 is nonzero and, while DAT_00601f1d is 0xff, minus the
// [0x00601de8]-scaled +0xc4 when byte +0x2e has bit 5; the four +0x14 join the sum (else they are
// cleared). ((sum << 3) << 8) / max(car word +0xb2 - [0x00601398], 0x200) + [0x00637f9c], clamped in
// magnitude to [0x0062249d], goes to 0x00637f78; while DAT_00629bd0 is 0xff the helpers
// 0x0041fdd5, 0x00420395 and 0x0041f2f1 follow. ebx ends as 0x006380d8.
void __cdecl FUN_004205a0(gp4::Regs* r) {
    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* const state = StateBase();

    int32_t divisor = static_cast<int32_t>(GP4_FIELD(int16_t, car, 0xb2)) - GP4_GLOBAL(int32_t, 0x00601398);
    if (divisor < 0x200) {
        divisor = 0x200;
    }

    uint32_t sum = GP4_FIELD(uint32_t, state, 0x338) + GP4_FIELD(uint32_t, state, 0x38)
        + GP4_FIELD(uint32_t, state, 0x638) + GP4_FIELD(uint32_t, state, 0x938)
        + GP4_GLOBAL(uint32_t, 0x00637fb4) + GP4_GLOBAL(uint32_t, 0x00637fc0)
        + GP4_GLOBAL(uint32_t, 0x00637fa8) + GP4_GLOBAL(uint32_t, 0x00607b04);

    uint32_t ebp = r->ebp;
    if (GP4_GLOBAL(uint8_t, 0x00601ea8) != 0u) {
        ebp = mulShr(GP4_FIELD(int16_t, car, 0x9a), GP4_GLOBAL(int32_t, 0x00601e38), 0xe);
        for (int k = 0; k < 4; ++k) {
            uint8_t* const rec = state + k * 0x300;
            const uint32_t force = mulShr(GP4_FIELD(int32_t, rec, 0xbc), GP4_GLOBAL(int32_t, 0x00601e34), 0xe);
            GP4_FIELD(uint32_t, rec, 0x14) = force;
            int32_t left = static_cast<int32_t>(GP4_FIELD(uint32_t, rec, 0x178)
                - mulShr(static_cast<int32_t>(force), static_cast<int32_t>(ebp), 0xe));
            if (left < 0) {
                left = 0;
            }
            GP4_FIELD(int32_t, rec, 0x178) = left;
        }
        for (int k = 0; k < 4; ++k) {
            uint8_t* const rec = state + k * 0x300;
            if (GP4_FIELD(uint32_t, rec, 0x2c4) != 0u) {
                GP4_FIELD(uint32_t, rec, 0x14) -= mulShr(GP4_FIELD(int32_t, rec, 0xc4), GP4_GLOBAL(int32_t, 0x00601de4), 0xa);
            }
        }
        if (GP4_GLOBAL(uint8_t, 0x00601f1d) == 0xffu) {
            for (int k = 0; k < 4; ++k) {
                uint8_t* const rec = state + k * 0x300;
                if ((GP4_FIELD(uint8_t, rec, 0x2e) & 0x20u) != 0u) {
                    GP4_FIELD(uint32_t, rec, 0x14) -= mulShr(GP4_FIELD(int32_t, rec, 0xc4), GP4_GLOBAL(int32_t, 0x00601de8), 0xa);
                }
            }
        }
        sum += GP4_FIELD(uint32_t, state, 0x14) + GP4_FIELD(uint32_t, state, 0x314)
            + GP4_FIELD(uint32_t, state, 0x614) + GP4_FIELD(uint32_t, state, 0x914);
    } else {
        GP4_FIELD(uint32_t, state, 0x14) = 0;
        GP4_FIELD(uint32_t, state, 0x314) = 0;
        GP4_FIELD(uint32_t, state, 0x614) = 0;
        GP4_FIELD(uint32_t, state, 0x914) = 0;
    }

    const int64_t dividend = static_cast<int64_t>(static_cast<int32_t>(sum << 3)) * 256;
    int32_t remainder;
    const int32_t quotient = gp4::idiv64(dividend, divisor, &remainder);
    uint32_t value = static_cast<uint32_t>(quotient) + GP4_GLOBAL(uint32_t, 0x00637f9c);
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x0062249d);
    if (static_cast<int32_t>(value) < 0) {
        uint32_t magnitude = 0u - value;
        if (magnitude >= limit) {
            magnitude = limit;
        }
        value = 0u - magnitude;
    } else if (value >= limit) {
        value = limit;
    }
    GP4_GLOBAL(uint32_t, 0x00637f78) = value;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = value;
    c.ebx = reinterpret_cast<uint32_t>(state);
    c.ecx = static_cast<uint32_t>(divisor);
    c.edx = static_cast<uint32_t>(remainder);
    c.ebp = ebp;
    const uint32_t mode = GP4_GLOBAL(uint8_t, 0x00629bd0);
    c.eflags = (c.eflags & ~0x8d5u) | subFlags8(mode, 0xffu);
    if (mode == 0xffu) {
        gp4::call_regs(0x0041fdd5, c);
        gp4::call_regs(0x00420395, c);
        gp4::call_regs(0x0041f2f1, c);
    }

    r->eax = c.eax;
    r->ebx = reinterpret_cast<uint32_t>(state);
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x004205a0, FUN_004205a0, "regs(esi:ptr[0xc0]:bytes) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=recs_006380d8+state_004178e0+integ_004200a7+state_0041f489+spin_0041f28d+wheel_004205a0")
