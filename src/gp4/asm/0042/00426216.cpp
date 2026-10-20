#include <gp4/gp4.h>

static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (left < right ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00426216
void __cdecl FUN_00426216(gp4::Regs* r) {
    // push eax ... pop eax: eax is preserved. edx returns the value clamped to 0x6000; the flags
    // are those of the final "cmp eax, 0x6000" on the value before the clamp.
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const int32_t a = GP4_FIELD(int32_t, body, 0x19c);
    const int32_t b = GP4_GLOBAL(int32_t, 0x00602b4c);
    const int32_t scale = GP4_FIELD(uint16_t, body, 0xba);
    const int32_t c = GP4_GLOBAL(int32_t, 0x0062d2dc);
    const uint32_t offset = GP4_GLOBAL(uint32_t, 0x0062d2a0);

    // imul / shrd 0x1c, sub 0x14, clamp at 0 (signed)
    int32_t v = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(
                    static_cast<int64_t>(a) * static_cast<int64_t>(b)) >> 0x1c));
    v -= 0x14;
    if (v < 0) v = 0;

    // imul by the word, shrd 0xe; imul by the second global, shrd 0xe; + global
    v = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(
            static_cast<int64_t>(v) * static_cast<int64_t>(scale)) >> 0xe));
    v = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(
            static_cast<int64_t>(v) * static_cast<int64_t>(c)) >> 0xe));
    v = static_cast<int32_t>(static_cast<uint32_t>(v) + offset);

    const uint32_t flags = subtractionFlags32(static_cast<uint32_t>(v), 0x6000u);
    if (v > 0x6000) v = 0x6000;

    r->edx = static_cast<uint32_t>(v);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00426216, FUN_00426216, "regs(esi:ptr[0x1a4]:bytes) -> (edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
