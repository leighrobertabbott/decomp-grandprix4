#include <gp4/gp4.h>
#include <gp4/flags.h>

// Flags of `shl value, 6` in the oracle model: CF = last bit shifted out, AF = 0,
// OF = bit31((value << 5) ^ (value << 6)); ZF/SF/PF come from the result.
static uint32_t Shl6Flags(uint32_t value) {
    const uint32_t result = value << 6;
    const uint32_t carry = (value >> 26) & 1u;
    const uint32_t overflow = ((value << 5) ^ result) >> 31;
    return gp4::flags::logic32(result) | carry | (overflow << 11);
}

// Second-vector twin of 0x0055671d: rotates the record's vector through 0x0055669e with ebx = 1 (the
// positive lateral side): edx = the length dword +0x10c, eax = the angle dword +0x110, ecx = the word
// +0x114 << 6. The resulting eax/edx are stored in [0x00934df0]/[0x00934df4]; eax is then reloaded
// with the sign-extended word +0x116 << 6, ecx keeps the value 0x0055669e left, ebx and edx are
// restored. The flags are those of the final `shl eax, 6`.
// FUNCTION: GP4 0x00556757
void __cdecl FUN_00556757(gp4::Regs* r) {
    const uint32_t record = r->edi;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.ebx = 1u;
    c.edx = GP4_FIELD(uint32_t, record, 0x10c);
    c.eax = GP4_FIELD(uint32_t, record, 0x110);
    c.ecx = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, record, 0x114))) << 6;
    gp4::call_regs(0x0055669e, c);
    GP4_GLOBAL(uint32_t, 0x00934df0) = c.eax;
    GP4_GLOBAL(uint32_t, 0x00934df4) = c.edx;

    const uint32_t value = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, record, 0x116)));
    r->eax = value << 6;
    r->ecx = c.ecx;
    r->eflags = gp4::flags::merge(r->eflags, Shl6Flags(value));
}
GP4_IMPL(0x00556757, FUN_00556757, "regs(edi:ptr[0x158]:bytes) -> (eax, ecx, cf, pf, af, zf, sf, of); globals=fuzz:f64+layout=multiply_0040ce7b")
