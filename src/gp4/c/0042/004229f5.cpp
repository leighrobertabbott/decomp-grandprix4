#include <gp4/gp4.h>

// Emits one (count < 2), two (count == 2) or three (count > 2) points through the
// rotate-and-offset routine 0x00422935: the angle in eax is DAT_006082dc, then plus and minus
// DAT_00608300, and for the third point DAT_006082dc + 0x80000000. esi counts the points and
// edi advances by 0x14 each.
// FUNCTION: GP4 0x004229f5
void __cdecl FUN_004229f5(gp4::Regs* r) {
    if (GP4_GLOBAL(int32_t, 0x006082b8) < 2) {
        r->eax = GP4_GLOBAL(uint32_t, 0x006082dc);
        gp4::call_regs(0x00422935, *r);
    } else if (GP4_GLOBAL(int32_t, 0x006082b8) == 2) {
        r->eax = GP4_GLOBAL(uint32_t, 0x006082dc) + GP4_GLOBAL(uint32_t, 0x00608300);
        gp4::call_regs(0x00422935, *r);
        r->eax = GP4_GLOBAL(uint32_t, 0x006082dc) - GP4_GLOBAL(uint32_t, 0x00608300);
        gp4::call_regs(0x00422935, *r);
    } else {
        r->eax = GP4_GLOBAL(uint32_t, 0x006082dc) + GP4_GLOBAL(uint32_t, 0x00608300);
        gp4::call_regs(0x00422935, *r);
        r->eax = GP4_GLOBAL(uint32_t, 0x006082dc) - GP4_GLOBAL(uint32_t, 0x00608300);
        gp4::call_regs(0x00422935, *r);
        r->eax = GP4_GLOBAL(uint32_t, 0x006082dc) + 0x80000000u;
        gp4::call_regs(0x00422935, *r);
    }
}
GP4_IMPL(0x004229f5, FUN_004229f5, "regs(ebx:ptr[0x40]:bytes, edi:ptr[0x60]:bytes, esi:u32) -> (eax, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
