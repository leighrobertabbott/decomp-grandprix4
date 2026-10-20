#include <gp4/gp4.h>

// Emits one point (count < 2: a plain copy through 0x00422991), two points (count == 2) or
// three points (count > 2) of a ring around the source point through the rotate-and-offset
// routine 0x00422935. eax carries the angle; esi counts the points and edi advances by 0x14 each.
// FUNCTION: GP4 0x00422a65
void __cdecl FUN_00422a65(gp4::Regs* r) {
    if (GP4_GLOBAL(int32_t, 0x006082b8) < 2) {
        gp4::call_regs(0x00422991, *r);
    } else if (GP4_GLOBAL(int32_t, 0x006082b8) == 2) {
        r->eax = GP4_GLOBAL(uint32_t, 0x006082dc);
        gp4::call_regs(0x00422935, *r);
        r->eax = GP4_GLOBAL(uint32_t, 0x006082dc) - 0x80000000u;
        gp4::call_regs(0x00422935, *r);
    } else {
        r->eax = GP4_GLOBAL(uint32_t, 0x006082dc) + 0x55555555u;
        gp4::call_regs(0x00422935, *r);
        r->eax = GP4_GLOBAL(uint32_t, 0x006082dc);
        gp4::call_regs(0x00422935, *r);
        r->eax = GP4_GLOBAL(uint32_t, 0x006082dc) - 0x55555555u;
        gp4::call_regs(0x00422935, *r);
    }
}
GP4_IMPL(0x00422a65, FUN_00422a65, "regs(ebx:ptr[0x40]:bytes, edi:ptr[0x60]:bytes, esi:u32) -> (eax, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
