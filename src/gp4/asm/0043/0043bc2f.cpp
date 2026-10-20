#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `add left, right`.
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x001u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 31) << 7)
        | ((((left ^ result) & (right ^ result)) >> 31) << 11);
}

// FUNCTION: GP4 0x0043bc2f
// For the driver in esi (0..21) of the current circuit [0x7ad894]: stores the four code bytes
// [0x601fa8], 0, 0xff, 0 at +0, +2, +1, +3 of its 0x7adb4c-based record (0x0051f82c: address
// 0x52a260 = driver * 0xf28 + circuit * 0xe4 + 0x7adb50, byte edx, value cl), then copies the 0x38-byte
// entry number [0x601fa8] of the table at 0x7ad9f4 into the driver's slot 0x0051f837 returns
// (driver * 0xf28 + circuit * 0xe4 + slot * 0x38 + 0x7adb54, slot = [0x601fa8]). pushal / popal: every
// register is preserved; the flags are those of `add eax, edx` computing the table address.
void __cdecl FUN_0043bc2f(gp4::Regs* r) {
    const uint32_t driver = r->esi;
    const uint32_t upper = r->ecx & 0xffffff00u;

    gp4::Regs step = *r;
    step.st_in = 0;
    step.st_out = 0;

    step.ebx = GP4_GLOBAL(uint32_t, 0x007ad894);
    step.edx = 0;
    step.ecx = upper | GP4_GLOBAL(uint8_t, 0x00601fa8);
    step.eax = driver;
    gp4::call_regs(0x0051f82c, step);

    step.ebx = GP4_GLOBAL(uint32_t, 0x007ad894);
    step.edx = 2;
    step.ecx = upper;
    step.eax = driver;
    gp4::call_regs(0x0051f82c, step);

    step.ebx = GP4_GLOBAL(uint32_t, 0x007ad894);
    step.edx = 1;
    step.ecx = upper | 0xffu;
    step.eax = driver;
    gp4::call_regs(0x0051f82c, step);

    step.ebx = GP4_GLOBAL(uint32_t, 0x007ad894);
    step.edx = 3;
    step.ecx = upper;
    step.eax = driver;
    gp4::call_regs(0x0051f82c, step);

    step.ebx = GP4_GLOBAL(uint32_t, 0x007ad894);
    step.ecx = GP4_GLOBAL(uint8_t, 0x00601fa8);
    step.eax = driver;
    gp4::call_regs(0x0051f837, step);

    uint8_t* destination = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(step.eax));
    const uint32_t entry = GP4_GLOBAL(uint8_t, 0x00601fa8) * 0x38u;
    const uint8_t* source = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(0x007ad9f4u + entry));
    for (uint32_t i = 0; i != 0x38u; ++i) {
        *destination++ = *source++;
    }

    r->eflags = (r->eflags & ~0x8d5u) | addFlags(0x007ad9f4u, entry);
}
GP4_IMPL(0x0043bc2f, FUN_0043bc2f, "regs(esi:i32[0..21]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trk_007ad894+tmpl_007ad9f4")
