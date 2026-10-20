#include <gp4/gp4.h>

// eax = blend fraction (14-bit), ecx = mode (0..2), edx = side (0 / -1), ebx/esi = car state used by
// the callees, edi = track segment record of the 0x160-byte ring at 0x008943a0, ebp = second
// operand of the final interpolator. Stores the fraction in DAT_00601c18 and (temporarily) in
// DAT_00601b34, blends the segment words of edi and edi + 0x160 (wrapped like the ring walkers)
// returned by 0x00431ffb, converts the result through 0x00403938 and finishes in 0x00403b65 or
// 0x00403c4c (selected by the byte DAT_00601ba6). ebp, ecx and edx come back unchanged, DAT_00601b34
// is restored; eax and the flags are those of the last interpolator.
// FUNCTION: GP4 0x00403524
void __cdecl FUN_00403524(gp4::Regs* r) {
    const uint32_t savedFraction = GP4_GLOBAL(uint32_t, 0x00601b34);
    GP4_GLOBAL(uint32_t, 0x00601c18) = r->eax;
    GP4_GLOBAL(uint32_t, 0x00601b34) = r->eax;

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    s.eax = s.ecx;
    gp4::call_regs(0x0040350c, s);
    if (!(s.eflags & 1u)) {
        s.ecx = s.eax;
        s.eax = s.ebp;
        gp4::call_regs(0x00429284, s);
        s.ebp = s.eax;
        s.eax = s.ecx;
    }

    const uint32_t savedEdx = s.edx;
    const uint32_t savedEax = s.eax;
    const uint32_t savedEdi = s.edi;

    gp4::call_regs(0x00431ffb, s);
    s.ecx = s.eax;

    s.edi += 0x160u;
    if (!(s.edi < GP4_GLOBAL(uint32_t, 0x00866ecc))) {
        if (s.edi < GP4_GLOBAL(uint32_t, 0x00891500)) {
            s.edi -= GP4_GLOBAL(uint32_t, 0x00866ec4);
        } else if (!(s.edi < GP4_GLOBAL(uint32_t, 0x00866ef8))) {
            s.edi = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
        }
    }
    gp4::call_regs(0x00431ffb, s);

    s.edi = savedEdi;
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(s.eax - s.ecx))
        * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00601c18));
    const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    s.eax = ((static_cast<uint32_t>(product) >> 14) | (high << 18)) + s.ecx;

    s.edx = savedEax;
    gp4::call_regs(0x00403938, s);

    s.edx = savedEdx;
    if (GP4_GLOBAL(uint8_t, 0x00601ba6) != 0u) {
        gp4::call_regs(0x00403c4c, s);
    } else {
        gp4::Regs t = s;
        gp4::call_regs(0x00403b65, t);
        s = t;
    }

    GP4_GLOBAL(uint32_t, 0x00601b34) = savedFraction;
    r->eax = s.eax;
    r->ebx = s.ebx;
    r->esi = s.esi;
    r->edi = s.edi;
    r->eflags = (r->eflags & ~0x8d5u) | (s.eflags & 0x8d5u);
}
GP4_IMPL(0x00403524, FUN_00403524, "regs(eax:i32[-0x4000..0x4000], ecx:u32[0..2], edx:i32[-1..0], ebp:i32[-0x4000..0x4000], esi:ptr[0x273]:layout=car_00431ffb, edi:rec[0x8943a0,0x160,0..17]) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+pitw_00852c44")
