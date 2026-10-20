#include <gp4/gp4.h>

struct Unknown_004358f8 {
    uint8_t Unknown00[0x418];
    int16_t Blend418;
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of "add reg32, reg32": CF, PF, AF, ZF, SF, OF
static inline uint32_t addFlags(uint32_t a, uint32_t b, uint32_t sum) {
    return (sum < a ? 0x001u : 0u)
        | parityFlag(sum)
        | (((a ^ b ^ sum) & 0x10u) ? 0x010u : 0u)
        | (sum == 0 ? 0x040u : 0u)
        | ((sum & 0x80000000u) ? 0x080u : 0u)
        | ((((a ^ sum) & (b ^ sum)) & 0x80000000u) ? 0x800u : 0u);
}

// Q14 fixed-point blend: the word at +0x418 of the body moves toward the value in eax by a
// factor t = min(DAT_00603558 * DAT_00603858 >> 14, 0x4000) / 0x4000.
// FUNCTION: GP4 0x004358f8
void __cdecl FUN_004358f8(gp4::Regs* r) {
    Unknown_004358f8* body = reinterpret_cast<Unknown_004358f8*>(r->esi);
    const int32_t target = (int32_t)r->eax;

    const int64_t p0 = (int64_t)GP4_GLOBAL(int32_t, 0x00603558) * (int64_t)GP4_GLOBAL(int32_t, 0x00603858);
    int32_t t = (int32_t)(uint32_t)(uint64_t)(p0 >> 14);
    if (t > 0x4000) {
        t = 0x4000;
    }

    const int64_t p1 = (int64_t)target * (int64_t)t;
    const uint32_t scaledTarget = (uint32_t)(uint64_t)(p1 >> 14);

    const int64_t p2 = (int64_t)(0x4000 - t) * (int64_t)(int32_t)body->Blend418;
    const uint32_t scaledOld = (uint32_t)(uint64_t)(p2 >> 14);
    const uint32_t sum = scaledOld + scaledTarget;

    body->Blend418 = (int16_t)sum;
    r->eax = sum;
    r->edx = (uint32_t)(uint64_t)(p2 >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(scaledOld, scaledTarget, sum);
}
GP4_IMPL(0x004358f8, FUN_004358f8, "regs(eax:u32, esi:ptr[0x420]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
