#include <gp4/gp4.h>

// Looks up the pointer 0x43bb15 leaves in [0x00601f24] for record esi while byte +0 of that
// record (0x7adb50 + esi*0xf28 + circuit*0xe4, read/written through 0x0051f816 / 0x0051f82c)
// temporarily holds the low byte of eax; the original byte is put back before returning.
// eax = the pointer, every other register is preserved by the push/pop pairs; the flags are those of
// the last flag writer (the `add eax, 0x7adb4c` in 0x0052a260, reached through the final 0x0051f82c).
// FUNCTION: GP4 0x0043bbad
void __cdecl FUN_0043bbad(gp4::Regs* r) {
    const uint32_t newByte = r->eax;
    const uint32_t record = r->esi;

    gp4::Regs step = *r;
    step.st_in = 0;
    step.st_out = 0;

    // old = byte +0 of the record
    step.ebx = GP4_GLOBAL(uint32_t, 0x007ad894);
    step.edx = 0;
    step.eax = record;
    gp4::call_regs(0x0051f816, step);
    const uint32_t oldByte = step.eax;

    // byte +0 = cl
    step.ecx = newByte;
    step.ebx = GP4_GLOBAL(uint32_t, 0x007ad894);
    step.edx = 0;
    step.eax = record;
    gp4::call_regs(0x0051f82c, step);

    // refresh the cached bytes and pointer at 0x601fa8.. / 0x601f24 (eax = record)
    gp4::call_regs(0x0043bb15, step);

    const uint32_t pointer = GP4_GLOBAL(uint32_t, 0x00601f24);

    // restore byte +0
    step.ecx = oldByte;
    step.ebx = GP4_GLOBAL(uint32_t, 0x007ad894);
    step.edx = 0;
    step.eax = record;
    gp4::call_regs(0x0051f82c, step);

    r->eax = pointer;
    r->eflags = (r->eflags & ~0x8d5u) | (step.eflags & 0x8d5u);
}
GP4_IMPL(0x0043bbad, FUN_0043bbad, "regs(eax:u32, esi:i32[0..21]) -> (eax, cf, pf, af, zf, sf, of); globals=layout=trk_007ad894+circ_007adb50+codes_0043ba12")
