#include <gp4/gp4.h>
#include <gp4/flags.h>

// Moves the bits of eax, then of ebx, that are set in the mask dword +0x2c of a record into the five dwords
// +0x30, +0x38, +0x34, +0x40 and +0x3c (the bit-set dwords of a 0x160-byte ring record). After the eax bits
// have been moved they are cleared from the +0x2c mask, so the ebx pass only moves ebx bits that are still
// set. All registers are restored by popal; the flags are those of the last `or [edi+0x3c], eax`.
// FUNCTION: GP4 0x00556414
void __cdecl FUN_00556414(gp4::Regs* r) {
    const uint32_t edi = r->edi;

    const uint32_t moved = r->eax & GP4_FIELD(uint32_t, edi, 0x2c);
    GP4_FIELD(uint32_t, edi, 0x30) |= moved;
    GP4_FIELD(uint32_t, edi, 0x38) |= moved;
    GP4_FIELD(uint32_t, edi, 0x34) |= moved;
    GP4_FIELD(uint32_t, edi, 0x40) |= moved;
    GP4_FIELD(uint32_t, edi, 0x3c) |= moved;
    GP4_FIELD(uint32_t, edi, 0x2c) &= ~moved;

    const uint32_t more = r->ebx & GP4_FIELD(uint32_t, edi, 0x2c);
    GP4_FIELD(uint32_t, edi, 0x30) |= more;
    GP4_FIELD(uint32_t, edi, 0x38) |= more;
    GP4_FIELD(uint32_t, edi, 0x34) |= more;
    GP4_FIELD(uint32_t, edi, 0x40) |= more;
    const uint32_t last = GP4_FIELD(uint32_t, edi, 0x3c) | more;
    GP4_FIELD(uint32_t, edi, 0x3c) = last;

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(last));
}
GP4_IMPL(0x00556414, FUN_00556414, "regs(eax:u32, ebx:u32, edi:ptr[0x48]:bytes) -> (cf, pf, af, zf, sf, of)")
