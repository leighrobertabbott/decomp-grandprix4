#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00535239
// Round-robin search over four flag bytes at 0x00b1bb74: starting at the index [0x007ab4b4] (an index
// of 4 restarts at 0), takes the first nonzero byte and stores its index back in [0x007ab4b4]; when all
// four bytes are zero the index becomes 4. ebx and ecx are saved and restored; edx = the stored index.
// The flags are those of the last flag writer: `cmp byte [edx + ebx], 0` when a byte is found, and
// the final `dec ecx` (CF kept from the preceding `cmp edx, 4`) when none is.
void __cdecl FUN_00535239(gp4::Regs* r) {
    const uint8_t* flagBytes = GP4_ARRAY(uint8_t, 0x00b1bb74);
    uint32_t edx = GP4_GLOBAL(uint32_t, 0x007ab4b4);
    uint32_t ecx = 4u;
    uint32_t arith = gp4::flags::sub32(edx, 4u);   // cmp edx, 4
    if (edx == 4u) edx = 0u;

    for (;;) {
        arith = gp4::flags::sub8(flagBytes[edx], 0u);   // cmp byte [edx + ebx], 0
        if (flagBytes[edx] != 0u) break;
        edx += 1u;
        arith = gp4::flags::sub32(edx, 4u);             // cmp edx, 4
        if (edx == 4u) edx = 0u;
        ecx -= 1u;
        arith = gp4::flags::dec32(ecx + 1u, arith);     // dec ecx
        if (ecx == 0u) {
            edx = 4u;
            break;
        }
    }

    GP4_GLOBAL(uint32_t, 0x007ab4b4) = edx;
    r->edx = edx;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00535239, FUN_00535239, "regs(edx:u32) -> (edx, cf, pf, af, zf, sf, of); globals=layout=idx_00535239")
