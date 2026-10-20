#include <gp4/gp4.h>
#include <gp4/x87.h>

// FUNCTION: GP4 0x0040233e
// Camera/body setup chain on the body at 0x00622e98: 0x0040200c builds the rotation matrix from the
// angle dwords, 0x00421887 rounds the transformed vector into 0x00607fd0.., the dwords 0x00607fd0..
// and 0x006097c0.. become doubles (FILD), 0x004024b5 derives and clamps the three components
// 0x00609778.., which are copied to body +0xb8/+0xc0/+0xc8, 0x00401f02 rotates them into
// +0xa0/+0xa8/+0xb0 and those are rounded (FISTP) into 0x006081e0.. . edx = the body address; eax
// and the flags are those the callees left.
void __cdecl FUN_0040233e(gp4::Regs* r) {
    using namespace gp4::x87;
    gp4::call_regs(0x0040200c, *r);
    gp4::call_regs(0x00421887, *r);
    GP4_GLOBAL(double, 0x00609760) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00607fd0));
    GP4_GLOBAL(double, 0x00609768) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00607fd4));
    GP4_GLOBAL(double, 0x00609770) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00607fd8));
    GP4_GLOBAL(double, 0x00609790) = static_cast<double>(GP4_GLOBAL(int32_t, 0x006097c0));
    GP4_GLOBAL(double, 0x00609798) = static_cast<double>(GP4_GLOBAL(int32_t, 0x006097c4));
    GP4_GLOBAL(double, 0x006097a0) = static_cast<double>(GP4_GLOBAL(int32_t, 0x006097c8));
    gp4::call_regs(0x004024b5, *r);

    uint8_t* body = GP4_ARRAY(uint8_t, 0x00622e98);
    r->edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(body));
    GP4_FIELD(double, body, 0xb8) = GP4_GLOBAL(double, 0x00609778);
    GP4_FIELD(double, body, 0xc0) = GP4_GLOBAL(double, 0x00609780);
    GP4_FIELD(double, body, 0xc8) = GP4_GLOBAL(double, 0x00609788);
    gp4::call_regs(0x00401f02, *r);
    GP4_GLOBAL(int32_t, 0x006081e0) = fistp32(GP4_FIELD(double, body, 0xa0));
    GP4_GLOBAL(int32_t, 0x006081e4) = fistp32(GP4_FIELD(double, body, 0xa8));
    GP4_GLOBAL(int32_t, 0x006081e8) = fistp32(GP4_FIELD(double, body, 0xb0));
}
GP4_IMPL(0x0040233e, FUN_0040233e, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
