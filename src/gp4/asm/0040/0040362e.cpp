#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040362e
// Wrapper around 0x00403524 for the car record in esi: the blend fraction in eax is parked in
// [0x00601b34] for the call (restored afterwards), ebp = dword +0x290, ecx = byte +0x1cf (mode) and
// edx = -1 when bit 7 of byte +0x1ce is set, else 0. Registers and flags are whatever 0x00403524 leaves.
void __cdecl FUN_0040362e(gp4::Regs* r) {
    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t saved = GP4_GLOBAL(uint32_t, 0x00601b34);
    GP4_GLOBAL(uint32_t, 0x00601b34) = r->eax;

    gp4::Regs call = *r;
    call.ebp = GP4_FIELD(uint32_t, car, 0x290);
    call.ecx = GP4_FIELD(uint8_t, car, 0x1cf);
    call.edx = (GP4_FIELD(uint8_t, car, 0x1ce) & 0x80u) != 0u ? 0xffffffffu : 0u;
    gp4::call_regs(0x00403524, call);
    *r = call;

    GP4_GLOBAL(uint32_t, 0x00601b34) = saved;
}
GP4_IMPL(0x0040362e, FUN_0040362e, "regs(eax:i32[-0x4000..0x4000], esi:ptr[0x298]:layout=car_00431ffb+car_0040362e, edi:rec[0x8943a0,0x160,0..17]) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44")
