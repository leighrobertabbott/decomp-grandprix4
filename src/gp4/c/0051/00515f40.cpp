#include <gp4/gp4.h>

// Calls an asm car-record test routine (esi = car) and returns its carry flag.
static inline bool carTestCarry(uintptr_t fn, uint8_t* car) {
    gp4::Regs c = {};
    c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car));
    gp4::call_regs(fn, c);
    return (c.eflags & 1u) != 0u;
}

// FUNCTION: GP4 0x00515f40
// pushal/popal routine: classifies the selected car [0x679d30] into the three dwords
// 0xa69878 (car index = (number & 0x3f) - 1), 0xa6987c (code) and 0xa69880 (kind).
void __cdecl FUN_00515f40() {
    uint8_t* car = GP4_GLOBAL(uint8_t*, 0x00679d30);
    GP4_GLOBAL(int32_t, 0x00a69878) = static_cast<int32_t>(car[0x7c] & 0x3fu) - 1;
    GP4_GLOBAL(int32_t, 0x00a6987c) = 0x10;

    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    if (mode == 0) {
        if (carTestCarry(0x004081cc, car)) {
            GP4_GLOBAL(int32_t, 0x00a6987c) = 0;
            GP4_GLOBAL(int32_t, 0x00a69880) = 3;
            return;
        }
        const uint32_t state = car[0x74];
        if (state == 0) {
            GP4_GLOBAL(int32_t, 0x00a6987c) = 10;
            GP4_GLOBAL(int32_t, 0x00a69880) = 2;
        } else if (state == 1) {
            GP4_GLOBAL(int32_t, 0x00a6987c) = 0xb;
            GP4_GLOBAL(int32_t, 0x00a69880) = 2;
        } else if (state == 2) {
            if (GP4_FIELD(uint32_t, car, 0x1c) < GP4_GLOBAL(uint32_t, 0x00891500)) {
                GP4_GLOBAL(int32_t, 0x00a6987c) = 8;
                GP4_GLOBAL(int32_t, 0x00a69880) = 2;
            } else {
                GP4_GLOBAL(int32_t, 0x00a6987c) = 0x10;
            }
        }
        return;
    }

    if ((GP4_GLOBAL(uint8_t, 0x007c2e75) == 0xff || mode == 0x44)
        && (car[0x8d] & 2u) == 0u
        && !carTestCarry(0x004081cc, car)) {
        const uint32_t state = car[0x74];
        if (state == 0) {
            GP4_GLOBAL(int32_t, 0x00a6987c) = 10;
            GP4_GLOBAL(int32_t, 0x00a69880) = 2;
        } else if (state == 1) {
            GP4_GLOBAL(int32_t, 0x00a6987c) = 0xb;
            GP4_GLOBAL(int32_t, 0x00a69880) = 2;
        } else if (state == 2) {
            GP4_GLOBAL(int32_t, 0x00a6987c) = 8;
            GP4_GLOBAL(int32_t, 0x00a69880) = 2;
        }
        return;
    }

    if (car == GP4_GLOBAL(uint8_t*, 0x0066d824)) {
        GP4_GLOBAL(int32_t, 0x00a6987c) = 7;
        GP4_GLOBAL(int32_t, 0x00a69880) = 2;
        return;
    }
    if (carTestCarry(0x00408216, car)) {
        GP4_GLOBAL(int32_t, 0x00a6987c) = 9;
        GP4_GLOBAL(int32_t, 0x00a69880) = 2;
        return;
    }
    if ((car[0x8d] & 2u) != 0u) {
        return;
    }
    if (carTestCarry(0x004081cc, car)) {
        return;
    }
    gp4::Regs c = {};
    c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car));
    gp4::call_regs(0x00516874, c);
    if (c.eax == 0u) {
        GP4_GLOBAL(int32_t, 0x00a6987c) = 1;
        GP4_GLOBAL(int32_t, 0x00a69880) = 2;
    } else if (c.eax == 2u) {
        GP4_GLOBAL(int32_t, 0x00a6987c) = 0xc;
        GP4_GLOBAL(int32_t, 0x00a69880) = 2;
    }
}
GP4_IMPL(0x00515f40, FUN_00515f40, "cdecl() -> void; globals=layout=carbg_0066fef0+state_0044334d+cars_0066fef0+sel_0051634b+state_004320f2")
