#include <gp4/gp4.h>
#include <gp4/x87.h>

// FUNCTION: GP4 0x00402906
// Composes the two orientations (esi+0xc/+0x10/+0x14 into slot 0x40, and the global angle triple
// at 0x00638dd0.. into slot 0x20), negates the slot-0x20 quaternion vector part, multiplies the
// two slots (0x00402564), rotates the result vector through the 3x3 matrix block at 0x00622cf8
// (0x00401e2e), then turns the product back into three binary angles
// (0x00402777 + 0x00402a9a) and stores them at 0x00638034 / 0x00638030 / 0x00638038.
void __cdecl FUN_00402906(gp4::Regs* r) {
    using namespace gp4::x87;

    uint8_t* const src = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs rg = *r;
    rg.st_in = 0;
    rg.st_out = 0;
    rg.st_xin_mask = 0;
    rg.st_xout_mask = 0;

    rg.eax = GP4_FIELD(uint32_t, src, 0x0c);
    rg.ebx = GP4_FIELD(uint32_t, src, 0x10);
    rg.ecx = GP4_FIELD(uint32_t, src, 0x14);
    rg.edx = 0x40;
    gp4::call_regs(0x00402655, rg);

    rg.eax = GP4_GLOBAL(uint32_t, 0x00638dd0);
    rg.ebx = GP4_GLOBAL(uint32_t, 0x00638dd4);
    rg.ecx = GP4_GLOBAL(uint32_t, 0x00638dd8);
    rg.edx = 0x20;
    gp4::call_regs(0x00402655, rg);

    GP4_GLOBAL(double, 0x00622c28) = narrow(fchs(ext(GP4_GLOBAL(double, 0x00622c28))));
    GP4_GLOBAL(double, 0x00622c30) = narrow(fchs(ext(GP4_GLOBAL(double, 0x00622c30))));
    GP4_GLOBAL(double, 0x00622c38) = narrow(fchs(ext(GP4_GLOBAL(double, 0x00622c38))));

    gp4::call_regs(0x00402564, rg);

    rg.edx = 0x00622cf8;
    uint8_t* const block = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(rg.edx));
    GP4_FIELD(double, block, 0xa0) = narrow(ext(GP4_GLOBAL(double, 0x00622c68)));
    GP4_FIELD(double, block, 0xa8) = narrow(ext(GP4_GLOBAL(double, 0x00622c70)));
    GP4_FIELD(double, block, 0xb0) = narrow(ext(GP4_GLOBAL(double, 0x00622c78)));
    gp4::call_regs(0x00401e2e, rg);
    GP4_GLOBAL(double, 0x00622c68) = narrow(ext(GP4_FIELD(double, block, 0xb8)));
    GP4_GLOBAL(double, 0x00622c70) = narrow(ext(GP4_FIELD(double, block, 0xc0)));
    GP4_GLOBAL(double, 0x00622c78) = narrow(ext(GP4_FIELD(double, block, 0xc8)));

    rg.edx = 0x60;
    gp4::call_regs(0x00402777, rg);

    gp4::cdecl_<0x00402a9a, void>();

    rg.eax = GP4_GLOBAL(uint32_t, 0x00622bd0);
    GP4_GLOBAL(uint32_t, 0x00638034) = rg.eax;
    rg.eax = GP4_GLOBAL(uint32_t, 0x00622bd4);
    GP4_GLOBAL(uint32_t, 0x00638030) = rg.eax;
    rg.eax = GP4_GLOBAL(uint32_t, 0x00622bd8);
    GP4_GLOBAL(uint32_t, 0x00638038) = rg.eax;

    r->eax = rg.eax;
    r->ebx = rg.ebx;
    r->ecx = rg.ecx;
    r->edx = rg.edx;
    r->eflags = rg.eflags;
}
GP4_IMPL(0x00402906, FUN_00402906, "regs(esi:ptr[0x40]:bytes) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=view_00609860")
