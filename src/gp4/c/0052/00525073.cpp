#include <gp4/gp4.h>
#include <gp4/flags.h>

// Variant of 0x00525018 that first steps the selection index backwards (or forwards when bit 0x80 of
// the key byte [0x0066d7a6] is set): the key byte gets bit 0x40 for the call of 0x00524fdc and
// loses it afterwards. The old index is pushed first. Then, with eax = the position at +0x1c of the
// record [0x00679d30], edx = position table 0x006f326c[new index] and the limit [0x00891500]:
// below the limit the distance |eax - edx| (wrapped the short way round when it reaches
// [0x00866ed8]) must be under 0xdc0 and the signed difference negative to answer CF = 1 (the old
// index is dropped, `add esp, 4; stc`); above it both must lie above the limit and eax - edx must
// be negative. Every other path restores the old index (pop) and returns CF = 0 (clc).
// The `add esp, 4` makes the other arithmetic flags of the CF = 1 exit a function of the stack
// pointer (pinned oracle model: PF only).
// FUNCTION: GP4 0x00525073
void __cdecl FUN_00525073(gp4::Regs* r) {
    const uint32_t saved = GP4_GLOBAL(uint32_t, 0x00679bd8);
    GP4_GLOBAL(uint8_t, 0x0066d7a6) = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x0066d7a6) | 0x40u);
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00524fdc, c);
    GP4_GLOBAL(uint8_t, 0x0066d7a6) = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x0066d7a6) & 0xbfu);

    const uint32_t index = GP4_GLOBAL(uint32_t, 0x00679bd8);
    const uint32_t edx = GP4_ARRAY(uint32_t, 0x006f326c)[index];
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00679d30)));
    uint32_t eax = GP4_FIELD(uint32_t, car, 0x1c);
    uint32_t ecx = r->ecx;
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00891500);

    uint32_t f = gp4::flags::sub32(eax, limit);
    bool found = false;
    if (eax < limit) {
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
            f = gp4::flags::sub32(ecx, 0xdc0u);
            if (ecx < 0xdc0u) {
                f = gp4::flags::logic32(eax);
                found = static_cast<int32_t>(eax) < 0;
            }
        }
    } else {
        f = gp4::flags::sub32(edx, limit);
        if (edx >= limit) {
            f = gp4::flags::sub32(eax, edx);
            eax -= edx;
            found = static_cast<int32_t>(eax) < 0;
        }
    }

    r->eax = eax;
    r->ebx = index;
    r->ecx = ecx;
    r->edx = edx;
    if (found) {
        // `add esp, 4; stc`: the flags of the stack-pointer add, CF forced.
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::CF | gp4::flags::PF);
    } else {
        GP4_GLOBAL(uint32_t, 0x00679bd8) = saved;
        r->eflags = gp4::flags::merge(r->eflags, f & ~gp4::flags::CF);
    }
}
GP4_IMPL(0x00525073, FUN_00525073, "regs(ecx:u32) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ptrtab_00524fa4+ring_00525205+pos_00525073")
