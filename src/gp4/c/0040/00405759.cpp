#include <gp4/gp4.h>
#include <gp4/x87.h>

using gp4::x87::Ext;

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// flags of "test dword, imm" (CF = OF = AF = 0)
static inline uint32_t testFlags32(uint32_t value) {
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value >> 31) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x00405759
void __cdecl FUN_00405759(gp4::Regs* r) {
    const uint8_t* body = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const int32_t scaleCount = GP4_GLOBAL(int32_t, 0x00602d7c);

    if (scaleCount == 0) {
        r->eax = 0x00770000u;
        r->eflags = (r->eflags & ~0x8d5u) | testFlags32(0u);
        return;
    }

    // divisor = scaleCount * [6f29cc] * word[esi+0xb2], dividend = [602d70] * [6f2bb4] * [6f2a8c]
    const uint32_t word = *reinterpret_cast<const uint16_t*>(body + 0xb2);
    const Ext divisor = gp4::x87::ext_i32(scaleCount) * gp4::x87::ext(GP4_GLOBAL(double, 0x006f29cc))
        * gp4::x87::ext_i32(static_cast<int32_t>(word));
    const Ext dividend = gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x00602d70)) * gp4::x87::ext(GP4_GLOBAL(double, 0x006f2bb4))
        * gp4::x87::ext(GP4_GLOBAL(double, 0x006f2a8c));

    gp4::Regs call = *r;
    call.eflags = (r->eflags & ~0x8d5u) | testFlags32(static_cast<uint32_t>(scaleCount));
    gp4::x87::set_in_st(call, 0, gp4::x87::ext_i32(0x37510));
    gp4::x87::set_in_st(call, 1, divisor);
    gp4::x87::set_in_st(call, 2, dividend);
    call.st_in = 3;
    call.st_out = 1;
    gp4::call_regs(0x00401ccc, call);

    Ext ratio = gp4::x87::out_st(call, 0);
    ratio.b[9] &= 0x7f;  // FABS
    const Ext scaled = gp4::x87::fsqrt(ratio) * gp4::x87::ext(GP4_GLOBAL(double, 0x006f2b04));
    const int32_t result = gp4::x87::fistp32(scaled);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = result;

    r->eax = static_cast<uint32_t>(result);
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x00405759, FUN_00405759, "regs(esi:ptr[0xba]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
