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
    left &= 0xffu;
    right &= 0xffu;
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | (result & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// Flags of the 32-bit `shl value, count` (count > 1): CF = last bit out, AF = 0,
// OF = bit31(value << (count - 1) xor result), as modelled by the oracle.
static inline uint32_t shlFlags32(uint32_t value, uint32_t count) {
    const uint32_t previous = value << (count - 1u);
    const uint32_t result = value << count;
    return ((previous >> 31) & 1u)
        | parityFlag8(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((previous ^ result) >> 31) & 1u) != 0u ? 0x800u : 0u);
}

// Debris-part mass update of the car at esi (pushal/popal: every register is preserved, the flags are those of
// the last flag writer). The detached-part mask ~[+0x40] & 0x7fffff goes through the mass model 0x444517, then
// 0x444715 rescales it. The centroid (0x62a064..6c), inertias (0x62a070..78) and mass (0x62a07c) are copied to
// the car (+0x58.., +0x204.. = inertia >> 8, +0xb2 = max(mass + [+0x19c]*[0x602b4c] >> 28, [0x62a0a0]),
// +0x168 = mass); when the car has an object slot (+0x25e != 0xff) the 0x78-byte object record at
// 0x63bd7c + slot*0x78 gets the same values (centroid << 5, inertias, +0x48 = word +0xb2, +0x4c = mask).
// FUNCTION: GP4 0x00444419
void __cdecl FUN_00444419(gp4::Regs* r) {
    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    const uint32_t mask = ~GP4_FIELD(uint32_t, car, 0x40) & 0x007fffffu;
    gp4::Regs model = *r;
    model.st_in = 0;
    model.st_out = 0;
    model.eax = mask;
    gp4::call_regs(0x00444517, model);
    gp4::Regs rescale = *r;
    rescale.st_in = 0;
    rescale.st_out = 0;
    gp4::call_regs(0x00444715, rescale);

    const int32_t centroidX = GP4_GLOBAL(int32_t, 0x0062a064);
    const int32_t centroidY = GP4_GLOBAL(int32_t, 0x0062a068);
    const int32_t centroidZ = GP4_GLOBAL(int32_t, 0x0062a06c);
    const int32_t inertiaX = GP4_GLOBAL(int32_t, 0x0062a070);
    const int32_t inertiaY = GP4_GLOBAL(int32_t, 0x0062a074);
    const int32_t inertiaZ = GP4_GLOBAL(int32_t, 0x0062a078);
    const int32_t mass = GP4_GLOBAL(int32_t, 0x0062a07c);

    GP4_FIELD(uint16_t, car, 0x58) = static_cast<uint16_t>(centroidX);
    GP4_FIELD(uint16_t, car, 0x5a) = static_cast<uint16_t>(centroidY);
    GP4_FIELD(uint16_t, car, 0x5c) = static_cast<uint16_t>(centroidZ);
    GP4_FIELD(uint16_t, car, 0x204) = static_cast<uint16_t>(inertiaX >> 8);
    GP4_FIELD(uint16_t, car, 0x206) = static_cast<uint16_t>(inertiaY >> 8);
    GP4_FIELD(uint16_t, car, 0x208) = static_cast<uint16_t>(inertiaZ >> 8);

    const int64_t product = static_cast<int64_t>(GP4_FIELD(int32_t, car, 0x19c)) * GP4_GLOBAL(int32_t, 0x00602b4c);
    const uint32_t productLow = static_cast<uint32_t>(product);
    const uint32_t productHigh = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    int32_t value = static_cast<int32_t>(((productLow >> 28) | (productHigh << 4)) + static_cast<uint32_t>(mass));
    const int32_t floorValue = GP4_GLOBAL(int32_t, 0x0062a0a0);
    if (value < floorValue) {
        value = floorValue;
    }
    GP4_FIELD(uint16_t, car, 0xb2) = static_cast<uint16_t>(value);
    GP4_FIELD(uint16_t, car, 0x168) = static_cast<uint16_t>(mass);

    const uint32_t slot = GP4_FIELD(uint8_t, car, 0x25e);
    uint32_t flags = cmpFlags8(slot, 0xffu);
    if (slot != 0xffu) {
        uint32_t* const object = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(0x0063bd7cu + slot * 0x78u));
        object[0x4c / 4] = ~GP4_FIELD(uint32_t, car, 0x40) & 0x007fffffu;
        object[0x30 / 4] = static_cast<uint32_t>(centroidX) << 5;
        object[0x34 / 4] = static_cast<uint32_t>(centroidY) << 5;
        object[0x38 / 4] = static_cast<uint32_t>(centroidZ) << 5;
        object[0x3c / 4] = static_cast<uint32_t>(inertiaX);
        object[0x40 / 4] = static_cast<uint32_t>(inertiaY);
        object[0x44 / 4] = static_cast<uint32_t>(inertiaZ);
        object[0x48 / 4] = GP4_FIELD(uint16_t, car, 0xb2);
        flags = shlFlags32(static_cast<uint32_t>(centroidZ), 5u);
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00444419, FUN_00444419, "regs(esi:ptr[0x266]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00444517")
