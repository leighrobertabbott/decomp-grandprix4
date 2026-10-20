#include <gp4/gp4.h>
#include <gp4/flags.h>

// ORs ax into five words of the record at edi (+0x3c, +0x30, +0x34, +0x38, +0x40). The flags are
// those of the last `or word [edi + 0x40], ax`; the upper half of eax is untouched.
// FUNCTION: GP4 0x00559bab
void __cdecl FUN_00559bab(gp4::Regs* r) {
    const uint16_t bits = static_cast<uint16_t>(r->eax);

    GP4_FIELD(uint16_t, r->edi, 0x3c) |= bits;
    GP4_FIELD(uint16_t, r->edi, 0x30) |= bits;
    GP4_FIELD(uint16_t, r->edi, 0x34) |= bits;
    GP4_FIELD(uint16_t, r->edi, 0x38) |= bits;
    const uint16_t last = static_cast<uint16_t>(GP4_FIELD(uint16_t, r->edi, 0x40) | bits);
    GP4_FIELD(uint16_t, r->edi, 0x40) = last;

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic16(last));
}
GP4_IMPL(0x00559bab, FUN_00559bab, "regs(eax:u32, edi:ptr[0x48]:bytes) -> (cf, pf, af, zf, sf, of)")
