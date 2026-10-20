#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit AND / OR result (CF = OF = AF = 0).
static inline uint32_t logicFlags8(uint32_t value) {
    value &= 0xffu;
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x80u) != 0u ? 0x80u : 0u);
}

// Car-record update (esi = car record): when the car is in first gear (+0x72 == 1) with flags +0x78 bit 6 and
// +0x6f bit 0 set and the global 0x00622424 clear, the scale 0x0062fb78 is recomputed by 0x00416fce, provided the
// value from 0x0041be40 is below [0x62fb7c] - [0x62fb80] - [0x62fb8c] and either bit 1 of +0x179 is already set or
// the value is below [0x62fb90]; it then sets that bit and 0x0062fb4a = 0xff. Every other path (a too-large value
// sets 0x007a8e01 through 0x0051f2c2 first) resets 0x0062fb78 = 0x4000, clears the bit and 0x0062fb4a.
// FUNCTION: GP4 0x0041be61
void __cdecl FUN_0041be61(gp4::Regs* r) {
    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    GP4_GLOBAL(int32_t, 0x0062fb78) = 0x4000;
    bool update = false;
    if (GP4_GLOBAL(uint8_t, 0x00622424) == 0u && (GP4_FIELD(uint8_t, car, 0x78) & 0x40u) != 0u) {
        const uint32_t gear = GP4_FIELD(uint8_t, car, 0x72);
        c.eax = (c.eax & 0xffffff00u) | gear;
        if (gear == 1u && (GP4_FIELD(uint8_t, car, 0x6f) & 1u) != 0u) {
            gp4::call_regs(0x0041be40, c);
            const int32_t value = static_cast<int32_t>(c.eax);
            if ((GP4_FIELD(uint8_t, car, 0x179) & 2u) != 0u || value < GP4_GLOBAL(int32_t, 0x0062fb90)) {
                const int32_t limit = GP4_GLOBAL(int32_t, 0x0062fb7c) - GP4_GLOBAL(int32_t, 0x0062fb80)
                    - GP4_GLOBAL(int32_t, 0x0062fb8c);
                c.edx = static_cast<uint32_t>(limit);
                if (value < limit) {
                    gp4::call_regs(0x00416fce, c);
                    const uint32_t flags179 = GP4_FIELD(uint8_t, car, 0x179) | 2u;
                    GP4_FIELD(uint8_t, car, 0x179) = static_cast<uint8_t>(flags179);
                    c.eflags = (c.eflags & ~0x8d5u) | logicFlags8(flags179);
                    GP4_GLOBAL(uint8_t, 0x0062fb4a) = 0xffu;
                    update = true;
                } else {
                    gp4::call_regs(0x0051f2c2, c);
                }
            }
        }
    }

    if (!update) {
        GP4_GLOBAL(int32_t, 0x0062fb78) = 0x4000;
        const uint32_t flags179 = GP4_FIELD(uint8_t, car, 0x179) & 0xfdu;
        GP4_FIELD(uint8_t, car, 0x179) = static_cast<uint8_t>(flags179);
        c.eflags = (c.eflags & ~0x8d5u) | logicFlags8(flags179);
        GP4_GLOBAL(uint8_t, 0x0062fb4a) = 0u;
    }
    *r = c;
}
GP4_IMPL(0x0041be61, FUN_0041be61, "regs(eax:u32, esi:ptr[0x250]:layout=car_0041aea9) -> (eax, edx, ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0041b950+state_00416fce+state_0041242f")
