#include <gp4/gp4.h>
#include <gp4/flags.h>

// Clears the bits of eax from the six mask dwords +0x2c..+0x40 of a record (the bit-set dwords of a
// 0x160-byte ring record). eax is left as ~eax; the flags are those of the last `and [edi+0x3c], eax`.
// FUNCTION: GP4 0x00556442
void __cdecl FUN_00556442(gp4::Regs* r) {
    const uint32_t keep = ~r->eax;
    const uint32_t edi = r->edi;

    GP4_FIELD(uint32_t, edi, 0x2c) = GP4_FIELD(uint32_t, edi, 0x2c) & keep;
    GP4_FIELD(uint32_t, edi, 0x30) = GP4_FIELD(uint32_t, edi, 0x30) & keep;
    GP4_FIELD(uint32_t, edi, 0x38) = GP4_FIELD(uint32_t, edi, 0x38) & keep;
    GP4_FIELD(uint32_t, edi, 0x34) = GP4_FIELD(uint32_t, edi, 0x34) & keep;
    GP4_FIELD(uint32_t, edi, 0x40) = GP4_FIELD(uint32_t, edi, 0x40) & keep;
    const uint32_t last = GP4_FIELD(uint32_t, edi, 0x3c) & keep;
    GP4_FIELD(uint32_t, edi, 0x3c) = last;

    r->eax = keep;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(last));
}
GP4_IMPL(0x00556442, FUN_00556442, "regs(eax:u32, edi:ptr[0x48]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
