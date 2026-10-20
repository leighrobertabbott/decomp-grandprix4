#include <gp4/gp4.h>
#include <gp4/flags.h>

// Tests whether the text at esi (after blanks, skipped by 0x005146e8) starts with "D" or "d", then "B" or
// "b", then a blank (space or tab). CF = 0 when it does (esi is left on the blank), CF = 1 otherwise (esi is
// then on the byte before the one that failed). al holds the byte 0x005146e8 returned, or the second letter
// read by `lodsb`. The other flags are those of the last compare (or of the `dec`/`sub esi` before `stc`).
// FUNCTION: GP4 0x00514705
void __cdecl FUN_00514705(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    gp4::call_regs(0x005146e8, c);

    uint32_t esi = c.esi;
    uint32_t eax = c.eax;
    uint32_t al = eax & 0xffu;
    uint32_t arith;
    uint32_t carry;
    if (al == 0x44u || al == 0x64u) {
        const uint8_t* p = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(esi));
        al = *p;
        esi += 1u;
        eax = (eax & 0xffffff00u) | al;
        if (al == 0x42u || al == 0x62u) {
            const uint32_t next = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(esi));
            if (next == 0x20u) {
                arith = gp4::flags::sub8(next, 0x20u);
                carry = 0u;
            } else if (next == 0x09u) {
                arith = gp4::flags::sub8(next, 0x09u);
                carry = 0u;
            } else {
                arith = gp4::flags::sub32(esi, 2u);
                esi -= 2u;
                carry = 1u;
            }
        } else {
            arith = gp4::flags::sub32(esi, 2u);
            esi -= 2u;
            carry = 1u;
        }
    } else {
        arith = gp4::flags::dec32(esi, gp4::flags::sub8(al, 0x64u));
        esi -= 1u;
        carry = 1u;
    }

    r->eax = eax;
    r->esi = esi;
    r->eflags = gp4::flags::merge(r->eflags, (arith & ~static_cast<uint32_t>(gp4::flags::CF)) | carry);
}
GP4_IMPL(0x00514705, FUN_00514705, "regs(eax:u32, esi:ptr[0x40]:u8{=0x44,0x64,0x42,0x62,0x20,0x09,0x44,0x42,0x20,0x41}) -> (eax, esi, cf, pf, af, zf, sf, of)")
