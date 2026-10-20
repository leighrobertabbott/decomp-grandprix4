#include <gp4/gp4.h>

// FUNCTION: GP4 0x004123c6
// Builds the four derived tables (0x004121c7, 0x004120e4, 0x00411f9f, 0x004123a6), then stores the
// 4.28 reciprocals 0x10000000 / (0x4000 - [0x00624790]) at 0x00624794 and 0x10000000 / [0x00624790]
// at 0x00624798. The last "xor edx, edx" leaves ZF = PF = 1 (the IDIV keeps those flags).
void __cdecl FUN_004123c6(gp4::Regs* r) {
    gp4::Regs call = *r;
    gp4::call_regs(0x004121c7, call);
    gp4::call_regs(0x004120e4, call);
    gp4::call_regs(0x00411f9f, call);
    gp4::call_regs(0x004123a6, call);

    int32_t remainder;
    const int32_t complement = 0x4000 - GP4_GLOBAL(int32_t, 0x00624790);
    GP4_GLOBAL(int32_t, 0x00624794) = gp4::idiv64(0x10000000ll, complement, &remainder);

    const int32_t share = GP4_GLOBAL(int32_t, 0x00624790);
    const int32_t quotient = gp4::idiv64(0x10000000ll, share, &remainder);
    GP4_GLOBAL(int32_t, 0x00624798) = quotient;

    r->eax = static_cast<uint32_t>(quotient);
    r->ebx = call.ebx;
    r->ecx = call.ecx;
    r->edx = static_cast<uint32_t>(remainder);
    r->ebp = static_cast<uint32_t>(share);
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x004123c6, FUN_004123c6, "regs() -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_004121c7+state_004120e4+state_00411f9f+state_004123a6+frac_00624790")
