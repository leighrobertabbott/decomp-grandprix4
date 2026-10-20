#include <gp4/gp4.h>
#include <gp4/flags.h>

// Converts bit 0x400 of edx into the sign bit: edx = 0x80000000 when (edx & 0x400) is set, else 0.
// eax is saved and restored. The flags are those of the last writer: `test edx, 0x400` (result 0)
// or `or eax, 0x80000000`, i.e. the logic flags of the value left in edx.
// FUNCTION: GP4 0x00559bd0
void __cdecl FUN_00559bd0(gp4::Regs* r) {
    const uint32_t result = (r->edx & 0x400u) != 0u ? 0x80000000u : 0u;
    r->edx = result;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(result));
}
GP4_IMPL(0x00559bd0, FUN_00559bd0, "regs(edx:u32) -> (edx, cf, pf, af, zf, sf, of)")
