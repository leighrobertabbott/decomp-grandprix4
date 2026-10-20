#include <gp4/gp4.h>
#include <gp4/flags.h>

// Writes a parenthesised text into the buffer at edi: '(' , the text produced by 0x005489a6 (which
// advances edi), ')' and a terminating NUL (edi ends after the ')', on the NUL). The flags are those of
// the final `inc edi` (CF kept from the callee).
// FUNCTION: GP4 0x00548995
void __cdecl FUN_00548995(gp4::Regs* r) {
    uint8_t* out = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    *out = 0x28u;
    gp4::Regs c = *r;
    c.edi = r->edi + 1u;
    c.eflags = gp4::flags::merge(r->eflags, gp4::flags::inc32(r->edi, r->eflags));
    gp4::call_regs(0x005489a6, c);

    uint8_t* close = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.edi));
    close[0] = 0x29u;
    close[1] = 0u;
    *r = c;
    r->edi = c.edi + 1u;
    r->eflags = gp4::flags::merge(c.eflags, gp4::flags::inc32(c.edi, c.eflags));
}
GP4_IMPL(0x00548995, FUN_00548995, "regs(eax:u32[0..0x3f], ebx:u32, ecx:u32, edx:u32, edi:ptr[0x100]:bytes, ebp:u32, esi:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798+drv22_0052d402")
