#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043bb15
// pushal / popal wrapper (every register comes back; the flags of the last flag writer, the final
// callee 0x0051f837, remain). Clamps eax & 0x3f to 0x15 (a record index of the current circuit
// 0x007ad894) and caches four lookups of 0x0051f816 (edx = 0, 2, 1, 3) in the bytes 0x00601fa8 /
// 0x00601fa9 / 0x00601faa / 0x00601fab, then stores the pointer 0x0051f837 returns for that record
// (ecx = the first byte, reduced to its bit 0 when it is >= 2 and the second byte is 0xff) in
// 0x00601f24.
void __cdecl FUN_0043bb15(gp4::Regs* r) {
    uint32_t index = r->eax & 0x3fu;
    if (static_cast<int32_t>(index) > 0x15) {
        index = 0x15u;
    }

    gp4::Regs step = *r;
    step.st_in = 0;
    step.st_out = 0;
    step.esi = index;

    step.ebx = GP4_GLOBAL(uint32_t, 0x007ad894);
    step.edx = 0;
    step.eax = index;
    gp4::call_regs(0x0051f816, step);
    GP4_GLOBAL(uint8_t, 0x00601fa8) = static_cast<uint8_t>(step.eax);

    step.ebx = GP4_GLOBAL(uint32_t, 0x007ad894);
    step.edx = 2;
    step.eax = index;
    gp4::call_regs(0x0051f816, step);
    GP4_GLOBAL(uint8_t, 0x00601fa9) = static_cast<uint8_t>(step.eax);

    step.ebx = GP4_GLOBAL(uint32_t, 0x007ad894);
    step.edx = 1;
    step.eax = index;
    gp4::call_regs(0x0051f816, step);
    GP4_GLOBAL(uint8_t, 0x00601faa) = static_cast<uint8_t>(step.eax);

    step.ebx = GP4_GLOBAL(uint32_t, 0x007ad894);
    step.edx = 3;
    step.eax = index;
    gp4::call_regs(0x0051f816, step);
    GP4_GLOBAL(uint8_t, 0x00601fab) = static_cast<uint8_t>(step.eax);

    uint32_t first = GP4_GLOBAL(uint8_t, 0x00601fa8);
    if (static_cast<int32_t>(first) >= 2 && GP4_GLOBAL(uint8_t, 0x00601fa9) == 0xffu) {
        first &= 1u;
    }
    step.ecx = first;
    step.ebx = GP4_GLOBAL(uint32_t, 0x007ad894);
    step.eax = index;
    gp4::call_regs(0x0051f837, step);
    GP4_GLOBAL(uint32_t, 0x00601f24) = step.eax;

    r->eflags = (r->eflags & ~0x8d5u) | (step.eflags & 0x8d5u);
}
GP4_IMPL(0x0043bb15, FUN_0043bb15, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=layout=trk_007ad894+circ_007adb50+codes_0043ba12")
