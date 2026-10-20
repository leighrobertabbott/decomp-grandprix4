#include <gp4/gp4.h>
#include <gp4/flags.h>

// `push edx ; call 0x00538f91 ; pop edx ; cmp edx, 2 ; jne ; call 0x005327ec ; mov edx, 0 ; ret`:
// runs the menu value handler, then for command 2 also loads esi with the byte buffer at 0x00843fc5
// (eax = 0). edx is cleared; the flags are those of `cmp edx, 2`.
// FUNCTION: GP4 0x005327f7
void __cdecl FUN_005327f7(gp4::Regs* r) {
    const uint32_t command = r->edx;
    gp4::call_regs(0x00538f91, *r);
    r->edx = command;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(command, 2u));
    if (command == 2u) {
        gp4::call_regs(0x005327ec, *r);
    }
    r->edx = 0u;
}
GP4_IMPL(0x005327f7, FUN_005327f7, "regs(eax:u32, edx:u32{1,2,5,6,0,1,2}, ebx:ptr[8]:u8{0,0,0,1,2}, esi:ptr[0x40]:u8{0,0,0,1,1,2,3,0x80}) -> (eax, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
