#include <gp4/gp4.h>

// Thin wrapper around 0x00403524 for the car record in esi: the blend fraction in eax is kept in
// DAT_00601b34 around the call, ebp = dword [esi+0x1a4], ecx = byte [esi+0x272] (the mode) and
// edx = -1 when bit 7 of byte [esi+0xfe] is set, else 0. The registers set up here come back from
// 0x00403524 unchanged; eax and the flags are its result.
// FUNCTION: GP4 0x004035f7
void __cdecl FUN_004035f7(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t savedFraction = GP4_GLOBAL(uint32_t, 0x00601b34);
    GP4_GLOBAL(uint32_t, 0x00601b34) = r->eax;

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    s.ebp = GP4_FIELD(uint32_t, car, 0x1a4);
    s.ecx = GP4_FIELD(uint8_t, car, 0x272);
    s.edx = (GP4_FIELD(uint8_t, car, 0xfe) & 0x80u) ? 0xffffffffu : 0u;
    gp4::call_regs(0x00403524, s);

    GP4_GLOBAL(uint32_t, 0x00601b34) = savedFraction;
    *r = s;
}
GP4_IMPL(0x004035f7, FUN_004035f7, "regs(eax:i32[-0x4000..0x4000], esi:ptr[0x273]:layout=car_00431ffb+car_004035f7, edi:rec[0x8943a0,0x160,0..17]) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44")
