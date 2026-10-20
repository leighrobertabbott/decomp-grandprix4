#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0051da29
// Looks up the 16-byte record of the control code in bl (6, 7, 0x20, 0x22, 0x23, 0x24, 8, 9,
// 0x21, 0x25, 0x26, 0x27 -> records 0x7ab02c + k*0x10): ecx = the record address, edx = its dword
// +8 and eax = its dword +4 or +0xc, the other one when bh is negative (codes 6 and 8 take +0xc
// first). CF = 0 on a hit (other flags from `test bh, 0xff`); an unknown code leaves eax/ecx/edx
// and returns CF = 1 with the flags of `cmp bl, 0x27`.
void __cdecl FUN_0051da29(gp4::Regs* r) {
    const uint8_t code = static_cast<uint8_t>(r->ebx);
    const uint8_t high = static_cast<uint8_t>(r->ebx >> 8);
    const bool alt = static_cast<int8_t>(high) < 0;
    uint32_t rec;
    uint32_t a;
    uint32_t d;

    if (code == 6) {
        rec = 0x007ab02cu;
        d = GP4_GLOBAL(uint32_t, 0x007ab034);
        a = alt ? GP4_GLOBAL(uint32_t, 0x007ab030) : GP4_GLOBAL(uint32_t, 0x007ab038);
    } else if (code == 7) {
        rec = 0x007ab03cu;
        d = GP4_GLOBAL(uint32_t, 0x007ab044);
        a = alt ? GP4_GLOBAL(uint32_t, 0x007ab048) : GP4_GLOBAL(uint32_t, 0x007ab040);
    } else if (code == 0x20) {
        rec = 0x007ab04cu;
        d = GP4_GLOBAL(uint32_t, 0x007ab054);
        a = alt ? GP4_GLOBAL(uint32_t, 0x007ab058) : GP4_GLOBAL(uint32_t, 0x007ab050);
    } else if (code == 0x22) {
        rec = 0x007ab05cu;
        d = GP4_GLOBAL(uint32_t, 0x007ab064);
        a = alt ? GP4_GLOBAL(uint32_t, 0x007ab068) : GP4_GLOBAL(uint32_t, 0x007ab060);
    } else if (code == 0x23) {
        rec = 0x007ab06cu;
        d = GP4_GLOBAL(uint32_t, 0x007ab074);
        a = alt ? GP4_GLOBAL(uint32_t, 0x007ab078) : GP4_GLOBAL(uint32_t, 0x007ab070);
    } else if (code == 0x24) {
        rec = 0x007ab07cu;
        d = GP4_GLOBAL(uint32_t, 0x007ab084);
        a = alt ? GP4_GLOBAL(uint32_t, 0x007ab088) : GP4_GLOBAL(uint32_t, 0x007ab080);
    } else if (code == 8) {
        rec = 0x007ab08cu;
        d = GP4_GLOBAL(uint32_t, 0x007ab094);
        a = alt ? GP4_GLOBAL(uint32_t, 0x007ab090) : GP4_GLOBAL(uint32_t, 0x007ab098);
    } else if (code == 9) {
        rec = 0x007ab09cu;
        d = GP4_GLOBAL(uint32_t, 0x007ab0a4);
        a = alt ? GP4_GLOBAL(uint32_t, 0x007ab0a8) : GP4_GLOBAL(uint32_t, 0x007ab0a0);
    } else if (code == 0x21) {
        rec = 0x007ab0acu;
        d = GP4_GLOBAL(uint32_t, 0x007ab0b4);
        a = alt ? GP4_GLOBAL(uint32_t, 0x007ab0b8) : GP4_GLOBAL(uint32_t, 0x007ab0b0);
    } else if (code == 0x25) {
        rec = 0x007ab0bcu;
        d = GP4_GLOBAL(uint32_t, 0x007ab0c4);
        a = alt ? GP4_GLOBAL(uint32_t, 0x007ab0c8) : GP4_GLOBAL(uint32_t, 0x007ab0c0);
    } else if (code == 0x26) {
        rec = 0x007ab0ccu;
        d = GP4_GLOBAL(uint32_t, 0x007ab0d4);
        a = alt ? GP4_GLOBAL(uint32_t, 0x007ab0d8) : GP4_GLOBAL(uint32_t, 0x007ab0d0);
    } else if (code == 0x27) {
        rec = 0x007ab0dcu;
        d = GP4_GLOBAL(uint32_t, 0x007ab0e4);
        a = alt ? GP4_GLOBAL(uint32_t, 0x007ab0e8) : GP4_GLOBAL(uint32_t, 0x007ab0e0);
    } else {
        r->eflags = gp4::flags::merge(r->eflags, (gp4::flags::sub8(code, 0x27u) & ~1u) | 1u);
        return;
    }

    r->ecx = rec;
    r->edx = d;
    r->eax = a;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(high));
}
GP4_IMPL(0x0051da29, FUN_0051da29, "regs(eax:u32, ebx:u32{0x06,0x8006,0x1207,0x8007,0x20,0xff20,0x22,0x8022,0x7f23,0x8023,0x24,0x9024,0x08,0x8008,0x0109,0xc009,0x21,0x8021,0x25,0x8025,0x26,0x8026,0x27,0xa027,0x00,0x8028,0x05,0x7f1f,0xff}, ecx:u32, edx:u32) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
