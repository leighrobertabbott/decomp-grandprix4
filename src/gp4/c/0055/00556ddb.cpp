#include <gp4/gp4.h>
#include <gp4/flags.h>

// `cmp byte [0x86857c], 2 ; je call ; add esi, 2 ; ret ; call: call 0x00556dee ; ret`: when the mode
// byte is 2 the command word at esi is decoded by 0x00556dee (which advances esi and sets eax and
// the flags); otherwise the word is skipped (esi += 2, flags of the add).
// FUNCTION: GP4 0x00556ddb
void __cdecl FUN_00556ddb(gp4::Regs* r) {
    if (GP4_GLOBAL(uint8_t, 0x0086857c) == 2u) {
        gp4::call_regs(0x00556dee, *r);
        return;
    }
    const uint32_t esi = r->esi;
    r->esi = esi + 2u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi, 2u));
}
GP4_IMPL(0x00556ddb, FUN_00556ddb, "regs(eax:u32, ebx:ptr[0xa0]:bytes, esi:ptr[0x40]:u16[0..0x10f]) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ptr_0086850c+mode_0086857c+max_00932440")
