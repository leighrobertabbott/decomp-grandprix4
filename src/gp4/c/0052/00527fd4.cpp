#include <gp4/gp4.h>
#include <gp4/flags.h>

// R3 frame-cell hand-off: when the byte [0x0081cd90] is set and eax < 0x328, 0x005280ac closes the cell of
// frame eax (CF clear: the cell is still live); then 0x005388ba stamps slot [edi] & 0x3ff and returns its
// dword +8. When that is nonzero it becomes edx and esi, ebx/eax/ebp/ecx are loaded from the four dwords at the
// old esi (+0/+4/+8/+0xc) and the empty routine 0x00531b3b runs. The registers and flags are those of the last
// instruction executed on the path taken.
// FUNCTION: GP4 0x00527fd4
void __cdecl FUN_00527fd4(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    const uint8_t enabled = GP4_GLOBAL(uint8_t, 0x0081cd90);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(enabled, 0u));
    if (enabled != 0u) {
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.eax, 0x328u));
        if (c.eax < 0x328u) {
            gp4::call_regs(0x005280ac, c);
            if ((c.eflags & gp4::CF) == 0u) {
                c.eax = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(c.edi));
                gp4::call_regs(0x005388ba, c);
                c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.eax));
                if (c.eax != 0u) {
                    c.edx = c.eax;
                    const uint32_t* source = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(c.esi));
                    c.ebx = source[0];
                    c.eax = source[1];
                    c.ebp = source[2];
                    c.ecx = source[3];
                    c.esi = c.edx;
                    gp4::cdecl_<0x00531b3b, void>();
                }
            }
        }
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00527fd4, FUN_00527fd4, "regs(eax:u32[0..1000], esi:ptr[0x10]:bytes, edi:ptr[8]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slots_005388ed+cells_00527eae+slot8_005388ba")
