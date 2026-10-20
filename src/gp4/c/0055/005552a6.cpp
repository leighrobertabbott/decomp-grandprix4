#include <gp4/gp4.h>
#include <gp4/flags.h>

// Two 16-bit angles (0x00401315: atan of the vector (edx, eax) in 1/0x10000 turns) of the difference
// vectors between the point pairs (0x00871d10, 0x00873910) - (0x00871cd8, 0x008738d8) and
// (0x00871d14, 0x00873914) - (0x00871cdc, 0x008738dc); they are stored in the words edi+0x6c and
// edi+0x6e. pushal/popal around it: registers preserved, the flags are those of the second 0x00401315.
// FUNCTION: GP4 0x005552a6
void __cdecl FUN_005552a6(gp4::Regs* r) {
    uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.eax = GP4_GLOBAL(uint32_t, 0x00871d10) - GP4_GLOBAL(uint32_t, 0x00871cd8);
    c.edx = GP4_GLOBAL(uint32_t, 0x00873910) - GP4_GLOBAL(uint32_t, 0x008738d8);
    gp4::call_regs(0x00401315, c);
    GP4_FIELD(uint16_t, rec, 0x6c) = static_cast<uint16_t>(c.eax);

    c.eax = GP4_GLOBAL(uint32_t, 0x00871d14) - GP4_GLOBAL(uint32_t, 0x00871cdc);
    c.edx = GP4_GLOBAL(uint32_t, 0x00873914) - GP4_GLOBAL(uint32_t, 0x008738dc);
    gp4::call_regs(0x00401315, c);
    GP4_FIELD(uint16_t, rec, 0x6e) = static_cast<uint16_t>(c.eax);

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x005552a6, FUN_005552a6, "regs(edi:ptr[0x78]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
