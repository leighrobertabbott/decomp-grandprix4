#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t AddFlags(uint32_t lhs, uint32_t rhs, uint32_t result) {
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return static_cast<uint32_t>((static_cast<uint64_t>(lhs) + rhs) >> 32)
        | ((~parity & 1u) << 2) | ((lhs ^ rhs ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 31) << 7)
        | ((~(lhs ^ rhs) & (lhs ^ result) & 0x80000000u) >> 20);
}

static inline uint32_t SubtractFlags(uint32_t lhs, uint32_t rhs, uint32_t result) {
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (lhs < rhs ? 1u : 0u) | ((~parity & 1u) << 2)
        | ((lhs ^ rhs ^ result) & 0x10u) | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) << 7)
        | (((lhs ^ rhs) & (lhs ^ result) & 0x80000000u) >> 20);
}

// FUNCTION: GP4 0x00411da4
// ecx selects a 0x158-byte record at 0x00601510; st0 is the input angle-like value X.
// If round(rec.f70 * DAT_00601450) >= -2 the result is 0.0. Otherwise the blend
// coefficient (1 - sign(X) * rec.f90) * rec.f88 is stored at rec+0x140 and the
// result is DAT_0062f870[ecx] * FUN_00411e17(X; rec+0x128) * DAT_006f2b9c.
void __cdecl FUN_00411da4(gp4::Regs* r) {
    using namespace gp4::x87;
    const uint32_t index = r->ecx;
    const uint64_t scaled = static_cast<uint64_t>(index) * 0x158u;
    const uint32_t rec = static_cast<uint32_t>(scaled) + 0x00601510u;
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(rec));
    r->edx = static_cast<uint32_t>(scaled >> 32);

    const Ext x = in_st(r, 0);
    const Ext product = ext(GP4_FIELD(double, record, 0x70)) * ext(GP4_GLOBAL(double, 0x00601450));
    const int32_t rounded = fistp32(product);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = rounded;

    if (rounded >= -2) {
        set_out_st(r, 0, ext_zero());
        r->eax = rec;
        r->eflags = (r->eflags & ~0x8d5u)
            | SubtractFlags(static_cast<uint32_t>(rounded), 0xfffffffeu, static_cast<uint32_t>(rounded) + 2u);
        return;
    }

    // fucompp / sahf: CF = C0 (X < 0 or unordered) selects -1.0, otherwise +1.0.
    const uint16_t status = fucom(x, ext_zero());
    Ext sign = ext_one();
    if (status & 0x0100u) sign = fchs(sign);
    const Ext blend = (fchs(sign * ext(GP4_FIELD(double, record, 0x90))) + ext_one())
        * ext(GP4_FIELD(double, record, 0x88));
    GP4_FIELD(double, record, 0x140) = narrow(blend);

    gp4::Regs call = *r;
    call.eax = rec + 0x128u;
    call.st_in = 1;
    call.st_out = 1;
    call.st_xin_mask = 0;
    call.st_xout_mask = 0;
    call.eflags = (r->eflags & ~0x8d5u) | AddFlags(rec, 0x128u, rec + 0x128u);
    set_in_st(call, 0, x);
    gp4::call_regs(0x00411e17, call);

    const Ext shaped = out_st(call, 0);
    const Ext result = (ext_i32(GP4_ARRAY(int32_t, 0x0062f870)[index]) * shaped)
        * ext(GP4_GLOBAL(double, 0x006f2b9c));

    set_out_st(r, 0, result);
    r->eax = call.eax;
    r->edx = call.edx;
    r->eflags = call.eflags;
}
GP4_IMPL(0x00411da4, FUN_00411da4, "regs(ecx:u32[0..3], st0:f64) -> (eax, edx, st0, cf, pf, af, zf, sf, of); globals=layout=tyre_00601510")
