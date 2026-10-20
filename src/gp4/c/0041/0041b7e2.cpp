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

// Flags of "shrd eax, edx, 14" (oracle model: AF = 0, OF = bit31(prev ^ result)).
static inline uint32_t shrd14Flags(uint32_t high, uint32_t low) {
    const uint64_t pair = (static_cast<uint64_t>(high) << 32) | low;
    const uint32_t previous = static_cast<uint32_t>(pair >> 13);
    const uint32_t result = static_cast<uint32_t>(pair >> 14);
    return (previous & 1u) | parityFlag8(result) | (result == 0u ? kZF : 0u) |
           ((result >> 31) != 0u ? kSF : 0u) | ((((previous ^ result) >> 31) != 0u) ? kOF : 0u);
}

// FUNCTION: GP4 0x0041b7e2
// Clears 0x00623c1c. level = [0x00623c34] >> 8 (ecx). When level >= [0x00601310] and
// [0x006258c4] < [0x00624790], factor = 0x4000 - (([0x006258c4] << 14) / [0x00624790]) (ebp) and
// [0x00623c1c] = (fistp(table(level) * [0x006013f8]) * factor) >> 14 (low 32 bits).
void __cdecl FUN_0041b7e2(gp4::Regs* r) {
    using namespace gp4::x87;
    GP4_GLOBAL(uint32_t, 0x00623c1c) = 0;

    const int32_t level = GP4_GLOBAL(int32_t, 0x00623c34) >> 8;
    r->ecx = static_cast<uint32_t>(level);
    uint32_t flags = cmpFlags(static_cast<uint32_t>(level), GP4_GLOBAL(uint32_t, 0x00601310));
    if (level >= GP4_GLOBAL(int32_t, 0x00601310)) {
        const int32_t position = GP4_GLOBAL(int32_t, 0x006258c4);
        r->eax = static_cast<uint32_t>(position);
        flags = cmpFlags(static_cast<uint32_t>(position), GP4_GLOBAL(uint32_t, 0x00624790));
        if (position < GP4_GLOBAL(int32_t, 0x00624790)) {
            int32_t remainder;
            const int32_t dividend = static_cast<int32_t>(static_cast<uint32_t>(position) << 14);
            const int32_t quotient = gp4::idiv64(static_cast<int64_t>(dividend), GP4_GLOBAL(int32_t, 0x00624790), &remainder);
            const int32_t factor = 0x4000 - quotient;
            r->ebp = static_cast<uint32_t>(factor);

            GP4_GLOBAL(int32_t, 0x006f2bf4) = level;
            gp4::Regs call = *r;
            call.edx = 0x0062f008u;
            call.st_in = 1;
            call.st_out = 1;
            call.st_xin_mask = 0;
            call.st_xout_mask = 0;
            set_in_st(call, 0, ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4)));
            gp4::call_regs(0x00411891, call);

            const Ext scaled = out_st(call, 0) * ext(GP4_GLOBAL(double, 0x006013f8));
            GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(scaled);

            const int64_t product = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006f2bf4)) * factor;
            const uint32_t low = static_cast<uint32_t>(product);
            const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
            const uint32_t result = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
            r->eax = result;
            r->edx = high;
            flags = shrd14Flags(high, low);
            GP4_GLOBAL(uint32_t, 0x00623c1c) = result;
        }
    }
    r->eflags = (r->eflags & ~kArithFlags) | flags;
}
GP4_IMPL(0x0041b7e2, FUN_0041b7e2, "regs() -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=frac_00624790")
