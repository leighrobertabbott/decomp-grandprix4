#include <gp4/gp4.h>
#include <gp4/flags.h>

// Distance test between the position stored at +0x1c of the record [0x00679d30] (eax) and the entry [0x00679bd8] of
// the position table 0x006f326c (edx), both compared against the limit [0x00891500]. CF is the answer (stc / clc):
// set when both lie on the same side of the limit and eax - edx is non-negative, or, when both are below it, when
// the difference taken the short way round (a span of at least [0x00866ed8] is wrapped by [0x00866ec4]) is
// non-negative. ecx receives |eax - edx| (or the wrapped span) only on the below-limit path; every other register
// effect is eax / ebx / edx. The flags are those of the last writer before the clc / stc.
// FUNCTION: GP4 0x00525018
void __cdecl FUN_00525018(gp4::Regs* r) {
    const uint32_t index = GP4_GLOBAL(uint32_t, 0x00679bd8);
    const uint32_t edx = GP4_ARRAY(uint32_t, 0x006f326c)[index];
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00679d30)));
    uint32_t eax = GP4_FIELD(uint32_t, car, 0x1c);
    uint32_t ecx = r->ecx;
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00891500);

    uint32_t f = gp4::flags::sub32(eax, limit);
    uint32_t carry = 0u;
    if (eax >= limit) {
        f = gp4::flags::sub32(edx, limit);
        if (edx >= limit) {
            f = gp4::flags::sub32(eax, edx);
            eax -= edx;
            carry = (static_cast<int32_t>(eax) >= 0) ? 1u : 0u;
        }
    } else {
        f = gp4::flags::sub32(edx, limit);
        if (edx < limit) {
            f = gp4::flags::sub32(eax, edx);
            eax -= edx;
            ecx = eax;
            f = gp4::flags::logic32(ecx);
            if (static_cast<int32_t>(ecx) < 0) {
                f = gp4::flags::neg32(ecx);
                ecx = 0u - ecx;
            }
            f = gp4::flags::sub32(ecx, GP4_GLOBAL(uint32_t, 0x00866ed8));
            if (static_cast<int32_t>(ecx) >= static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00866ed8))) {
                f = gp4::flags::neg32(ecx);
                ecx = 0u - ecx;
                f = gp4::flags::add32(ecx, GP4_GLOBAL(uint32_t, 0x00866ec4));
                ecx += GP4_GLOBAL(uint32_t, 0x00866ec4);
                f = gp4::flags::neg32(eax);
                eax = 0u - eax;
            }
            f = gp4::flags::logic32(eax);
            carry = (static_cast<int32_t>(eax) >= 0) ? 1u : 0u;
        }
    }

    r->eax = eax;
    r->ebx = index;
    r->ecx = ecx;
    r->edx = edx;
    r->eflags = gp4::flags::merge(r->eflags, (f & ~gp4::flags::CF) | carry);
}
GP4_IMPL(0x00525018, FUN_00525018, "regs(ecx:u32) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ptrtab_00524fa4+state_0040f5f3")
