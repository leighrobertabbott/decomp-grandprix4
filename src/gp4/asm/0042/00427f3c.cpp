#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t compareFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Byte offset into the Q14 word table at 0x007c5c5c for a signed 16-bit angle: |angle| >> 2, even.
static inline int32_t tableValue(uint16_t angle) {
    if (static_cast<int16_t>(angle) < 0) {
        angle = static_cast<uint16_t>(0u - angle);
    }
    const uint32_t offset = static_cast<uint32_t>(angle >> 2) & 0xfffeu;
    return GP4_FIELD(int16_t, GP4_ARRAY(uint8_t, 0x007c5c5c), offset);
}

// imul ; shrd eax, edx, 14: low dword of (a * b) >> 14.
static inline uint32_t mulShr14(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// One step forward along the segment ring: past the lap end [0x866ecc] a record below the pit start
// [0x891500] wraps back by [0x866ec4], one at or past the pit end [0x866ef8] clamps to the last pit record.
static inline uint32_t nextSegment(uint32_t segment) {
    segment += 0x160u;
    if (segment >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
        if (segment < GP4_GLOBAL(uint32_t, 0x00891500)) {
            segment -= GP4_GLOBAL(uint32_t, 0x00866ec4);
        } else if (segment >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
            segment = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
        }
    }
    return segment;
}

// edi = track segment, esi = vector (word +0x18 = heading). First records in 0x624820 the lowest word
// +0x16 (from 0x1000) of the non-increasing run of up to [0x62481c] segments ahead. Then steps along
// the ring while 0x427e7c reports a negative heading error (at most [0x624818] steps, else 0); at the
// sign change it interpolates the crossing point (0x624834/0x624838) between the last two offset
// vectors, takes its length (0x4010f3) and returns min(|cos(h) << 10 / max(|len * sin(h) >> 14|, 0x100)|,
// 0x10000) with h = (word [edi] - word [esi+0x18]) / 2. With no negative step at all it returns 0.
// FUNCTION: GP4 0x00427f3c
void __cdecl FUN_00427f3c(gp4::Regs* r) {
    const uint32_t start = r->edi;
    uint8_t* vector = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    uint32_t count = GP4_GLOBAL(uint32_t, 0x0062481c);
    int32_t lowest = 0x1000;
    uint32_t segment = start;
    do {
        const int32_t height = GP4_FIELD(int16_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(segment)), 0x16);
        if (height > lowest) {
            break;
        }
        lowest = height;
        segment = nextSegment(segment);
    } while (--count != 0u);
    GP4_GLOBAL(int32_t, 0x00624820) = lowest;

    GP4_GLOBAL(uint32_t, 0x00624810) = 0;
    segment = start;
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    int32_t heading;
    for (;;) {
        call.edi = segment;
        gp4::call_regs(0x00427e7c, call);
        heading = static_cast<int32_t>(call.eax);
        if (heading >= 0) {
            break;
        }
        GP4_GLOBAL(int32_t, 0x00624840) = heading;
        GP4_GLOBAL(uint32_t, 0x00624824) = GP4_GLOBAL(uint32_t, 0x00623e30);
        GP4_GLOBAL(uint32_t, 0x00624828) = GP4_GLOBAL(uint32_t, 0x00623e38);
        segment = nextSegment(segment);
        GP4_GLOBAL(uint32_t, 0x00624810) += 1u;
        if (GP4_GLOBAL(uint32_t, 0x00624810) == GP4_GLOBAL(uint32_t, 0x00624818)) {
            r->eax = 0;
            r->ecx = call.ecx;
            r->edx = call.edx;
            r->ebp = call.ebp;
            r->eflags = (r->eflags & ~0x8d5u) | 0x44u;   // cmp of equal values
            return;
        }
    }
    if (GP4_GLOBAL(uint32_t, 0x00624810) == 0u) {
        r->eax = 0;
        r->ecx = call.ecx;
        r->edx = call.edx;
        r->ebp = call.ebp;
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;   // test of a zero dword
        return;
    }

    GP4_GLOBAL(int32_t, 0x00624844) = heading;
    GP4_GLOBAL(uint32_t, 0x0062482c) = GP4_GLOBAL(uint32_t, 0x00623e30);
    GP4_GLOBAL(uint32_t, 0x00624830) = GP4_GLOBAL(uint32_t, 0x00623e38);
    const int32_t before = GP4_GLOBAL(int32_t, 0x00624840);
    int32_t span = static_cast<int32_t>(static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00624844)) - static_cast<uint32_t>(before));
    if (span == 0) {
        span = 1;
    }
    uint32_t magnitude = static_cast<uint32_t>(before);
    if (before < 0) {
        magnitude = 0u - magnitude;
    }
    int32_t remainder;
    const int32_t fraction = gp4::idiv64(static_cast<int64_t>(static_cast<int32_t>(magnitude << 14)), span, &remainder);

    const uint32_t x = mulShr14(static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0062482c) - GP4_GLOBAL(uint32_t, 0x00624824)), fraction)
        + GP4_GLOBAL(uint32_t, 0x00624824);
    GP4_GLOBAL(uint32_t, 0x00624834) = x;
    const uint32_t y = mulShr14(static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00624830) - GP4_GLOBAL(uint32_t, 0x00624828)), fraction)
        + GP4_GLOBAL(uint32_t, 0x00624828);
    GP4_GLOBAL(uint32_t, 0x00624838) = y;

    const uint32_t xx = GP4_GLOBAL(uint32_t, 0x00624834) * GP4_GLOBAL(uint32_t, 0x00624834);
    const uint32_t yy = GP4_GLOBAL(uint32_t, 0x00624838) * GP4_GLOBAL(uint32_t, 0x00624838);
    gp4::Regs root = *r;
    root.st_in = 0;
    root.st_out = 0;
    root.eax = yy + xx;
    gp4::call_regs(0x004010f3, root);
    GP4_GLOBAL(uint32_t, 0x0062483c) = root.eax;

    const uint16_t difference = static_cast<uint16_t>(
        GP4_FIELD(uint16_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(segment)), 0) - GP4_FIELD(uint16_t, vector, 0x18));
    const uint16_t half = static_cast<uint16_t>(static_cast<int16_t>(difference) >> 1);
    const int32_t cosine = tableValue(static_cast<uint16_t>(0x4000u - half));
    const int32_t sine = tableValue(half);

    int32_t lateral = static_cast<int32_t>(mulShr14(GP4_GLOBAL(int32_t, 0x0062483c), sine));
    if (lateral < 0) {
        int32_t size = static_cast<int32_t>(0u - static_cast<uint32_t>(lateral));
        if (size < 0x100) {
            size = 0x100;
        }
        lateral = static_cast<int32_t>(0u - static_cast<uint32_t>(size));
    } else if (lateral < 0x100) {
        lateral = 0x100;
    }

    int32_t rest;
    int32_t ratio = gp4::idiv64(static_cast<int64_t>(cosine) * 1024, lateral, &rest);
    if (ratio < 0) {
        ratio = -ratio;
    }
    const uint32_t flags = compareFlags32(static_cast<uint32_t>(ratio), 0x10000u);
    if (ratio > 0x10000) {
        ratio = 0x10000;
    }

    r->eax = static_cast<uint32_t>(ratio);
    r->ecx = static_cast<uint32_t>(lateral);
    r->edx = static_cast<uint32_t>(rest);
    r->ebp = static_cast<uint32_t>(cosine);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00427f3c, FUN_00427f3c, "regs(edi:rec[0x8943a0,0x160,0..29], esi:ptr[0x20]:bytes) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+walk_00427f3c")
