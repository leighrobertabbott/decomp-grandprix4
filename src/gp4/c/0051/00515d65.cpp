#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00515d65
// Builds a five-entry threshold test for a car record (esi). 0x0041ae0e returns a base value in eax
// (from the record's +0x72 and +0x1cd bytes); records whose signed byte +0x72 is below 2 lose 0xaf0.
// Entry 0 of the dword array 0x0088ff1c becomes eax - 0x180a; then bit i of the mask 0x00866138 is
// set when the record's word +0x62 minus 0x180a is not below entry i (signed compare). eax ends as
// the mask, edx as 5 and the flags are those of the final `cmp edx, 5`; esi is restored.
void __cdecl FUN_00515d65(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0041ae0e, c);

    const uint8_t* body = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    int32_t base = static_cast<int32_t>(c.eax);
    if (static_cast<int8_t>(body[0x72]) < 2) {
        base -= 0xaf0;
    }
    base -= 0x180a;
    GP4_ARRAY(int32_t, 0x0088ff1c)[0] = base;

    const int32_t position = static_cast<int32_t>(*reinterpret_cast<const uint16_t*>(body + 0x62)) - 0x180a;
    uint32_t mask = 0u;
    for (uint32_t i = 0; i != 5u; ++i) {
        if (!(position < GP4_ARRAY(int32_t, 0x0088ff1c)[i])) {
            mask |= 1u << i;
        }
    }
    GP4_GLOBAL(uint32_t, 0x00866138) = mask;

    r->eax = mask;
    r->edx = 5u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(5u, 5u));
}
GP4_IMPL(0x00515d65, FUN_00515d65, "regs(esi:ptr[0x1d5]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
