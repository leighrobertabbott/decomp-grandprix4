#include <gp4/gp4.h>

// pushal wrapper for the car record in esi. When the byte 0x007aafd4 is set it runs 0x00444f84 and, if the byte
// 0x007c2ea9 is 0x80, 0x00445070; it then always runs 0x00408783 and 0x004415c9. popal restores every register; the
// flags are those 0x004415c9 leaves.
// FUNCTION: GP4 0x004415dd
void __cdecl FUN_004415dd(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    if (GP4_GLOBAL(uint8_t, 0x007aafd4) != 0u) {
        gp4::call_regs(0x00444f84, call);
        if (GP4_GLOBAL(uint8_t, 0x007c2ea9) == 0x80u) {
            gp4::call_regs(0x00445070, call);
        }
    }
    gp4::call_regs(0x00408783, call);
    gp4::call_regs(0x004415c9, call);
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x004415dd, FUN_004415dd, "regs(esi:ptr[0x41c]:layout=rec_0043259c) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0043c1ba+state_004320f2+stream_0062d1a0+counter_0040f0dd+scale_0066ec60")
