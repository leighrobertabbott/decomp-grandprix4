#include <gp4/gp4.h>
#include <gp4/flags.h>

// Flags-only test on a record at edi: the low 11 bits of the word at +0x1a are compared with 0x432.
// eax and edx are pushed and restored around the routine, so the loads of the words at +0x76 and
// +0x80 on the equal path (into ax / dx) leave no register effect; every register is unchanged and
// the flags are those of `cmp (word & 0x7ff), 0x432`.
// FUNCTION: GP4 0x0055b845
void __cdecl FUN_0055b845(gp4::Regs* r) {
    const gp4::Regs saved = *r;
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    const uint32_t kind = static_cast<uint32_t>(GP4_FIELD(uint16_t, record, 0x1a)) & 0x7ffu;
    const uint32_t arith = gp4::flags::sub32(kind, 0x432u);
    if (kind == 0x432u) {
        r->eax = (r->eax & 0xffff0000u) | GP4_FIELD(uint16_t, record, 0x76);
        r->edx = (r->edx & 0xffff0000u) | GP4_FIELD(uint16_t, record, 0x80);
    }
    *r = saved;
    r->eflags = gp4::flags::merge(saved.eflags, arith);
}
GP4_IMPL(0x0055b845, FUN_0055b845, "regs(edi:ptr[0x88]:u8{0x32,0x04}) -> (cf, pf, af, zf, sf, of)")
