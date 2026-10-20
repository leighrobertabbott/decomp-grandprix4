#include <gp4/gp4.h>

// FUNCTION: GP4 0x005147a3
// `call 0x00514748 ; stosb ; ret`: parses a decimal number at esi (eax = value, esi advanced past
// it, flags from the parser) and stores its low byte at edi, then advances edi by one.
void __cdecl FUN_005147a3(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x00514748, c);

    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.edi));
    *dst++ = static_cast<uint8_t>(c.eax);

    r->eax = c.eax;
    r->esi = c.esi;
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(dst));
    r->eflags = c.eflags;
}
GP4_IMPL(0x005147a3, FUN_005147a3, "regs(esi:ptr[0x40]:u8{=0x30,0x31,0x35,0x39,0x41,0x7a,0x80,0xff,0x2d,0x2b,0x2c,0x0d,0x0a,0x20,0x09}, edi:ptr[0x40]:bytes) -> (eax, esi, edi, cf, pf, af, zf, sf, of)")
