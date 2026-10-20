#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `add dst, src` (CF PF AF ZF SF OF).
static inline uint32_t addFlags32(uint32_t dst, uint32_t src) {
    const uint32_t result = dst + src;
    return (result < dst ? 0x01u : 0u)
        | parityFlag8(result)
        | (((dst ^ src ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(dst ^ src) & (dst ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00424cf5
// Starts the object slot of the car in esi: the car's object number (byte +0x202) goes to +0x25e,
// marks slot [0x63bcfc + n] as 2 and [0x63bd3c + n] as the number, clears the 0x78-byte object record
// n at 0x63bd7c (0x424865), fills its car number (+0x6d from 0x424865's saved eax, +0x6c), sets the
// "all" mask (+0x74), flags +0x6f |= 3, runs 0x424878 with the car's mask ~[esi+0x40] & 0x7fffff and
// finally stamps the object's due time +0x60 with the clock [0x66d814] + 0xa4cb80. pushal/popal
// restore every register; clc replaces CF in the flags of the last add.
void __cdecl FUN_00424cf5(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t slot = GP4_FIELD(uint8_t, car, 0x202);
    GP4_FIELD(uint8_t, car, 0x25e) = static_cast<uint8_t>(slot);
    GP4_ARRAY(uint8_t, 0x0063bcfc)[slot] = 2;
    GP4_ARRAY(uint8_t, 0x0063bd3c)[slot] = GP4_FIELD(uint8_t, car, 0x202);

    uint8_t* object = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(0x0063bd7cu + slot * 0x78u));

    gp4::Regs clear = *r;
    clear.st_in = 0;
    clear.st_out = 0;
    clear.eax = slot;
    clear.ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(object));
    gp4::call_regs(0x00424865, clear);

    GP4_FIELD(uint8_t, object, 0x6d) = static_cast<uint8_t>(slot);
    GP4_FIELD(uint8_t, object, 0x6c) = GP4_FIELD(uint8_t, car, 0x202);
    GP4_FIELD(uint32_t, object, 0x74) = 0xffffffffu;
    GP4_FIELD(uint8_t, object, 0x6f) |= 1;
    GP4_FIELD(uint8_t, object, 0x6f) |= 2;

    gp4::Regs start = clear;
    start.ecx = 0;
    start.eax = ~GP4_FIELD(uint32_t, car, 0x40) & 0x007fffffu;
    gp4::call_regs(0x00424878, start);

    const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
    GP4_FIELD(uint32_t, object, 0x60) = clock;
    const uint32_t due = clock + 0x00a4cb80u;
    GP4_FIELD(uint32_t, object, 0x60) = due;
    r->eflags = (r->eflags & ~0x8d5u) | (addFlags32(clock, 0x00a4cb80u) & ~1u);
}
GP4_IMPL(0x00424cf5, FUN_00424cf5, "regs(esi:ptr[0x41c]:layout=car_0041e500+rec_00420e1d+car_00424878+car_00424cf5) -> (cf, pf, af, zf, sf, of); globals=layout=state_0042458b+state_00420e1d+blk_00424553+state_0066d8c0+state_0041bce9+state_00444517+state_00424a21+state_0043c4d9")
