#include <gp4/gp4.h>

// Emits the points of a ring (count in DAT_006082b8) around the source point at ebx: for a count
// up to 2 one point at angle DAT_006082dc through 0x0042286d; above 2 two points at angles
// DAT_006082dc + DAT_00608300 (0x0042286d) and DAT_006082dc - DAT_00608300 (0x004228df). eax, esi,
// edi and the flags are whatever the last callee leaves.
// FUNCTION: GP4 0x004229b2
void __cdecl FUN_004229b2(gp4::Regs* r) {
    if (GP4_GLOBAL(int32_t, 0x006082b8) < 2) {
        r->eax = GP4_GLOBAL(uint32_t, 0x006082dc);
        gp4::call_regs(0x0042286d, *r);
    } else if (GP4_GLOBAL(int32_t, 0x006082b8) == 2) {
        r->eax = GP4_GLOBAL(uint32_t, 0x006082dc);
        gp4::call_regs(0x0042286d, *r);
    } else {
        r->eax = GP4_GLOBAL(uint32_t, 0x006082dc) + GP4_GLOBAL(uint32_t, 0x00608300);
        gp4::call_regs(0x0042286d, *r);
        r->eax = GP4_GLOBAL(uint32_t, 0x006082dc) - GP4_GLOBAL(uint32_t, 0x00608300);
        gp4::call_regs(0x004228df, *r);
    }
}
GP4_IMPL(0x004229b2, FUN_004229b2, "regs(ebx:ptr[0x40]:bytes, edi:ptr[0x50]:bytes, esi:u32) -> (eax, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
