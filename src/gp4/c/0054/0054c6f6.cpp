#include <gp4/gp4.h>
#include <gp4/flags.h>

// Writes the script text at esi to the buffer at edi (0x0054d7c0) and pads it with spaces up to 0x20
// characters counted from the source length: ecx = 0x20 - strlen(esi) (0x0054d8d0), `rep stosb` of 0x20
// at the new edi. esi ends past the source terminator, edi past the padding, ecx = 0, al = 0x20.
// FUNCTION: GP4 0x0054c6f6
void __cdecl FUN_0054c6f6(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0054d8d0, c);
    const uint32_t negLen = 0u - c.eax;
    const uint32_t pad = negLen + 0x20u;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(negLen, 0x20u));
    c.eax = pad;
    gp4::call_regs(0x0054d7c0, c);
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.edi));
    for (uint32_t n = pad; n != 0u; --n) {
        *dst++ = 0x20u;
    }
    c.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(dst));
    c.ecx = 0u;
    c.eax = (c.eax & 0xffffff00u) | 0x20u;
    *r = c;
}
GP4_IMPL(0x0054c6f6, FUN_0054c6f6, "regs(ebx:u32, edx:u32, ebp:u32, esi:ptr[0x40]:u8{0,0x41,0x61,0x20,0x30,0x39}, edi:ptr[0x200]:bytes) -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
