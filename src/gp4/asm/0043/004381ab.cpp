#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Q14 cosine of a 16-bit angle through the word table at 0x7c5c5c (|angle| >> 3 indexes it).
static inline int32_t cosQ14(uint16_t angle) {
    if (static_cast<int16_t>(angle) < 0) {
        angle = static_cast<uint16_t>(0u - angle);
    }
    return GP4_ARRAY(int16_t, 0x007c5c5c)[angle >> 3];
}

// imul; shrd eax, edx, 14
static inline uint32_t mulQ14(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * b;
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// pushal / popal: every general register is preserved; the carry is the clc / stc epilogue and the
// other flags are those of the last flag-writing instruction.
static inline void finish(gp4::Regs* r, uint32_t flags, bool carry) {
    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~0x01u) | (carry ? 0x01u : 0u);
}

// FUNCTION: GP4 0x004381ab
// Projects a point at distance [0x629f24] from the car at esi in the direction heading (word +0xe)
// + word [0x629f1c + 2*eax] (stored at 0x629ea4 / 0x629ea8, as x / z with the car's +0 / +4), clears
// the bytes 0x627944 / 0x627945, sets 0x627947 = 0xff and settles the point on the track ring with
// 0x00406dbb starting at the car's segment [esi+0x1c]. CF = 1 (and the 0x629eac..0x629ebc copy of the
// set of five dwords 0x629edc / 0x629ee4 / 0x629eec / 0x629ef4 / 0x629efc, element 1 when the found
// segment is at or above [0x891500], else element 0) when the lookup moved by at least 2 and the
// segment's flag +0x2c has bit 0x20 (moved backwards) or 0x10 (forwards); otherwise CF = 0.
void __cdecl FUN_004381ab(gp4::Regs* r) {
    const uint32_t index = r->eax;
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const int32_t radius = GP4_GLOBAL(int32_t, 0x00629f24);
    const uint16_t angle = static_cast<uint16_t>(GP4_ARRAY(uint16_t, 0x00629f1c)[index] + GP4_FIELD(uint16_t, car, 0x0e));

    const uint32_t x = mulQ14(cosQ14(static_cast<uint16_t>(0x4000u - angle)), radius) + GP4_FIELD(uint32_t, car, 0x00);
    GP4_GLOBAL(uint32_t, 0x00629ea4) = x;
    const uint32_t z = mulQ14(cosQ14(angle), radius) + GP4_FIELD(uint32_t, car, 0x04);
    GP4_GLOBAL(uint32_t, 0x00629ea8) = z;

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    s.ebx = index;
    s.edi = GP4_FIELD(uint32_t, car, 0x1c);
    s.eax = GP4_GLOBAL(uint32_t, 0x00629ea4);
    s.edx = GP4_GLOBAL(uint32_t, 0x00629ea8);
    GP4_GLOBAL(uint8_t, 0x00627945) = 0;
    GP4_GLOBAL(uint8_t, 0x00627944) = 0;
    GP4_GLOBAL(uint8_t, 0x00627947) = 0xff;
    gp4::call_regs(0x00406dbb, s);

    uint8_t* segment = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(s.edi));
    GP4_GLOBAL(uint32_t, 0x00629ec0) = s.edi;
    const int32_t moved = static_cast<int32_t>(s.eax);
    const int32_t distance = moved < 0 ? -moved : moved;
    if (distance < 2) {
        finish(r, cmpFlags32(static_cast<uint32_t>(distance), 2u), false);
        return;
    }
    const uint32_t direction = moved < 0 ? 0x20u : 0x10u;
    if ((GP4_FIELD(uint32_t, segment, 0x2c) & direction) == 0u) {
        // test dword [edi+0x2c], mask: zero result -> ZF = PF = 1
        finish(r, 0x44u, false);
        return;
    }

    const uint32_t pitStart = GP4_GLOBAL(uint32_t, 0x00891500);
    const uint32_t slot = s.edi < pitStart ? 0u : 1u;
    const uint32_t flags = cmpFlags32(s.edi, pitStart);
    GP4_GLOBAL(uint32_t, 0x00629eac) = GP4_ARRAY(uint32_t, 0x00629edc)[slot];
    GP4_GLOBAL(uint32_t, 0x00629eb0) = GP4_ARRAY(uint32_t, 0x00629ee4)[slot];
    GP4_GLOBAL(uint32_t, 0x00629eb4) = GP4_ARRAY(uint32_t, 0x00629eec)[slot];
    GP4_GLOBAL(uint32_t, 0x00629eb8) = GP4_ARRAY(uint32_t, 0x00629ef4)[slot];
    GP4_GLOBAL(uint32_t, 0x00629ebc) = GP4_ARRAY(uint32_t, 0x00629efc)[slot];
    finish(r, flags, true);
}
GP4_IMPL(0x004381ab, FUN_004381ab, "regs(eax:u32[0..3], esi:u32[0x66fef0..0x66fef0]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_004317ae+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+segflag_004381ab+state_004381ab; cases=600")
