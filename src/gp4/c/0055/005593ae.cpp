#include <gp4/gp4.h>
#include <gp4/flags.h>

// Segment record update: edi = the record [0x00874b18], ebx = the record [0x00874b1c]. The byte
// +0x2c of edi is read once into al; bit 7 runs 0x00559391 with dl = 0x80, bit 6 runs it with
// dl = 0x40 (the test uses the original al, which 0x00559391 does not touch); then 0x005593d9 ors
// a mask derived from al into five bytes of the ebx record. Registers and flags are those the last
// routine left (al = 3, dl/dh from 0x00559391, flags of its last `or`).
// FUNCTION: GP4 0x005593ae
void __cdecl FUN_005593ae(gp4::Regs* r) {
    r->edi = GP4_GLOBAL(uint32_t, 0x00874b18);
    r->ebx = GP4_GLOBAL(uint32_t, 0x00874b1c);
    const uint8_t flagsByte = GP4_FIELD(uint8_t, r->edi, 0x2c);
    r->eax = (r->eax & 0xffffff00u) | flagsByte;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(flagsByte & 0x80u));
    if ((flagsByte & 0x80u) != 0u) {
        c.edx = (c.edx & 0xffffff00u) | 0x80u;
        gp4::call_regs(0x00559391, c);
    }
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(flagsByte & 0x40u));
    if ((flagsByte & 0x40u) != 0u) {
        c.edx = (c.edx & 0xffffff00u) | 0x40u;
        gp4::call_regs(0x00559391, c);
    }
    gp4::call_regs(0x005593d9, c);
    *r = c;
}
GP4_IMPL(0x005593ae, FUN_005593ae, "regs(eax:u32, edx:u32) -> (eax, ebx, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=rec_005593ae")
