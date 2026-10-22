#include <gp4/gp4.h>
#include <gp4/flags.h>

// Standings screen runner. Refreshes the places shown (0x00548e66), the leader time fields (0x00548e0b)
// and the lap-time conversion constants (0x00548cec), sets the flag byte [0x00849542 + 0xd] and then
// loops: after the empty routine 0x00527f80 (eax = 0x2f4) it runs the screen loop 0x0053c8af on the
// record at esi = 0x00849542; a result of 3 or more leaves, 0, 1 and 2 call the handler stored at
// 0x00546904 + 4 * result with eax pointing at that table entry.
// Registers and flags thread through the callees; the exit flags are those of `cmp eax, 3`.
// FUNCTION: GP4 0x005468bb
void __cdecl FUN_005468bb(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    gp4::call_regs(0x00548e66, c);
    gp4::call_regs(0x00548e0b, c);
    gp4::call_regs(0x00548cec, c);

    c.esi = 0x00849542u;
    GP4_FIELD(uint8_t, c.esi, 0x0d) = 1u;

    for (;;) {
        c.eax = 0x2f4u;
        gp4::call_regs(0x00527f80, c);
        c.esi = 0x00849542u;
        gp4::call_regs(0x0053c8af, c);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.eax, 3u));
        if (c.eax >= 3u) {
            break;
        }
        const uint32_t scaled = c.eax << 2;
        const uint32_t entry = scaled + 0x00546904u;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(scaled, 0x00546904u));
        c.eax = entry;
        gp4::call_regs(*reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(entry)), c);
    }

    *r = c;
}
GP4_IMPL(0x005468bb, FUN_005468bb, "regs(ebx:u32, ecx:u32, edx:u32, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=drv_005117d3+tabs_0043ced2+trk_007ad894+order_00675958+time_0040ed93+circ_007c3ae8+case_0054fd4d+anim_0052822a+slots_005388ed+cells_00527eae+slot8_005388ba+surfcopy_00549d55+frame_0082fd70; emu=skip:win32 PeekMessageA")
