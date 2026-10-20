#include <gp4/gp4.h>
#include <gp4/x87.h>

struct Unknown_0041d338 {
    int32_t Position00;
    int32_t Position04;
    int32_t Position08;
    uint8_t Unknown0c[0x58 - 0x0c];
    int16_t Offset58;
    int16_t Offset5a;
    int16_t Offset5c;
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `add left, right` whose result is `result`.
static inline uint32_t addFlags(uint32_t left, uint32_t right, uint32_t result) {
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((left ^ result) & (right ^ result)) & 0x80000000u) >> 20);
}

// Rotates the sign-extended word triple at esi+0x58 (scaled by 32) through the matrix of the body
// at 0x00622e98 and, after subtracting the rounded result from the dwords at 0x00638040 (x, y, z),
// the new vector through the matrix of the body at 0x00622cf8; the rounded second result is added
// to the dwords at esi+0 / esi+4 (x, z swapped: esi+8 receives y + [0x00637fe4]). edx is left
// holding 0x00622cf8, eax the last rounded value; the flags are those of the final `add [esi+4], eax`.
// FUNCTION: GP4 0x0041d338
void __cdecl FUN_0041d338(gp4::Regs* r) {
    Unknown_0041d338* record = reinterpret_cast<Unknown_0041d338*>(static_cast<uintptr_t>(r->esi));
    uint8_t* first = GP4_ARRAY(uint8_t, 0x00622e98);
    uint8_t* second = GP4_ARRAY(uint8_t, 0x00622cf8);

    GP4_GLOBAL(int32_t, 0x00623e18) = static_cast<int32_t>(static_cast<uint32_t>(static_cast<int32_t>(record->Offset58)) << 5);
    GP4_GLOBAL(int32_t, 0x00623e1c) = static_cast<int32_t>(static_cast<uint32_t>(static_cast<int32_t>(record->Offset5a)) << 5);
    GP4_GLOBAL(int32_t, 0x00623e20) = static_cast<int32_t>(static_cast<uint32_t>(static_cast<int32_t>(record->Offset5c)) << 5);

    GP4_FIELD(double, first, 0xb8) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00623e18));
    GP4_FIELD(double, first, 0xc0) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00623e1c));
    GP4_FIELD(double, first, 0xc8) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00623e20));

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(first));
    gp4::call_regs(0x00401f02, call);

    GP4_GLOBAL(int32_t, 0x00623e24) = gp4::x87::fistp32(GP4_FIELD(double, first, 0xa0));
    GP4_GLOBAL(int32_t, 0x00623e28) = gp4::x87::fistp32(GP4_FIELD(double, first, 0xa8));
    GP4_GLOBAL(int32_t, 0x00623e2c) = gp4::x87::fistp32(GP4_FIELD(double, first, 0xb0));

    GP4_GLOBAL(uint32_t, 0x00638040) -= GP4_GLOBAL(uint32_t, 0x00623e24);
    GP4_GLOBAL(uint32_t, 0x00638044) -= GP4_GLOBAL(uint32_t, 0x00623e28);
    GP4_GLOBAL(uint32_t, 0x00638048) -= GP4_GLOBAL(uint32_t, 0x00623e2c);

    GP4_FIELD(double, second, 0xb8) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00638040));
    GP4_FIELD(double, second, 0xc0) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00638044));
    GP4_FIELD(double, second, 0xc8) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00638048));

    call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(second));
    gp4::call_regs(0x00401f02, call);

    GP4_GLOBAL(int32_t, 0x00623e30) = gp4::x87::fistp32(GP4_FIELD(double, second, 0xa0));
    GP4_GLOBAL(int32_t, 0x00623e34) = gp4::x87::fistp32(GP4_FIELD(double, second, 0xa8));
    GP4_GLOBAL(int32_t, 0x00623e38) = gp4::x87::fistp32(GP4_FIELD(double, second, 0xb0));

    record->Position00 = static_cast<int32_t>(static_cast<uint32_t>(record->Position00) + GP4_GLOBAL(uint32_t, 0x00623e30));
    record->Position08 = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00623e34) + GP4_GLOBAL(uint32_t, 0x00637fe4));

    const uint32_t last = GP4_GLOBAL(uint32_t, 0x00623e38);
    const uint32_t before = static_cast<uint32_t>(record->Position04);
    const uint32_t sum = before + last;
    record->Position04 = static_cast<int32_t>(sum);

    r->eax = last;
    r->edx = 0x00622cf8u;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(before, last, sum);
}
GP4_IMPL(0x0041d338, FUN_0041d338, "regs(esi:ptr[0x64]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
