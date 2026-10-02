#include <gp4/gp4.h>

// FUNCTION: GP4 0x004125ce
void __cdecl FUN_004125ce(gp4::Regs* r) {
    const uint32_t source = GP4_GLOBAL(uint32_t, 0x00603854);
    uint32_t scaled = (source << 16) / 128u;
    if ((scaled & 0xffffu) != 0u) {
        scaled += 0x10000u;
    }
    const uint32_t divisor = scaled >> 16;
    const uint32_t quotient = source / divisor;
    const uint32_t remainder = source % divisor;
    GP4_GLOBAL(uint32_t, 0x006228ac) = quotient;
    GP4_GLOBAL(uint32_t, 0x00608164) = quotient;
    r->eax = quotient;
    r->ecx = divisor;
    r->edx = remainder;
    r->ebp = 128u;
    // IDIV arithmetic flags are undefined; the pinned Unicorn oracle
    // retains the preceding XOR EDX,EDX flags.
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x004125ce, FUN_004125ce, "regs() -> (eax,ecx,edx,ebp,cf,pf,af,zf,sf,of); globals=layout=state_004125ce")
