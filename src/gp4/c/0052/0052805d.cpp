#include <gp4/gp4.h>
#include <gp4/flags.h>

// Byte-identical twin of 0x0052800e whose last call goes to the empty routine 0x00531b33 instead of
// 0x00531b3b. pushal/popal wrapper: when the byte [0x0081cd90] is set, ebx and eax are parked in
// [0x0081cd88]/[0x0081cd8c], the frame number edx (< 0x328) goes to the cell closer 0x005280ac (CF clear: the
// cell is still live), 0x005388ba stamps slot [edi] & 0x3ff and returns its dword +8; when nonzero it becomes
// edx and esi, ebx/eax are reloaded from the parked globals, ebp/ecx from old esi +8/+0xc and the empty
// routine runs. popal restores every general register, so only the flags of the last flag writer survive.
// FUNCTION: GP4 0x0052805d
void __cdecl FUN_0052805d(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    const uint8_t enabled = GP4_GLOBAL(uint8_t, 0x0081cd90);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(enabled, 0u));
    if (enabled != 0u) {
        GP4_GLOBAL(uint32_t, 0x0081cd88) = c.ebx;
        GP4_GLOBAL(uint32_t, 0x0081cd8c) = c.eax;
        c.eax = c.edx;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.eax, 0x328u));
        if (c.eax < 0x328u) {
            gp4::call_regs(0x005280ac, c);
            if ((c.eflags & gp4::CF) == 0u) {
                c.eax = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(c.edi));
                gp4::call_regs(0x005388ba, c);
                c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.eax));
                if (c.eax != 0u) {
                    c.edx = c.eax;
                    c.ebx = GP4_GLOBAL(uint32_t, 0x0081cd88);
                    c.eax = GP4_GLOBAL(uint32_t, 0x0081cd8c);
                    const uint32_t* source = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(c.esi));
                    c.ebp = source[2];
                    c.ecx = source[3];
                    c.esi = c.edx;
                    gp4::call_regs(0x00531b33, c);
                }
            }
        }
    }

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0052805d, FUN_0052805d, "regs(eax:u32, ebx:u32, edx:u32[0..1000], esi:ptr[0x10]:bytes, edi:ptr[8]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slots_005388ed+cells_00527eae+slot8_005388ba")
