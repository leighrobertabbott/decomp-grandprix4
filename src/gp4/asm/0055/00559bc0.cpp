#include <gp4/gp4.h>
#include <gp4/flags.h>

// ORs eax into five dwords of the record at edi (+0x3c, +0x30, +0x34, +0x38, +0x40). The flags are
// those of the last `or dword [edi + 0x40], eax`.
// FUNCTION: GP4 0x00559bc0
void __cdecl FUN_00559bc0(gp4::Regs* r) {
    const uint32_t bits = r->eax;

    GP4_FIELD(uint32_t, r->edi, 0x3c) |= bits;
    GP4_FIELD(uint32_t, r->edi, 0x30) |= bits;
    GP4_FIELD(uint32_t, r->edi, 0x34) |= bits;
    GP4_FIELD(uint32_t, r->edi, 0x38) |= bits;
    const uint32_t last = GP4_FIELD(uint32_t, r->edi, 0x40) | bits;
    GP4_FIELD(uint32_t, r->edi, 0x40) = last;

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(last));
}
GP4_IMPL(0x00559bc0, FUN_00559bc0, "regs(eax:u32, edi:ptr[0x48]:bytes) -> (cf, pf, af, zf, sf, of)")
