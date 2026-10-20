#include <gp4/gp4.h>
#include <gp4/x87.h>

// Converts the three dwords at edx+0/+4/+8 to doubles scaled by the qword at 0x006f2bdc (pi/2^31)
// and stores them at edx+0x10/+0x18/+0x20 (same body as the 0x401d5c angle conversion, GP3-heritage
// copy). FILD * FMUL stays in extended precision until the FSTP.
// FUNCTION: GP4 0x0051edd5
void __cdecl FUN_0051edd5(gp4::Regs* r) {
    uint8_t* block = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edx));
    const gp4::x87::Ext scale = gp4::x87::ext(GP4_GLOBAL(double, 0x006f2bdc));
    GP4_FIELD(double, block, 0x10) = gp4::x87::narrow(gp4::x87::ext_i32(GP4_FIELD(int32_t, block, 0x0)) * scale);
    GP4_FIELD(double, block, 0x18) = gp4::x87::narrow(gp4::x87::ext_i32(GP4_FIELD(int32_t, block, 0x4)) * scale);
    GP4_FIELD(double, block, 0x20) = gp4::x87::narrow(gp4::x87::ext_i32(GP4_FIELD(int32_t, block, 0x8)) * scale);
}
GP4_IMPL(0x0051edd5, FUN_0051edd5, "regs(edx:ptr[0x30]:bytes) -> ()")
