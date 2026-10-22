#include <gp4/gp4.h>
#include <gp4/flags.h>

// Screen entry: after the empty routine 0x00527f80 (eax = 0x324) it prepares the item chain header
// esi = 0x008507b6 and flushes its flags through 0x0053e4db with eax = 2 when the mode dword [0x007ad898]
// is 2 or 3, or when it is 0 and the byte [0x007aafd3] is 0xff (eax = 0 otherwise), then runs the screen
// loop 0x0053c8af on that header. Registers and flags thread through the callees.
// FUNCTION: GP4 0x0052a2c4
void __cdecl FUN_0052a2c4(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    c.eax = 0x324u;
    gp4::call_regs(0x00527f80, c);

    c.esi = 0x008507b6u;
    c.eax = 0u;
    const uint32_t mode = GP4_GLOBAL(uint32_t, 0x007ad898);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(mode, 2u));
    if (mode == 2u) {
        c.eax |= 2u;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.eax));
    } else {
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(mode, 3u));
        if (mode == 3u) {
            c.eax |= 2u;
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.eax));
        } else {
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(mode, 0u));
            if (mode == 0u) {
                const uint8_t marker = GP4_GLOBAL(uint8_t, 0x007aafd3);
                c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(marker, 0xffu));
                if (marker == 0xffu) {
                    c.eax |= 2u;
                    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.eax));
                }
            }
        }
    }

    gp4::call_regs(0x0053e4db, c);
    gp4::call_regs(0x0053c8af, c);

    *r = c;
}
GP4_IMPL(0x0052a2c4, FUN_0052a2c4, "regs(ebx:u32, ecx:u32, edx:u32, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=case_0054fd4d+anim_0052822a+slots_005388ed+cells_00527eae+slot8_005388ba+surfcopy_00549d55+frame_0082fd70; emu=skip:win32 PeekMessageA")
