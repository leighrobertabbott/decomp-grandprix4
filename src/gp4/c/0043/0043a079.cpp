#include <gp4/gp4.h>
#include <gp4/x87.h>

// FUNCTION: GP4 0x0043a079
// Rotates a vector through an orientation body and returns the resulting binary angles. eax / ebx /
// ecx are binary angles: 0x00402655 turns them into a rotation at slot 0x20. The vector
// 0x00622c28 / 30 / 38 (that rotation's vector part) is copied into the body 0x006231d8 at +0xb8 /
// +0xc0 / +0xc8, rotated by its matrix (0x00401f02, result at +0xa0 / +0xa8 / +0xb0) and copied back.
// The body's angle doubles +0x18 / +0x10 / +0x20 are converted to binary angles (scale 0x006f2be4,
// rounded through 0x006f2bf4) and become the rotation at slot 0x40; the two slots are multiplied
// (0x00402564) and turned back into angles (0x00402777, 0x00402a9a). Returns eax / ebx / ecx =
// 0x00622bd4 / bd0 / bd8, edx = 0x60 and the flags 0x00402777 leaves.
void __cdecl FUN_0043a079(gp4::Regs* r) {
    using namespace gp4::x87;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    uint8_t* const body = GP4_ARRAY(uint8_t, 0x006231d8);

    c.edx = 0x20u;
    gp4::call_regs(0x00402655, c);

    GP4_FIELD(double, body, 0xb8) = narrow(ext(GP4_GLOBAL(double, 0x00622c28)));
    GP4_FIELD(double, body, 0xc0) = narrow(ext(GP4_GLOBAL(double, 0x00622c30)));
    GP4_FIELD(double, body, 0xc8) = narrow(ext(GP4_GLOBAL(double, 0x00622c38)));
    c.edx = 0x006231d8u;
    gp4::call_regs(0x00401f02, c);
    GP4_GLOBAL(double, 0x00622c28) = narrow(ext(GP4_FIELD(double, body, 0xa0)));
    GP4_GLOBAL(double, 0x00622c30) = narrow(ext(GP4_FIELD(double, body, 0xa8)));
    GP4_GLOBAL(double, 0x00622c38) = narrow(ext(GP4_FIELD(double, body, 0xb0)));

    GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(ext(GP4_FIELD(double, body, 0x18)) * ext(GP4_GLOBAL(double, 0x006f2be4)));
    c.eax = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x006f2bf4));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(ext(GP4_FIELD(double, body, 0x10)) * ext(GP4_GLOBAL(double, 0x006f2be4)));
    c.ebx = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x006f2bf4));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(ext(GP4_FIELD(double, body, 0x20)) * ext(GP4_GLOBAL(double, 0x006f2be4)));
    c.ecx = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x006f2bf4));

    c.edx = 0x40u;
    gp4::call_regs(0x00402655, c);
    gp4::call_regs(0x00402564, c);
    c.edx = 0x60u;
    gp4::call_regs(0x00402777, c);
    gp4::cdecl_<0x00402a9a, void>();

    r->eax = GP4_GLOBAL(uint32_t, 0x00622bd4);
    r->ebx = GP4_GLOBAL(uint32_t, 0x00622bd0);
    r->ecx = GP4_GLOBAL(uint32_t, 0x00622bd8);
    r->edx = c.edx;
    r->eflags = c.eflags;
}
GP4_IMPL(0x0043a079, FUN_0043a079, "regs(eax:u32, ebx:u32, ecx:u32) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=view_00609860+ang_006231d8")
