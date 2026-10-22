#include <gp4/gp4.h>
#include <gp4/flags.h>

// Builds a mask in ebx (eax is saved and restored): while the dialog state byte [0x007ab020] is zero
// the mask is 0xd; otherwise the slot of the table 0x00b1b8d8 whose id is this machine's id
// [0x00b1bb40] is looked up (0x00541d06) and a found slot whose kind byte (+0x14) is 1 gives 7.
// Finally 0x60 is added unless this machine's id equals the host id (the flags of
// 0x00541a0a). The flags are those of the last `or` that ran, else of the compare in 0x00541a0a.
// FUNCTION: GP4 0x0054156b
void __cdecl FUN_0054156b(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    uint32_t mask = 0u;
    c.ebx = mask;
    const uint32_t state = GP4_GLOBAL(uint8_t, 0x007ab020);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(state, 0u));
    if (state == 0u) {
        mask |= 0xdu;
        c.ebx = mask;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(mask));
    } else {
        c.eax = GP4_GLOBAL(uint32_t, 0x00b1bb40);
        gp4::call_regs(0x00541d06, c);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.eax));
        if (c.eax != 0u) {
            const uint32_t kind = GP4_FIELD(uint8_t, c.eax, 0x14);
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(kind, 1u));
            if (kind == 1u) {
                mask |= 7u;
                c.ebx = mask;
                c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(mask));
            }
        }
    }

    gp4::call_regs(0x00541a0a, c);
    if ((c.eflags & gp4::flags::ZF) == 0u) {
        mask |= 0x60u;
        c.ebx = mask;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(mask));
    }

    r->ebx = mask;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054156b, FUN_0054156b, "regs() -> (ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slots_00541d5a+kind_00541cdb+host_00b1bb40+flag_007ab020")
