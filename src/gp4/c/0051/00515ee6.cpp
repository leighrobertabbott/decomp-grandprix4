#include <gp4/gp4.h>

// Calls an asm car-record test routine (esi = car) and returns its carry flag.
static inline bool carTestCarry(uintptr_t fn, uint8_t* car) {
    gp4::Regs c = {};
    c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car));
    gp4::call_regs(fn, c);
    return (c.eflags & 1u) != 0u;
}

// FUNCTION: GP4 0x00515ee6
// pushal/popal routine, the short form of 0x00515f40: classifies the selected car [0x679d30] into
// the three dwords 0xa69878 (car index = (number & 0x3f) - 1), 0xa6987c (code, preset to 0x10)
// and 0xa69880 (kind). If either car test (0x004081a1, then 0x00408134) reports carry the code
// becomes 2 and the kind 1.
void __cdecl FUN_00515ee6() {
    uint8_t* car = GP4_GLOBAL(uint8_t*, 0x00679d30);
    GP4_GLOBAL(int32_t, 0x00a69878) = static_cast<int32_t>(car[0x7c] & 0x3fu) - 1;
    GP4_GLOBAL(int32_t, 0x00a6987c) = 0x10;

    if (carTestCarry(0x004081a1, car) || carTestCarry(0x00408134, car)) {
        GP4_GLOBAL(int32_t, 0x00a6987c) = 2;
        GP4_GLOBAL(int32_t, 0x00a69880) = 1;
    }
}
GP4_IMPL(0x00515ee6, FUN_00515ee6, "cdecl() -> void; globals=layout=carbg_0066fef0+state_0044334d")
