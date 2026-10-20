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

// FUNCTION: GP4 0x00411f09
// ecx = wheel index (0x158-byte records at 0x00601510; 0/1 use the front table 0x0062f5b8, 2/3 the
// rear table 0x0062f7e0). st1 = X, st0 = Y on entry. Stores
//   rec+0xb8 = cos(atan(X * t.f20)) * t.f18
//   rec+0xc0 = cos(atan((Y + t.f10) * rec.b8) * t.f08) / cos(atan(rec.b8 * t.f10) * t.f08)
// and returns FUN_00411da4(X) * rec.c0 in st0.
void __cdecl FUN_00411f09(gp4::Regs* r) {
    using namespace gp4::x87;
    const uint32_t index = r->ecx;
    const uint32_t rec = static_cast<uint32_t>(static_cast<uint64_t>(index) * 0x158u) + 0x00601510u;
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(rec));
    uint8_t* table = static_cast<int32_t>(index) < 2 ? GP4_ARRAY(uint8_t, 0x0062f5b8)
                                                          : GP4_ARRAY(uint8_t, 0x0062f7e0);

    const Ext y = in_st(r, 0);
    const Ext x = in_st(r, 1);

    const Ext slip = fcos(fpatan(x * ext(GP4_FIELD(double, table, 0x20)), ext_one())) * ext(GP4_FIELD(double, table, 0x18));
    GP4_FIELD(double, record, 0xb8) = narrow(slip);

    const Ext top = fcos(fpatan((y + ext(GP4_FIELD(double, table, 0x10))) * ext(GP4_FIELD(double, record, 0xb8)), ext_one())
                         * ext(GP4_FIELD(double, table, 8)));
    const Ext bottom = fcos(fpatan(ext(GP4_FIELD(double, record, 0xb8)) * ext(GP4_FIELD(double, table, 0x10)), ext_one())
                            * ext(GP4_FIELD(double, table, 8)));
    GP4_FIELD(double, record, 0xc0) = narrow(top / bottom);

    gp4::Regs call = *r;
    call.st_in = 1;
    call.st_out = 1;
    call.st_xin_mask = 0;
    call.st_xout_mask = 0;
    call.eflags = (r->eflags & ~kArithFlags) | cmpFlags(index, 2u);
    set_in_st(call, 0, x);
    gp4::call_regs(0x00411da4, call);

    const Ext result = out_st(call, 0) * ext(GP4_FIELD(double, record, 0xc0));
    set_out_st(r, 0, result);
    r->eax = rec;
    r->edx = call.edx;
    r->eflags = call.eflags;
}
GP4_IMPL(0x00411f09, FUN_00411f09, "regs(ecx:u32[0..3], st0:f64, st1:f64) -> (eax, edx, st0, cf, pf, af, zf, sf, of); globals=layout=tyre_00601510")
