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
// The block at +0xf8 is the coefficient block of 0x00411e17 (Blend18 lands on +0x110).
struct Unknown_Record_00601510 {
    uint8_t Unknown00[0x48];
    double Unknown48;
    double Unknown50;
    uint8_t Unknown58[0x70 - 0x58];
    double Unknown70;
    uint8_t Unknown78[8];
    double Unknown80;
    uint8_t Unknown88[0x110 - 0x88];
    double Unknown110;
    uint8_t Unknown118[0x158 - 0x118];
};
static_assert(sizeof(Unknown_Record_00601510) == 0x158, "record stride");

// FUNCTION: GP4 0x00411d31
// Sibling of 0x00411cc4 using another coefficient set of the same record: the stored factor is
// (1 - sign(st0) * rec.+0x48) * rec.+0x80 / rec.+0x50 at rec.+0x110, and 0x00411e17 runs on the
// block at rec+0xf8.
void __cdecl FUN_00411d31(gp4::Regs* r) {
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
        const Ext factor = fchs(sign * ext(rec->Unknown48)) + ext_one();
        rec->Unknown110 = narrow((factor * ext(rec->Unknown80)) / ext(rec->Unknown50));

        eax = recAddress + 0xf8u;
        flags = additionFlags32(recAddress, 0xf8u);

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
GP4_IMPL(0x00411d31, FUN_00411d31, "regs(ecx:u32[0..3], st0:f64) -> (eax, edx, st0, cf, pf, af, zf, sf, of); globals=layout=tyre_00601510")
