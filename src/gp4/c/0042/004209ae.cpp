#include <gp4/gp4.h>

// FUNCTION: GP4 0x004209ae
// Hand-written: esi = destination block, ebp is clobbered with the divisor.
// Four non-negative-clamped timer values are divided by the divisor and the
// quotients stored at esi+0x2ec..0x2f8.
void __cdecl FUN_004209ae(gp4::Regs* r) {
    uint8_t* dest = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t divisor = GP4_GLOBAL(uint32_t, 0x006228b0);

    int32_t v0 = GP4_GLOBAL(int32_t, 0x00622930);
    if (v0 < 0) v0 = 0;
    const uint32_t q0 = static_cast<uint32_t>(v0) / divisor;
    GP4_FIELD(uint32_t, dest, 0x2ec) = q0;

    int32_t v1 = GP4_GLOBAL(int32_t, 0x00622934);
    if (v1 < 0) v1 = 0;
    const uint32_t q1 = static_cast<uint32_t>(v1) / divisor;
    GP4_FIELD(uint32_t, dest, 0x2f0) = q1;

    int32_t v2 = GP4_GLOBAL(int32_t, 0x00622938);
    if (v2 < 0) v2 = 0;
    const uint32_t q2 = static_cast<uint32_t>(v2) / divisor;
    GP4_FIELD(uint32_t, dest, 0x2f4) = q2;

    int32_t v3 = GP4_GLOBAL(int32_t, 0x0062293c);
    if (v3 < 0) v3 = 0;
    const uint32_t q3 = static_cast<uint32_t>(v3) / divisor;
    GP4_FIELD(uint32_t, dest, 0x2f8) = q3;

    r->eax = q3;
    r->edx = static_cast<uint32_t>(v3) % divisor;
    r->ebp = divisor;
    // DIV leaves the flags of the preceding XOR EDX,EDX (pinned oracle model).
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x004209ae, FUN_004209ae,
         "regs(esi:ptr[0x308]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_004209ae")
