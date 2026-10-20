#include <gp4/gp4.h>

// Re-arms the camera/driver view on the player car [0x00679d30]: when byte +0x7a has bit 5, the first
// free car record found by 0x00409aec (esi) is stored at 0x00852d3c and bytes 0x00852d57/0x00852d5c
// are reset to 0x80/0; then byte 0x0066d8aa = 0x80 and the 24-hour deadline is armed by 0x00442cb4,
// whose flags are left. esi ends as the player car, or as the record returned by 0x00409aec.
// FUNCTION: GP4 0x00442c82
void __cdecl FUN_00442c82(gp4::Regs* r) {
    const uint32_t player = GP4_GLOBAL(uint32_t, 0x00679d30);
    const uint8_t* car = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(player));
    r->esi = player;

    if ((car[0x7a] & 0x20u) != 0u) {
        gp4::Regs scan = *r;
        scan.st_in = 0;
        scan.st_out = 0;
        gp4::call_regs(0x00409aec, scan);
        r->esi = scan.esi;
        GP4_GLOBAL(uint32_t, 0x00852d3c) = scan.esi;
        GP4_GLOBAL(uint8_t, 0x00852d57) = 0x80;
        GP4_GLOBAL(uint8_t, 0x00852d5c) = 0;
    }

    GP4_GLOBAL(uint8_t, 0x0066d8aa) = 0x80;

    gp4::Regs arm = *r;
    arm.st_in = 0;
    arm.st_out = 0;
    gp4::call_regs(0x00442cb4, arm);
    r->eflags = (r->eflags & ~0x8d5u) | (arm.eflags & 0x8d5u);
}
GP4_IMPL(0x00442c82, FUN_00442c82, "regs() -> (esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=gap_0043530b+flags_00408f7b+scan_00409aec+clock_0066d814")
