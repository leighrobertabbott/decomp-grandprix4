#include <gp4/gp4.h>
#include <gp4/flags.h>

// Pan byte writer (pushal wrapper). When bit 0x20 of the flag byte 0x874b4f is clear nothing happens
// (flags of that `test`). Otherwise al = (al & [0x874b4f]) + 0x7c is stored at ebx+0x1c, or at ebx+0x1d
// when dl == 0x40 (the byte it replaces goes to cl), and 0x0055933e then runs with al restored to the
// masked value. pushal/popal keep every register; the flags are those the callee leaves.
// FUNCTION: GP4 0x00559310
void __cdecl FUN_00559310(gp4::Regs* r) {
    const uint32_t flagByte = GP4_GLOBAL(uint8_t, 0x00874b4f);
    if ((flagByte & 0x20u) == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(flagByte & 0x20u));
        return;
    }
    gp4::Regs c = *r;
    const uint32_t masked = c.eax & flagByte & 0xffu;
    const uint32_t biased = (masked + 0x7cu) & 0xffu;
    if ((c.edx & 0xffu) == 0x40u) {
        c.ecx = (c.ecx & 0xffffff00u) | GP4_FIELD(uint8_t, c.ebx, 0x1d);
        GP4_FIELD(uint8_t, c.ebx, 0x1d) = static_cast<uint8_t>(biased);
    } else {
        c.ecx = (c.ecx & 0xffffff00u) | GP4_FIELD(uint8_t, c.ebx, 0x1c);
        GP4_FIELD(uint8_t, c.ebx, 0x1c) = static_cast<uint8_t>(biased);
    }
    c.eax = (c.eax & 0xffffff00u) | ((biased - 0x7cu) & 0xffu);
    gp4::call_regs(0x0055933e, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00559310, FUN_00559310, "regs(eax:u32, ecx:u32, edx:u32{0x40,0x40,0x41,0,0xff,0x12340040}, ebx:ptr[0x20]:bytes, edi:ptr[0x6a]:u8{=0,0,0,1,0x7f,0x80,0xff}) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=pan_0055933e")
