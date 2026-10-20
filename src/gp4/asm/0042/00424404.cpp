#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// FUNCTION: GP4 0x00424404
void __cdecl FUN_00424404(gp4::Regs* r) {
    void* body = reinterpret_cast<void*>(static_cast<uintptr_t>(r->esi));

    GP4_GLOBAL(int32_t, 0x00607fc4) = 0;
    GP4_GLOBAL(int32_t, 0x00607fcc) = 0;
    GP4_GLOBAL(int32_t, 0x00607fc8) = GP4_GLOBAL(int32_t, 0x00602fd8) >> 5;

    // Each term: (field >> 5) * factor as a signed 64-bit product, shrd by 14, negated.
    const int32_t a0 = GP4_FIELD(int32_t, body, 0x18) >> 5;
    const int64_t p0 = static_cast<int64_t>(a0) * static_cast<int64_t>(GP4_FIELD(int32_t, body, 0x50));
    const uint32_t r0 = 0u - static_cast<uint32_t>(static_cast<uint64_t>(p0) >> 14);
    GP4_GLOBAL(uint32_t, 0x00607fdc) = r0;

    const int32_t a1 = GP4_FIELD(int32_t, body, 0x1c) >> 5;
    const int64_t p1 = static_cast<int64_t>(a1) * static_cast<int64_t>(GP4_FIELD(int32_t, body, 0x54));
    const uint32_t r1 = 0u - static_cast<uint32_t>(static_cast<uint64_t>(p1) >> 14);
    GP4_GLOBAL(uint32_t, 0x00607fe0) = r1;

    const int32_t a2 = GP4_FIELD(int32_t, body, 0x20) >> 5;
    const int64_t p2 = static_cast<int64_t>(a2) * static_cast<int64_t>(GP4_FIELD(int32_t, body, 0x50));
    const uint32_t shifted = static_cast<uint32_t>(static_cast<uint64_t>(p2) >> 14);
    const uint32_t r2 = 0u - shifted;
    GP4_GLOBAL(uint32_t, 0x00607fe4) = r2;

    r->eax = r2;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(p2) >> 32);

    // Flags are those of the final `neg eax`.
    uint32_t flags = parityFlag(r2);
    if (shifted != 0u) flags |= 0x001u;               // CF
    if ((shifted & 0xfu) != 0u) flags |= 0x010u;      // AF
    if (r2 == 0u) flags |= 0x040u;                    // ZF
    if ((r2 & 0x80000000u) != 0u) flags |= 0x080u;    // SF
    if (shifted == 0x80000000u) flags |= 0x800u;      // OF
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00424404, FUN_00424404, "regs(esi:ptr[0x5c]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
