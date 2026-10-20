#include <gp4/gp4.h>

// FUNCTION: GP4 0x0042b88b
// Car record at esi: with the track fraction [0x601b34] temporarily set to the sign-extended word +0x66,
// runs the lane offset 0x00403938 on eax = sign-extended word +0x94, edx = byte +0x294 (the lane mode) and
// edi = the segment pointer +0x1c. edi and edx are saved and restored; eax is the result and the flags
// those of 0x00403938 (the stores and pops leave them alone).
void __cdecl FUN_0042b88b(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t savedFraction = GP4_GLOBAL(uint32_t, 0x00601b34);
    GP4_GLOBAL(uint32_t, 0x00601b34) = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, rec, 0x66)));

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.edi = GP4_FIELD(uint32_t, rec, 0x1c);
    call.eax = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, rec, 0x94)));
    call.edx = GP4_FIELD(uint8_t, rec, 0x294);
    gp4::call_regs(0x00403938, call);

    GP4_GLOBAL(uint32_t, 0x00601b34) = savedFraction;

    call.edx = r->edx;
    call.edi = r->edi;
    call.eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
    *r = call;
}
GP4_IMPL(0x0042b88b, FUN_0042b88b, "regs(esi:ptr[0x29c]:layout=car_00431ffb+car_004035c3+car_0042b17e) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403800+ring_008943a0+records_008943a0+div_00403bb6")
