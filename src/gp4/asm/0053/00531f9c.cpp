#include <gp4/gp4.h>
#include <gp4/flags.h>

// Command dispatcher: clears the word [0x7a93e4], then for ax <= 0x8a calls the handler in the dword table at
// 0x531d74 (eax = the table slot address, other registers as the caller left them). ebx..ebp are pushed
// and popped around it; eax and the flags are the handler's (ax > 0x8a: eax unchanged, flags of `cmp ax, 0x8a`).
// FUNCTION: GP4 0x00531f9c
void __cdecl FUN_00531f9c(gp4::Regs* r) {
    GP4_GLOBAL(uint16_t, 0x007a93e4) = 0;

    const uint32_t command = r->eax & 0xffffu;
    if (command > 0x8au) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub16(command, 0x8au));
        return;
    }

    const uint32_t slot = command << 2;
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = slot + 0x00531d74u;
    c.eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(slot, 0x00531d74u));
    gp4::call_regs(GP4_ARRAY(uint32_t, 0x00531d74)[command], c);

    r->eax = c.eax;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00531f9c, FUN_00531f9c, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of)")
