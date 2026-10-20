#include <gp4/gp4.h>
#include <gp4/flags.h>

// Ring-record marker: when the record pointer edi lies above the high-water mark [0x00871ca4]
// (unsigned), the pending dword mask [0x008705f4] is or-ed into the record's +0x2c, the pending
// byte mask [0x0087062f] into its +0x67, and the mark moves up to edi. On that path eax = the dword
// mask and dl = the byte mask (the upper bits of edx are kept). Flags: `cmp edi, [mark]` when the
// record is not above the mark, else those of the final `or byte [edi + 0x67], dl`.
// FUNCTION: GP4 0x00557f0e
void __cdecl FUN_00557f0e(gp4::Regs* r) {
    const uint32_t record = r->edi;
    uint32_t arith = gp4::flags::sub32(record, GP4_GLOBAL(uint32_t, 0x00871ca4));
    if (record > GP4_GLOBAL(uint32_t, 0x00871ca4)) {
        const uint32_t dwordMask = GP4_GLOBAL(uint32_t, 0x008705f4);
        GP4_FIELD(uint32_t, record, 0x2c) |= dwordMask;
        r->eax = dwordMask;
        const uint8_t byteMask = GP4_GLOBAL(uint8_t, 0x0087062f);
        r->edx = (r->edx & 0xffffff00u) | byteMask;
        const uint8_t merged = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x67) | byteMask);
        GP4_FIELD(uint8_t, record, 0x67) = merged;
        arith = gp4::flags::logic8(merged);
        GP4_GLOBAL(uint32_t, 0x00871ca4) = record;
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00557f0e, FUN_00557f0e, "regs(edx:u32, edi:ptr[0x70]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=hwm_00557f0e")
