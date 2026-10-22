#include <gp4/gp4.h>
#include <gp4/flags.h>

// Under pushal/popal: expands the driver name number [0x007c2e76] of the 0x18-byte name table at 0x007c2b3c
// ("michael schumacher" is entry 0) with the script interpreter 0x0054d7c0 into the buffer 0x00823300.
// popal restores every register; the flags are those the interpreter leaves.
// FUNCTION: GP4 0x00528a89
void __cdecl FUN_00528a89(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.edi = 0x00823300u;
    c.esi = static_cast<uint32_t>(GP4_GLOBAL(uint8_t, 0x007c2e76)) * 0x18u + 0x007c2b3cu;
    c.edx = 0u;
    c.eax = c.esi;
    gp4::call_regs(0x0054d7c0, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00528a89, FUN_00528a89, "regs(ebx:u32, ecx:u32, ebp:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=items_0054d7c0")
