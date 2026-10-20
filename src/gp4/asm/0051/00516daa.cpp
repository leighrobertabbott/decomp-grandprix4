#include <gp4/gp4.h>
#include <gp4/flags.h>

// Looks up a car's sort key: eax indexes the byte list at 0x00675958, whose low six bits (minus 1,
// clamped to 21) select one of three 22-entry dword tables chosen by edx (signed compare with 1):
// edx < 1 -> 0x006759c8, edx == 1 -> 0x00675a20, edx > 1 -> 0x00675970.
// eax = the table entry; the flags are those of `cmp edx, 1`.
// FUNCTION: GP4 0x00516daa
void __cdecl FUN_00516daa(gp4::Regs* r) {
    const uint32_t index = r->eax;
    const int32_t kind = (int32_t)r->edx;

    const uint8_t entry = GP4_ARRAY(uint8_t, 0x00675958)[index];
    uint32_t slot = ((uint32_t)entry - 1u) & 0x3fu;
    if ((int32_t)slot > 0x15) {
        slot = 0x15u;
    }

    uint32_t value;
    if (kind == 1) {
        value = GP4_ARRAY(uint32_t, 0x00675a20)[slot];
    } else if (kind > 1) {
        value = GP4_ARRAY(uint32_t, 0x00675970)[slot];
    } else {
        value = GP4_ARRAY(uint32_t, 0x006759c8)[slot];
    }

    r->eax = value;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32((uint32_t)kind, 1u));
}
GP4_IMPL(0x00516daa, FUN_00516daa, "regs(eax:u32[0..21], edx:i32[-1..3]) -> (eax, cf, pf, af, zf, sf, of); globals=layout=order_00675958+keys_006759c8")
