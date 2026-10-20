#include <gp4/gp4.h>
#include <gp4/x87.h>

using gp4::x87::Ext;
using gp4::x87::ext;
using gp4::x87::ext_i32;

static inline uint32_t compareFlags32(uint32_t left, uint32_t right) {
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

static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (result < left ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// One 0x158-byte record of the array at 0x00601510 (only the fields used here).
// The block at +0xc8 is the coefficient block of 0x00411e17 (Blend18 lands on +0xe0).
struct Unknown_Record_00601510 {
    uint8_t Unknown00[0x18];
    double Unknown18;
    uint8_t Unknown20[0x70 - 0x20];
    double Unknown70;
    double Unknown78;
    uint8_t Unknown80[0xc8 - 0x80];
    uint8_t UnknownC8[0xe0 - 0xc8];
    double UnknownE0;
    uint8_t UnknownE8[0x158 - 0xe8];
};
static_assert(sizeof(Unknown_Record_00601510) == 0x158, "record stride");

// FUNCTION: GP4 0x00411cc4
// ecx = record index, st0 = the angle-like input. When round(rec.+0x70 * K) (K = 0x00601450)
// is below -2, the signed factor (1 - sign(st0) * rec.+0x18) * rec.+0x78 is stored to rec.+0xe0
// and 0x00411e17 is run on the record's coefficient block (rec+0xc8); the result is
// scaled by the integer table 0x0062f860[index] and by 0x006f2b9c. Otherwise the result is 0.
void __cdecl FUN_00411cc4(gp4::Regs* r) {
    using namespace gp4::x87;
    const uint32_t index = r->ecx;
    const uint64_t product = static_cast<uint64_t>(0x158u) * index;   // mul ecx
    const uint32_t recAddress = static_cast<uint32_t>(product) + 0x00601510u;
    Unknown_Record_00601510* const rec = reinterpret_cast<Unknown_Record_00601510*>(static_cast<uintptr_t>(recAddress));
    const Ext input = in_st(r, 0);

    const int32_t scaled = fistp32(ext(rec->Unknown70) * ext(GP4_GLOBAL(double, 0x00601450)));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = scaled;

    Ext result;
    uint32_t eax;
    uint32_t flags;
    if (scaled >= -2) {
        result = ext_zero();
        eax = recAddress;
        flags = compareFlags32(static_cast<uint32_t>(scaled), 0xfffffffeu);
    } else {
        Ext sign = ext_one();
        if ((fucom(input, ext_zero()) & 0x0100u) != 0) {
            sign = fchs(sign);
        }
        const Ext factor = fchs(sign * ext(rec->Unknown18)) + ext_one();
        rec->UnknownE0 = narrow(factor * ext(rec->Unknown78));

        eax = recAddress + 0xc8u;
        flags = additionFlags32(recAddress, 0xc8u);

        gp4::Regs callee = *r;
        callee.eax = eax;
        callee.st_xin_mask = 0;
        callee.st_xout_mask = 0;
        callee.st_in = 1;
        callee.st_out = 1;
        set_in_st(callee, 0, input);
        gp4::call_regs(0x00411e17, callee);

        const Ext weighted = out_st(callee, 0) * ext_i32(GP4_ARRAY(const int32_t, 0x0062f860)[index]);
        result = weighted * ext(GP4_GLOBAL(double, 0x006f2b9c));
    }

    r->eax = eax;
    r->edx = static_cast<uint32_t>(product >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
    set_out_st(r, 0, result);
}
GP4_IMPL(0x00411cc4, FUN_00411cc4, "regs(ecx:u32[0..3], st0:f64) -> (eax, edx, st0, cf, pf, af, zf, sf, of); globals=layout=tyre_00601510")
