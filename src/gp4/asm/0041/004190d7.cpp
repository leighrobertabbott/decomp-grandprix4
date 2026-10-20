#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 8-bit `cmp left, right`.
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80u) != 0u ? 0x800u : 0u);
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of a byte logic result (test / or): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// One of the four 0x300-byte wheel records at 0x006380d8.
struct Unknown_004190d7_Wheel {
    uint8_t Unknown00[0x20];
    int32_t Unknown20;
    uint8_t Unknown24[0x28 - 0x24];
    int32_t Unknown28;
    uint8_t Unknown2C[0x2d - 0x2c];
    uint8_t Unknown2D;
    uint8_t Unknown2E;
    uint8_t Unknown2F[0x300 - 0x2f];
};

// FUNCTION: GP4 0x004190d7
// esi = car record; its dword +0x240 holds one byte per wheel (the wheel is considered when the byte
// is below 0x20). A considered wheel (byte +0x2e bit 5 clear, byte +0x2d zero) sets bit 3 of +0x86
// and bit (3 - wheel) of +0x76 when |dword +0x20| >= DAT_00626934 or |dword +0x28| >= DAT_00626938.
// ebx = 0x006380d8, ecx = the top byte of +0x240, eax = the last absolute value loaded, the flags
// those of the last comparison / or.
void __cdecl FUN_004190d7(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t ecx = GP4_FIELD(uint32_t, car, 0x240);
    uint32_t eax = r->eax;
    uint32_t flags = 0;
    const Unknown_004190d7_Wheel* wheel = reinterpret_cast<const Unknown_004190d7_Wheel*>(0x006380d8u);

    for (uint32_t i = 0; i < 4u; ++i, ++wheel) {
        if (i != 0u) {
            ecx >>= 8;
        }
        const uint32_t cl = ecx & 0xffu;
        flags = cmpFlags8(cl, 0x20u);
        if (cl >= 0x20u) {
            continue;
        }
        const uint32_t masked = wheel->Unknown2E & 0x20u;
        flags = logicFlags8(masked);
        if (masked != 0u) {
            continue;
        }
        flags = cmpFlags8(wheel->Unknown2D, 0u);
        if (wheel->Unknown2D != 0u) {
            continue;
        }
        eax = static_cast<uint32_t>(wheel->Unknown20);
        if (static_cast<int32_t>(eax) < 0) {
            eax = 0u - eax;
        }
        const uint32_t limitX = GP4_GLOBAL(uint32_t, 0x00626934);
        flags = cmpFlags32(eax, limitX);
        bool fire = static_cast<int32_t>(eax) >= static_cast<int32_t>(limitX);
        if (!fire) {
            eax = static_cast<uint32_t>(wheel->Unknown28);
            if (static_cast<int32_t>(eax) < 0) {
                eax = 0u - eax;
            }
            const uint32_t limitY = GP4_GLOBAL(uint32_t, 0x00626938);
            flags = cmpFlags32(eax, limitY);
            fire = !(static_cast<int32_t>(eax) < static_cast<int32_t>(limitY));
        }
        if (fire) {
            car[0x86] = static_cast<uint8_t>(car[0x86] | 8u);
            car[0x76] = static_cast<uint8_t>(car[0x76] | (8u >> i));
            flags = logicFlags8(car[0x76]);
        }
    }

    r->eax = eax;
    r->ebx = 0x006380d8u;
    r->ecx = ecx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004190d7, FUN_004190d7, "regs(eax:u32, esi:ptr[0x248]:layout=rec_004190d7) -> (eax, ebx, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=telem_00418942+wheels_004190d7")
