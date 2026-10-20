#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the byte at 0x0082456b is not 0xff, pushes a menu level with 0x0054faff (al = 3, esi = the
// record 0x008334b3; the new level reaches the message pump, PeekMessageA) and sets the byte back to
// 0xff. Otherwise only the flags of `cmp byte, 0xff` change.
// FUNCTION: GP4 0x0054af35
void __cdecl FUN_0054af35(gp4::Regs* r) {
    const uint32_t cur = GP4_GLOBAL(uint8_t, 0x0082456b);
    const uint32_t cmpFlags = gp4::flags::sub8(cur, 0xffu);
    if (cur != 0xffu) {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.eflags = gp4::flags::merge(c.eflags, cmpFlags);
        c.esi = 0x008334b3u;
        c.eax = (c.eax & 0xffffff00u) | 3u;
        gp4::call_regs(0x0054faff, c);
        GP4_GLOBAL(uint8_t, 0x0082456b) = 0xffu;
        *r = c;
        return;
    }
    r->eflags = gp4::flags::merge(r->eflags, cmpFlags);
}
GP4_IMPL(0x0054af35, FUN_0054af35, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=case_0054fd4d+anim_0052822a+slots_005388ed+cells_00527eae+slot8_005388ba+surfcopy_00549d55+frame_0082fd70+list_00549db9+pool_00549db9+script_0054d798+textout_0054d211; wall=3600; emu=skip:win32 PeekMessageA")
