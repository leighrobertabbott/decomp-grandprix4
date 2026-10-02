#include <gp4/gp4.h>

struct Unknown_00625abc { uint32_t Unknown00; uint8_t Unknown04[0xa0]; uint32_t UnknownA4; };
struct Unknown_0063decc { uint32_t Unknown00; uint32_t Unknown04; uint8_t Unknown08[0x24]; };

// FUNCTION: GP4 0x004188c2
void __cdecl FUN_004188c2(gp4::Regs* r) {
    Unknown_00625abc* first = GP4_ARRAY(Unknown_00625abc, 0x00625abc);
    for (int i = 0; i < 0x16; i++) {
        first[i].Unknown00 = 0;
        first[i].UnknownA4 = 0;
    }
    Unknown_0063decc* second = GP4_ARRAY(Unknown_0063decc, 0x0063decc);
    uint32_t last = 0;
    for (int i = 0; i < 0x10; i++) {
        last = second[i].Unknown00;
        second[i].Unknown04 = last;
        second[i].Unknown00 = 0;
    }
    // eax keeps the last value read, ecx the exhausted loop counter, and the
    // flags those of the final "add edi, 0x2c" (0x0063e160 + 0x2c: all clear).
    r->eax = last;
    r->ecx = 0;
    r->eflags = r->eflags & ~0x8d5u;
}
GP4_IMPL(0x004188c2, FUN_004188c2, "regs() -> (eax, ecx, cf, pf, af, zf, sf, of)")
