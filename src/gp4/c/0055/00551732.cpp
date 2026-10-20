#include <gp4/gp4.h>
#include <gp4/flags.h>

// Resets the selected car's phase state. Only when the master mode byte [0x0066d7a4] is 0 or 1
// (`cmp byte, 1; ja` leaves the flags of that compare otherwise): esi = the selected car record
// [0x00679d30]; 0x004081c2 tests bit 0x40 of its byte +0x7b and reports it in CF. CF set runs the phase
// step 0x0055263a; CF clear writes 0x8000 to the three words 0x0066d882/84/86 (the mov stores keep the
// flags of 0x004081c2).
// FUNCTION: GP4 0x00551732
void __cdecl FUN_00551732(gp4::Regs* r) {
    const uint32_t mode = GP4_GLOBAL(uint8_t, 0x0066d7a4);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(mode, 1u));
    if (mode > 1u) {
        return;
    }
    r->esi = GP4_GLOBAL(uint32_t, 0x00679d30);
    gp4::call_regs(0x004081c2, *r);
    if ((r->eflags & gp4::flags::CF) != 0u) {
        gp4::call_regs(0x0055263a, *r);
    } else {
        GP4_GLOBAL(uint16_t, 0x0066d882) = 0x8000u;
        GP4_GLOBAL(uint16_t, 0x0066d884) = 0x8000u;
        GP4_GLOBAL(uint16_t, 0x0066d886) = 0x8000u;
    }
}
GP4_IMPL(0x00551732, FUN_00551732, "regs(edx:u32) -> (eax, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=gate_0051ae76+ptr_00679d30")
