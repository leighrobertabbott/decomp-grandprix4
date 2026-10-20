#include <gp4/gp4.h>

struct Unknown_00435a4e {
    uint8_t Unknown00[0xcb];
    uint8_t FlagsCB;
    uint8_t UnknownCC[0xda - 0xcc];
    int16_t WordDA;
    uint8_t UnknownDC[0x2a8 - 0xdc];
    int32_t Blend2A8;
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

// Q14 fixed-point blend: the dword at +0x2a8 moves toward the word at +0xda by a factor
// t = min(factor * DAT_00603858 >> 14, 0x4000) / 0x4000, where factor is DAT_006033f0, or
// DAT_006033f4 when bit 3 of the byte at +0xcb is set.
// FUNCTION: GP4 0x00435a4e
void __cdecl FUN_00435a4e(gp4::Regs* r) {
    Unknown_00435a4e* body = reinterpret_cast<Unknown_00435a4e*>(r->esi);

    int32_t factor = GP4_GLOBAL(int32_t, 0x006033f0);
    if ((body->FlagsCB & 8) != 0) {
        factor = GP4_GLOBAL(int32_t, 0x006033f4);
    }
    const int64_t p0 = (int64_t)factor * (int64_t)GP4_GLOBAL(int32_t, 0x00603858);
    int32_t t = (int32_t)(uint32_t)(uint64_t)(p0 >> 14);
    if (t > 0x4000) {
        t = 0x4000;
    }

    const int64_t p1 = (int64_t)(int32_t)body->WordDA * (int64_t)t;
    const uint32_t scaledTarget = (uint32_t)(uint64_t)(p1 >> 14);

    const int32_t rest = (int32_t)(0x4000u - (uint32_t)t);
    const int64_t p2 = (int64_t)rest * (int64_t)body->Blend2A8;
    const uint32_t scaledOld = (uint32_t)(uint64_t)(p2 >> 14);
    const uint32_t sum = scaledOld + scaledTarget;

    body->Blend2A8 = (int32_t)sum;
    r->eax = sum;
    r->ecx = (uint32_t)t;
    r->edx = (uint32_t)(uint64_t)(p2 >> 32);
    r->ebp = scaledTarget;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(scaledOld, scaledTarget, sum);
}
GP4_IMPL(0x00435a4e, FUN_00435a4e, "regs(esi:ptr[0x2b0]:bytes) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
