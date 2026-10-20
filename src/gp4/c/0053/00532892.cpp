#include <gp4/gp4.h>
#include <gp4/flags.h>

#pragma pack(push, 1)
struct Unknown_827f8b {
    uint32_t Unknown00;
    uint32_t Unknown04;
};
#pragma pack(pop)

// Fills the record at 0x00827f8b (first dword cleared, second dword = ecx) and pushes it on the menu
// stack with 0x0054faff (al = 2, esi = the record). Registers and flags come back as the callee leaves
// them.
// FUNCTION: GP4 0x00532892
void __cdecl FUN_00532892(gp4::Regs* r) {
    Unknown_827f8b* rec = &GP4_GLOBAL(Unknown_827f8b, 0x00827f8b);
    rec->Unknown00 = 0u;
    rec->Unknown04 = r->ecx;
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.esi = 0x00827f8bu;
    c.eax = (c.eax & 0xffffff00u) | 2u;
    gp4::call_regs(0x0054faff, c);
    *r = c;
}
GP4_IMPL(0x00532892, FUN_00532892, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=case_0054fd4d+anim_0052822a+slots_005388ed+cells_00527eae+slot8_005388ba+surfcopy_00549d55+frame_0082fd70+list_00549db9; wall=3600")
