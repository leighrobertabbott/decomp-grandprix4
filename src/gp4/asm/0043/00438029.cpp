#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `sub left, right`.
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Lane-edge extent at one side of a segment record (edi): the difference of the two angle dwords
// (`angleA - angleB`, high 16 bits) gives an angle 0x4000 - hi, folded to 0..0x8000 and looked up in the Q14
// cosine table; times the radius dword (`scale`), Q14 -> |.| >> 8, plus the lane word (`width`) and 0x50.
static inline int32_t edgeExtent(uint8_t* rec, uint32_t angleA, uint32_t angleB, uint32_t scale, uint32_t width) {
    const uint32_t diff = GP4_FIELD(uint32_t, rec, angleA) - GP4_FIELD(uint32_t, rec, angleB);
    uint16_t angle = (uint16_t)(0x4000u - (uint16_t)(diff >> 16));
    if ((int16_t)angle < 0) {
        angle = (uint16_t)(0u - angle);
    }
    angle = (uint16_t)(angle >> 2);
    const uint32_t index = (uint32_t)angle & 0xfffeu;
    const int32_t cosine = GP4_ARRAY(int16_t, 0x007c5c5c)[index >> 1];
    const int64_t product = (int64_t)cosine * (int64_t)GP4_FIELD(int32_t, rec, scale);
    int32_t value = (int32_t)(uint32_t)((uint64_t)product >> 14);
    if (value < 0) {
        value = (int32_t)(0u - (uint32_t)value);
    }
    return (int32_t)((uint32_t)(value >> 8) + (uint32_t)GP4_FIELD(uint16_t, rec, width) + 0x50u);
}

// esi = car (word +0x90: its sign picks the left or right lane edge), edi = segment record. The two edge
// extents (near edge from +0xec and far edge from +0x24c, i.e. this record and the next) are reduced to the
// smaller; CF = 1 when (min - [0x629e98] - 0x280) is negative, i.e. the car is within 0x280 of the edge.
// FUNCTION: GP4 0x00438029
void __cdecl FUN_00438029(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    int32_t near;
    int32_t far;
    uint32_t farWidth;
    if ((int16_t)GP4_FIELD(uint16_t, car, 0x90) < 0) {
        near = edgeExtent(rec, 0x104, 0xec, 0x100, 0x60);
        far = edgeExtent(rec, 0x264, 0x24c, 0x260, 0x1c0);
        farWidth = GP4_FIELD(uint16_t, rec, 0x1c0);
    } else {
        near = edgeExtent(rec, 0x110, 0xec, 0x10c, 0x62);
        far = edgeExtent(rec, 0x270, 0x24c, 0x26c, 0x1c2);
        farWidth = GP4_FIELD(uint16_t, rec, 0x1c2);
    }

    const int32_t smaller = far > near ? near : far;
    const uint32_t gap = (uint32_t)smaller - GP4_GLOBAL(uint32_t, 0x00629e98);
    const uint32_t result = gap - 0x280u;
    uint32_t flags = subFlags32(gap, 0x280u) & ~1u;
    if ((result & 0x80000000u) != 0u) {
        flags |= 1u;  // stc
    }

    r->eax = (uint32_t)smaller;
    r->ecx = result;
    r->edx = farWidth;
    r->ebp = (uint32_t)near;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00438029, FUN_00438029, "regs(esi:ptr[0x98]:bytes, edi:ptr[0x278]:bytes) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
