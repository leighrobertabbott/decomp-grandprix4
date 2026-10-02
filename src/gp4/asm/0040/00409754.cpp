#include <gp4/gp4.h>

struct Unknown_00409754 {
    uint8_t Unknown00[0x278];
    int16_t Unknown278;
    int16_t Unknown27A;
    int16_t Unknown27C;
    int16_t Unknown27E;
    int16_t Unknown280;
    int16_t Unknown282;
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// FUNCTION: GP4 0x00409754
void __cdecl FUN_00409754(gp4::Regs* r) {
    Unknown_00409754* body = reinterpret_cast<Unknown_00409754*>(static_cast<uintptr_t>(r->esi));
    const int32_t first = GP4_GLOBAL(int32_t, 0x00630224);
    const int32_t second = GP4_GLOBAL(int32_t, 0x00630228);

    const int16_t firstShifted = static_cast<int16_t>(first >> 10);
    body->Unknown280 = firstShifted;
    body->Unknown27E = firstShifted;
    const int32_t result = second >> 10;
    const int16_t secondShifted = static_cast<int16_t>(result);
    body->Unknown27A = secondShifted;
    body->Unknown278 = secondShifted;
    body->Unknown282 = 0;
    body->Unknown27C = 0;

    // the last sar eax, 10 leaves the flags: CF = last bit shifted out; OF = AF = 0 (oracle model)
    r->eax = static_cast<uint32_t>(result);
    const uint32_t flags = (((static_cast<uint32_t>(second) >> 9) & 1u) != 0u ? 1u : 0u)
        | parityFlag(static_cast<uint32_t>(result))
        | (result == 0 ? 0x40u : 0u)
        | (result < 0 ? 0x80u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00409754, FUN_00409754, "regs(esi:ptr[0x28a]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
