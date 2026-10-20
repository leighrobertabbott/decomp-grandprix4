#include <gp4/gp4.h>
#include <gp4/flags.h>

// Text-script interpreter: esi walks a zero-terminated string. Each byte (0x14 or more all act as 0x14) selects
// a handler from the dword table at 0x00545f27; the handler (a separate routine that advances esi and may
// load registers or draw text) runs with eax = the table slot address and the flags of the `add eax,
// 0x545f27`. The loop ends when the byte at esi is zero after a handler; the flags are then those of
// that `cmp byte [esi], 0` (ZF = PF = 1). Every register is whatever the last handler left.
// FUNCTION: GP4 0x00545f09
void __cdecl FUN_00545f09(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    for (;;) {
        uint32_t opcode = GP4_GLOBAL(uint8_t, c.esi);
        if (opcode > 0x14u) {
            opcode = 0x14u;
        }
        const uint32_t slot = (opcode << 2) + 0x00545f27u;
        c.eax = slot;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(opcode << 2, 0x00545f27u));
        gp4::call_regs(GP4_GLOBAL(uint32_t, slot), c);
        if (GP4_GLOBAL(uint8_t, c.esi) == 0u) {
            break;
        }
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(0u, 0u));
}
GP4_IMPL(0x00545f09, FUN_00545f09, "regs(ebx:u32, ecx:u32, edx:u32, ebp:u32, esi:ptr[0x40]:u8{=0,1,2,3,4,5,0x41,0x20}) -> (eax, ebx, ecx, edx, esi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cursor_00545f09+surf_0054654a")
