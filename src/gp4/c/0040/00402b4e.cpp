#include <gp4/gp4.h>

// FUNCTION: GP4 0x00402b4e
// Builds two Euler-angle rotations (0x00402655 at slot 0x20 from the angles 0x00638030/34/38, at
// slot 0x40 from the negated 0x00622b44 and 0x00622b40/48), multiplies them (0x00402564), turns the
// product at slot 0x60 back into angles (0x00402777), converts those to binary angles (0x00402a9a)
// and stores them as 0x00638034 / 0x00638030 / 0x00638038. The registers on exit are what the
// callees leave behind (ebx / ecx from the second 0x00402655, edx = 0x60, eax = the last angle).
void __cdecl FUN_00402b4e(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.eax = GP4_GLOBAL(uint32_t, 0x00638030);
    c.ebx = GP4_GLOBAL(uint32_t, 0x00638034);
    c.ecx = GP4_GLOBAL(uint32_t, 0x00638038);
    c.edx = 0x20u;
    gp4::call_regs(0x00402655, c);

    c.eax = 0u - GP4_GLOBAL(uint32_t, 0x00622b44);
    c.ebx = GP4_GLOBAL(uint32_t, 0x00622b40);
    c.ecx = GP4_GLOBAL(uint32_t, 0x00622b48);
    c.edx = 0x40u;
    gp4::call_regs(0x00402655, c);

    gp4::call_regs(0x00402564, c);

    c.edx = 0x60u;
    gp4::call_regs(0x00402777, c);

    gp4::cdecl_<0x00402a9a, void>();

    GP4_GLOBAL(uint32_t, 0x00638034) = GP4_GLOBAL(uint32_t, 0x00622bd0);
    GP4_GLOBAL(uint32_t, 0x00638030) = GP4_GLOBAL(uint32_t, 0x00622bd4);
    const uint32_t last = GP4_GLOBAL(uint32_t, 0x00622bd8);
    GP4_GLOBAL(uint32_t, 0x00638038) = last;

    r->eax = last;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->eflags = c.eflags;
}
GP4_IMPL(0x00402b4e, FUN_00402b4e, "regs() -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
