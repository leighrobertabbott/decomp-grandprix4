#include <gp4/gp4.h>
#include <gp4/flags.h>

// Selector dispatch on ebx with ecx = the dword [0x007ad87c]: 0 runs the stdcall query 0x00504c60 (ecx) (a
// display-mode code from the current screen width/height), 1 runs the stdcall query 0x00504cf0 (ecx) (a dword
// of the 0x24-byte mode table at 0x0066b9dc); both are followed by the register-neutral routine 0x0054cb71
// and leave eax/ecx/edx/flags as the callee does. Any other selector runs the empty failure stub 0x0054e232
// with esi = 0x00532af7 inside pushal/popal; the flags stay those of `cmp ebx, 1`.
// FUN_00532acc
static __declspec(noinline) void queryMode(gp4::Regs* c, uint32_t arg) {
    const uint32_t args[1] = {arg};
    gp4::call_regs_stdcall(0x00504c60, *c, args);
    gp4::call_regs(0x0054cb71, *c);
}

static __declspec(noinline) void queryTable(gp4::Regs* c, uint32_t arg) {
    const uint32_t args[1] = {arg};
    gp4::call_regs_stdcall(0x00504cf0, *c, args);
    gp4::call_regs(0x0054cb71, *c);
}

// FUNCTION: GP4 0x00532acc
void __cdecl FUN_00532acc(gp4::Regs* r) {
    const uint32_t selector = r->ebx;
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ecx = GP4_GLOBAL(uint32_t, 0x007ad87c);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(selector));

    if (selector == 0u) {
        queryMode(&c, c.ecx);
    } else {
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(selector, 1u));
        if (selector == 1u) {
            queryTable(&c, c.ecx);
        } else {
            gp4::Regs stub = c;
            stub.esi = 0x00532af7u;
            gp4::call_regs(0x0054e232, stub);
            c.eflags = stub.eflags;
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
GP4_IMPL(0x00532acc, FUN_00532acc, "regs(ebx:u32{0,1,2,3,0x80000000}) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cb_00531648")
