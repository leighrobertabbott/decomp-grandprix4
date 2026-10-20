#include <gp4/gp4.h>
#include <gp4/x87.h>

enum : uint32_t {
    kArithFlags = 0x8d5u,   // OF SF ZF AF PF CF
    kPF = 0x4u, kAF = 0x10u, kZF = 0x40u, kSF = 0x80u, kOF = 0x800u
};

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? kPF : 0u;
}

// Flags of a 32-bit cmp a, b.
static inline uint32_t cmpFlags(uint32_t a, uint32_t b) {
    const uint32_t result = a - b;
    return (a < b ? 1u : 0u) | ((a ^ b ^ result) & kAF) | parityFlag8(result) |
           (result == 0u ? kZF : 0u) | ((result >> 31) != 0u ? kSF : 0u) |
           ((((a ^ b) & (a ^ result)) >> 31) != 0u ? kOF : 0u);
}

// FUNCTION: GP4 0x00411e69
// ecx = wheel index (0x158-byte records at 0x00601510; 0/1 use the front table 0x0062f5e0, 2/3 the
// rear table 0x0062f808). st0 = A, st1 = B on entry. With t = the table and rec the record:
//   rec+0xa0 = sin(atan(t.f48 * B) * [0x006f2a5c]) * (cos(atan(t.f50 * A)) * rec.f98)
//   rec+0xa8 = cos(atan((A - t.f28) * t.f20)) * t.f18
//   rec+0xb0 = cos(atan((B + t.f10) * rec.a8) * t.f08) / cos(atan(rec.a8 * t.f10) * t.f08)
// and returns FUN_00411cc4(A) * rec.b0 + rec.a0 in st0.
void __cdecl FUN_00411e69(gp4::Regs* r) {
    using namespace gp4::x87;
    const uint32_t index = r->ecx;
    const uint32_t rec = static_cast<uint32_t>(static_cast<uint64_t>(index) * 0x158u) + 0x00601510u;
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(rec));
    uint8_t* table = static_cast<int32_t>(index) < 2 ? GP4_ARRAY(uint8_t, 0x0062f5e0)
                                                    : GP4_ARRAY(uint8_t, 0x0062f808);

    const Ext a = in_st(r, 0);
    const Ext b = in_st(r, 1);

    const Ext longitudinal = fcos(fpatan(ext(GP4_FIELD(double, table, 0x50)) * a, ext_one())) * ext(GP4_FIELD(double, record, 0x98));
    const Ext lateral = fsin(fpatan(ext(GP4_FIELD(double, table, 0x48)) * b, ext_one()) * ext(GP4_GLOBAL(double, 0x006f2a5c)));
    GP4_FIELD(double, record, 0xa0) = narrow(longitudinal * lateral);

    const Ext slip = fcos(fpatan((a - ext(GP4_FIELD(double, table, 0x28))) * ext(GP4_FIELD(double, table, 0x20)), ext_one()))
                     * ext(GP4_FIELD(double, table, 0x18));
    GP4_FIELD(double, record, 0xa8) = narrow(slip);

    const Ext top = fcos(fpatan((b + ext(GP4_FIELD(double, table, 0x10))) * ext(GP4_FIELD(double, record, 0xa8)), ext_one())
                         * ext(GP4_FIELD(double, table, 8)));
    const Ext bottom = fcos(fpatan(ext(GP4_FIELD(double, record, 0xa8)) * ext(GP4_FIELD(double, table, 0x10)), ext_one())
                            * ext(GP4_FIELD(double, table, 8)));
    GP4_FIELD(double, record, 0xb0) = narrow(top / bottom);

    gp4::Regs call = *r;
    call.st_in = 1;
    call.st_out = 1;
    call.st_xin_mask = 0;
    call.st_xout_mask = 0;
    call.eflags = (r->eflags & ~kArithFlags) | cmpFlags(index, 2u);
    set_in_st(call, 0, a);
    gp4::call_regs(0x00411cc4, call);

    const Ext result = out_st(call, 0) * ext(GP4_FIELD(double, record, 0xb0)) + ext(GP4_FIELD(double, record, 0xa0));
    set_out_st(r, 0, result);
    r->eax = rec;
    r->edx = call.edx;
    r->eflags = call.eflags;
}
GP4_IMPL(0x00411e69, FUN_00411e69, "regs(ecx:u32[0..3], st0:f64, st1:f64) -> (eax, edx, st0, cf, pf, af, zf, sf, of); globals=layout=tyre_00601510")
