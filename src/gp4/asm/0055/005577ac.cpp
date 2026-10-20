#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sibling of 0x0055776f using the status mask routine 0x00557788: spreads the bits found for the
// ring record in ebx and its predecessor over the five dwords +0x3c, +0x30, +0x34, +0x38 and +0x40
// of the record. pushal/popal keeps every register; the flags are those of the last
// `or dword [ebx + 0x40], eax`.
// FUNCTION: GP4 0x005577ac
void __cdecl FUN_005577ac(gp4::Regs* r) {
    const uint32_t record = r->ebx;
    gp4::Regs callee = *r;
    callee.edi = record;
    gp4::call_regs(0x00557788, callee);
    const uint32_t bits = callee.eax;
    GP4_FIELD(uint32_t, record, 0x3c) |= bits;
    GP4_FIELD(uint32_t, record, 0x30) |= bits;
    GP4_FIELD(uint32_t, record, 0x34) |= bits;
    GP4_FIELD(uint32_t, record, 0x38) |= bits;
    GP4_FIELD(uint32_t, record, 0x40) |= bits;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(GP4_FIELD(uint32_t, record, 0x40)));
}
GP4_IMPL(0x005577ac, FUN_005577ac, "regs(ebx:rec[0x8943a0,0x160,0..29]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+bits_0055616f+flags2c_0055774b")
