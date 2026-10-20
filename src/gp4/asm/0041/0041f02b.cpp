#include <gp4/gp4.h>

// Track-segment record (0x160 bytes at 0x008943a0): word +0 is the reference angle, word +0x7a the
// surface type that indexes the 10-byte table at 0x00675a82.
struct Unknown_0041f02b_Segment {
    int16_t Unknown00;
    uint8_t Unknown02[0x7a - 0x02];
    uint16_t SurfaceType7A;
};

// Entry of the 10-byte surface table 0x00675a82 for a segment surface type (kept out of line so the
// per-field offsets stay relative to the table base).
static __declspec(noinline) const uint8_t* surfaceEntry(uint32_t surfaceType) {
    return GP4_ARRAY(const uint8_t, 0x00675a82) + surfaceType * 10u;
}

// FUNCTION: GP4 0x0041f02b
// Surface/grip sampling for the four wheel positions. eax (low word) is an angle, edx a height and edi
// the segment record whose word +0 is subtracted from the angle. The cosine/sine (Q14, table
// 0x007c5c5c) of 0x4000 - delta and delta, scaled by the car dimensions 0x00601350 / 0x00601348 /
// 0x0060134c, give four sample positions at 0x0062ac78..84 (the height edx << 8 is added to each);
// 0x0051f2ca then looks up the surface texture and fills 0x0062ac88... From the segment's surface
// entry (0x00675a82 + 10 * word +0x7a) the limits 0x00602fa4 / 0x00602fa0 are derived and each of the
// four samples goes through 0x0041ef48; the four result bytes are packed into eax (first sample in the
// low byte). Every register but eax is restored; the flags are those of the final "cmp ecx, 4".
void __cdecl FUN_0041f02b(gp4::Regs* r) {
    const Unknown_0041f02b_Segment* segment =
        reinterpret_cast<const Unknown_0041f02b_Segment*>(static_cast<uintptr_t>(r->edi));
    const uint32_t height = r->edx;

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    GP4_GLOBAL(uint32_t, 0x0062283c) = height;

    const uint16_t delta = static_cast<uint16_t>(static_cast<uint16_t>(r->eax) - static_cast<uint16_t>(segment->Unknown00));

    uint16_t complement = static_cast<uint16_t>(0x4000u - delta);
    if (static_cast<int16_t>(complement) < 0) {
        complement = static_cast<uint16_t>(0u - complement);
    }
    const uint32_t cosOffset = (static_cast<uint32_t>(complement) >> 2) & 0xfffeu;
    const int32_t cosine = *reinterpret_cast<const int16_t*>(GP4_ARRAY(const uint8_t, 0x007c5c5c) + cosOffset);

    uint16_t magnitude = delta;
    if (static_cast<int16_t>(magnitude) < 0) {
        magnitude = static_cast<uint16_t>(0u - magnitude);
    }
    const uint32_t sinOffset = (static_cast<uint32_t>(magnitude) >> 2) & 0xfffeu;
    const int32_t sine = *reinterpret_cast<const int16_t*>(GP4_ARRAY(const uint8_t, 0x007c5c5c) + sinOffset);

    const uint32_t lift = height << 8;

    const int64_t forward = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00601350)) * cosine;
    const uint32_t front = static_cast<uint32_t>(static_cast<uint64_t>(forward) >> 14) + lift;

    const int64_t sideways = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00601348)) * sine;
    const uint32_t side = static_cast<uint32_t>(static_cast<uint64_t>(sideways) >> 14);
    GP4_GLOBAL(uint32_t, 0x0062ac84) = side + front;
    GP4_GLOBAL(uint32_t, 0x0062ac80) = front - side;

    const int32_t negated = static_cast<int32_t>(0u - GP4_GLOBAL(uint32_t, 0x0060134c));
    const int64_t rear = static_cast<int64_t>(negated) * cosine;
    const uint32_t back = static_cast<uint32_t>(static_cast<uint64_t>(rear) >> 14) + lift;
    GP4_GLOBAL(uint32_t, 0x0062ac7c) = back + side;
    GP4_GLOBAL(uint32_t, 0x0062ac78) = back - side;

    // pushal / popal wrapper around the texture lookup: every register is preserved.
    s.eax = 0x0062ac78u;
    s.ebx = back;
    s.ecx = static_cast<uint32_t>(sine);
    s.ebp = static_cast<uint32_t>(cosine);
    s.edx = static_cast<uint32_t>(static_cast<uint64_t>(rear) >> 32);
    gp4::call_regs(0x0051f2ca, s);

    const uint8_t* entry = surfaceEntry(segment->SurfaceType7A);
    int32_t floorPosition = entry[0];
    const int32_t position = entry[1];
    GP4_GLOBAL(int32_t, 0x00622834) = position;
    int32_t upperLimit = entry[3];
    if (upperLimit > position) {
        upperLimit = position;
    }
    GP4_GLOBAL(int32_t, 0x00602fa4) = upperLimit;
    int32_t span = static_cast<int32_t>(GP4_ARRAY(const uint8_t, 0x00937dd0)[position >> 4]) << 4;
    int32_t reduction = entry[4];
    if (reduction < 0x10) {
        reduction = 0;
    }
    if (reduction > span) {
        reduction = span;
    }
    const int32_t margin = 0x50 - reduction;
    GP4_GLOBAL(int32_t, 0x00602fa0) = margin;

    uint32_t packed = back;
    int32_t current = floorPosition;
    for (uint32_t k = 0; k != 4u; ++k) {
        int32_t sample = GP4_ARRAY(const int32_t, 0x0062ac78)[k];
        GP4_GLOBAL(int32_t, 0x00622830) = sample;
        sample = (sample - 5) * 16 + GP4_GLOBAL(int32_t, 0x00622834);
        if (sample < current) {
            sample = current;
        }
        int32_t drop = static_cast<int32_t>(static_cast<uint32_t>(GP4_ARRAY(const int32_t, 0x0062ac88)[k]) << 4)
            - GP4_GLOBAL(int32_t, 0x00602fa0);
        if (drop < 0) {
            drop = 0;
        }
        GP4_GLOBAL(int32_t, 0x00622838) = drop;
        current = sample - GP4_GLOBAL(int32_t, 0x00622838);
        if (current < GP4_GLOBAL(int32_t, 0x00602fa4)) {
            current = GP4_GLOBAL(int32_t, 0x00602fa4);
        }
        s.eax = static_cast<uint32_t>(sample);
        s.ebx = packed;
        s.ecx = k;
        s.edx = static_cast<uint32_t>(drop);
        s.ebp = static_cast<uint32_t>(current);
        gp4::call_regs(0x0041ef48, s);
        packed = (packed & 0xffffff00u) | (s.eax & 0xffu);
        packed = (packed >> 8) | (packed << 24);
    }

    r->eax = packed;
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0041f02b, FUN_0041f02b, "regs(eax:u32, edx:i32[-32768..32767], edi:rec[0x8943a0,0x160,0..25]) -> (eax, cf, pf, af, zf, sf, of); globals=layout=surf_0041f02b")
