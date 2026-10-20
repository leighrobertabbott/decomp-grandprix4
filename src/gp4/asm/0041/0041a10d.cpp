#include <gp4/gp4.h>

struct Unknown_0041a10d {
    uint8_t Unknown00[0x2fc];
    int32_t Unknown2FC[4];
    uint8_t Unknown30C[0x37c - 0x30c];
    int32_t Unknown37C[4];
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of "cmp left, right" (dword).
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0041a10d
// Angle (0x401315) of the vector (half the signed sum of the two groups of four dwords at
// +0x2fc / +0x37c, [0x601348] << 4), stored at [0x62f890]; |angle| clamped to 0x4000 gives
// [0x62f880] = 0x4000 below the threshold [0x62f884], else 0x4000 minus the scaled excess
// ((|angle| - [0x62f884]) * [0x62f88c] >> 14), not below the floor [0x62f888].
void __cdecl FUN_0041a10d(gp4::Regs* r) {
    const Unknown_0041a10d* body = reinterpret_cast<const Unknown_0041a10d*>(r->esi);

    uint32_t sum = (uint32_t)body->Unknown2FC[0] - (uint32_t)body->Unknown37C[0];
    sum -= (uint32_t)body->Unknown2FC[1];
    sum += (uint32_t)body->Unknown37C[1];
    sum += (uint32_t)body->Unknown2FC[2];
    sum -= (uint32_t)body->Unknown37C[2];
    sum -= (uint32_t)body->Unknown2FC[3];
    sum += (uint32_t)body->Unknown37C[3];

    gp4::Regs regs = *r;
    regs.eax = (uint32_t)((int32_t)sum >> 1);
    regs.edx = (uint32_t)GP4_GLOBAL(int32_t, 0x00601348) << 4;
    gp4::call_regs(0x00401315, regs);

    int32_t angle = (int32_t)regs.eax;
    GP4_GLOBAL(int32_t, 0x0062f890) = angle;
    if (angle < 0) {
        angle = (int32_t)(0u - (uint32_t)angle);
    }
    if (angle > 0x4000) {
        angle = 0x4000;
    }

    uint32_t flags;
    uint32_t edx = regs.edx;
    int32_t result;
    const int32_t threshold = GP4_GLOBAL(int32_t, 0x0062f884);
    flags = cmpFlags32((uint32_t)angle, (uint32_t)threshold);
    if (angle < threshold) {
        result = 0x4000;
    } else {
        const int32_t excess = (int32_t)((uint32_t)angle - (uint32_t)GP4_GLOBAL(int32_t, 0x0062f884));
        const int64_t product = (int64_t)excess * (int64_t)GP4_GLOBAL(int32_t, 0x0062f88c);
        edx = (uint32_t)((uint64_t)product >> 32);
        const uint32_t scaled = (uint32_t)((uint64_t)product >> 14);
        result = (int32_t)(0x4000u - scaled);
        const int32_t floorValue = GP4_GLOBAL(int32_t, 0x0062f888);
        flags = cmpFlags32((uint32_t)result, (uint32_t)floorValue);
        if (result < floorValue) {
            result = GP4_GLOBAL(int32_t, 0x0062f888);
        }
    }
    GP4_GLOBAL(int32_t, 0x0062f880) = result;

    r->eax = (uint32_t)result;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041a10d, FUN_0041a10d, "regs(esi:ptr[0x390]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
