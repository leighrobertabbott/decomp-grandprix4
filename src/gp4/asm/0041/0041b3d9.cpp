#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of the 16-bit `add left, right`.
static inline uint32_t addFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left + right) & 0xffffu;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0041b3d9
// Steering/response value for the car record at esi: eax = the response curve 0x0041b286 of the
// incoming eax, or (record bit 0x80 of +0xcd) -(round([0x6020d8 + 0x60 * byte +0x202]) >> 8), the
// rounded integer also being kept in [0x6f2bf4]. Every register except eax is restored. When
// edx == -1 the result is also stored in [0x602928] and (word [0x679c1c] - result) sign-extended in
// [0x60292c]; the flags are those of that 16-bit add, otherwise those of `cmp edx, -1`.
void __cdecl FUN_0041b3d9(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t mode = r->edx;

    int32_t result;
    if ((GP4_FIELD(uint8_t, car, 0xcd) & 0x80u) == 0u) {
        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        gp4::call_regs(0x0041b286, call);
        result = static_cast<int32_t>(call.eax);
    } else {
        const uint32_t index = GP4_FIELD(uint8_t, car, 0x202);
        const double* entry = reinterpret_cast<const double*>(static_cast<uintptr_t>(0x006020d8u + index * 0x60u));
        GP4_GLOBAL(int32_t, 0x006f2bf4) = gp4::x87::fistp32(*entry);
        result = -(GP4_GLOBAL(int32_t, 0x006f2bf4) >> 8);
    }

    uint32_t flags = subFlags32(mode, 0xffffffffu);
    if (mode == 0xffffffffu) {
        GP4_GLOBAL(int32_t, 0x00602928) = result;
        const uint16_t sign = GP4_GLOBAL(uint16_t, 0x00679c1c);
        const uint16_t negated = static_cast<uint16_t>(0u - static_cast<uint32_t>(result));
        flags = addFlags16(negated, sign);
        GP4_GLOBAL(int32_t, 0x0060292c) = static_cast<int16_t>(static_cast<uint16_t>(negated + sign));
    }

    r->eax = static_cast<uint32_t>(result);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041b3d9, FUN_0041b3d9, "regs(eax:i32, edx:i32[-1..0], esi:ptr[0x420]:layout=car_0042ba87+slot_0041b3d9) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=tab_006020d8")
