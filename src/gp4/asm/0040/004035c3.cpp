#include <gp4/gp4.h>

struct Unknown_004035c3_Car {
    uint8_t Unknown00[0x71];
    uint8_t Unknown71;
    uint8_t Unknown72[0x1a0 - 0x72];
    uint32_t Unknown1A0;
    uint8_t Unknown1A4[0x294 - 0x1a4];
    uint8_t Unknown294;
};

// FUNCTION: GP4 0x004035c3
// eax = blend fraction, esi = car record, edi = track segment record. Loads ebp = [esi+0x1a0],
// ecx = byte [esi+0x294] (mode) and edx = -1 when bit 3 of [esi+0x71] is set (else 0), then runs
// 0x00403524 with DAT_00601b34 temporarily holding the fraction (restored on return). eax, ebx, esi,
// edi and the flags are those the callee leaves.
void __cdecl FUN_004035c3(gp4::Regs* r) {
    const Unknown_004035c3_Car* car = reinterpret_cast<const Unknown_004035c3_Car*>(static_cast<uintptr_t>(r->esi));

    const uint32_t savedFraction = GP4_GLOBAL(uint32_t, 0x00601b34);
    GP4_GLOBAL(uint32_t, 0x00601b34) = r->eax;

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.ebp = car->Unknown1A0;
    call.ecx = car->Unknown294;
    call.edx = (car->Unknown71 & 8u) != 0u ? 0xffffffffu : 0u;
    gp4::call_regs(0x00403524, call);

    GP4_GLOBAL(uint32_t, 0x00601b34) = savedFraction;

    r->eax = call.eax;
    r->ebx = call.ebx;
    r->ecx = call.ecx;
    r->edx = call.edx;
    r->esi = call.esi;
    r->edi = call.edi;
    r->ebp = call.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x004035c3, FUN_004035c3, "regs(eax:i32[-0x4000..0x4000], esi:ptr[0x29c]:layout=car_00431ffb+car_004035c3, edi:rec[0x8943a0,0x160,0..17]) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44")
