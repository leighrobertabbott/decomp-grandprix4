#include <gp4/gp4.h>
#include <gp4/flags.h>

// Builds the request mask in ebx: 0x1f7 when the byte [0x007ab020] is zero; otherwise looks up the record
// of [0x00b1bb40] with 0x00541d06 and, when it exists and its byte +0x14 is 1, ORs in 0x1df. Then
// 0x00541a0a is run and, when it leaves ZF clear, 0x180 is ORed in as well. eax is pushed and popped around
// the body (0x00541d06 and 0x00541a0a keep every other register); the flags are those of the last
// compare / `or`.
// FUNCTION: GP4 0x005415dc
void __cdecl FUN_005415dc(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    uint32_t mask = 0u;
    const uint8_t active = GP4_GLOBAL(uint8_t, 0x007ab020);
    uint32_t flags = gp4::flags::sub8(active, 0u);
    if (active == 0u) {
        mask |= 0x1f7u;
        flags = gp4::flags::logic32(mask);
    } else {
        c.eax = GP4_GLOBAL(uint32_t, 0x00b1bb40);
        c.ebx = mask;
        c.eflags = gp4::flags::merge(r->eflags, flags);
        gp4::call_regs(0x00541d06, c);
        const uint32_t record = c.eax;
        flags = gp4::flags::logic32(record);
        if (record != 0u) {
            const uint8_t kind = GP4_FIELD(uint8_t, record, 0x14);
            flags = gp4::flags::sub8(kind, 1u);
            if (kind == 1u) {
                mask |= 0x1dfu;
                flags = gp4::flags::logic32(mask);
            }
        }
    }

    c.ebx = mask;
    c.eflags = gp4::flags::merge(r->eflags, flags);
    gp4::call_regs(0x00541a0a, c);
    flags = c.eflags;
    if ((flags & 0x40u) == 0u) {
        mask |= 0x180u;
        flags = gp4::flags::logic32(mask);
    }

    c.eax = r->eax;
    c.ebx = mask;
    c.eflags = gp4::flags::merge(r->eflags, flags);
    *r = c;
}
GP4_IMPL(0x005415dc, FUN_005415dc, "regs(eax:u32) -> (ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slots_00541d5a+kind_00541cdb+slotsel_00540884+host_00b1bb40")
