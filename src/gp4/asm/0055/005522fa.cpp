#include <gp4/gp4.h>
#include <gp4/flags.h>

// Runs the per-segment draw setup 0x00552126 + 0x0055218f over the 0x160-byte ring records from edi up to
// (not including) ebx in chunks of [0x009631e4] records. pushal/popal wrapper: every register comes back
// unchanged, only the flags of the last compare remain. Each round stores the chunk start in [0x009631fc], the
// chunk length in [0x009631f8] (the chunk size, or the records left up to the end [0x009631f0] when the chunk
// would pass it) and the chunk end in [0x00963200]; chunks of fewer than 2 records end the routine (flags of
// `cmp length, 2`). The middle record of the chunk, wrapped like the setup's own walk (lap end [0x00866ecc] /
// lap length [0x00866ec4] before the pit start [0x00891500], else clamped to the last pit record [0x00866ef8]),
// goes to [0x009631e8]; the next chunk starts at the chunk end and the loop ends when it reaches ebx (flags of
// that compare).
// FUNCTION: GP4 0x005522fa
void __cdecl FUN_005522fa(gp4::Regs* r) {
    uint32_t record = r->edi;
    GP4_GLOBAL(uint32_t, 0x009631ec) = r->edi;
    GP4_GLOBAL(uint32_t, 0x009631f0) = r->ebx;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    uint32_t arith;
    for (;;) {
        GP4_GLOBAL(uint32_t, 0x009631fc) = record;
        GP4_GLOBAL(uint32_t, 0x009631f8) = GP4_GLOBAL(uint32_t, 0x009631e4);
        uint32_t chunkEnd = GP4_GLOBAL(uint32_t, 0x009631f8) * 0x160u + GP4_GLOBAL(uint32_t, 0x009631fc);
        if (chunkEnd > GP4_GLOBAL(uint32_t, 0x009631f0)) {
            int32_t remainder;
            GP4_GLOBAL(int32_t, 0x009631f8) = gp4::idiv64(
                static_cast<int64_t>(GP4_GLOBAL(uint32_t, 0x009631f0) - GP4_GLOBAL(uint32_t, 0x009631fc)), 0x160, &remainder);
            chunkEnd = GP4_GLOBAL(uint32_t, 0x009631f0);
        }
        GP4_GLOBAL(uint32_t, 0x00963200) = chunkEnd;

        const int32_t length = GP4_GLOBAL(int32_t, 0x009631f8);
        if (length < 2) {
            arith = gp4::flags::sub32(static_cast<uint32_t>(length), 2u);
            break;
        }

        const uint32_t half = (static_cast<uint32_t>(length) >> 1) * 0x160u;
        uint32_t middle = GP4_GLOBAL(uint32_t, 0x009631fc);
        if (middle < GP4_GLOBAL(uint32_t, 0x00891500)) {
            middle += half;
            if (middle >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
                middle -= GP4_GLOBAL(uint32_t, 0x00866ec4);
            }
        } else {
            middle += half;
            if (middle >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
                middle = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
            }
        }
        GP4_GLOBAL(uint32_t, 0x009631e8) = middle;

        c.edi = middle;
        gp4::call_regs(0x00552126, c);
        gp4::call_regs(0x0055218f, c);

        record = GP4_GLOBAL(uint32_t, 0x00963200);
        if (record == GP4_GLOBAL(uint32_t, 0x009631f0)) {
            arith = gp4::flags::sub32(record, GP4_GLOBAL(uint32_t, 0x009631f0));
            break;
        }
    }

    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005522fa, FUN_005522fa, "regs(edi:rec[0x8943a0,0x160,0..6], ebx:rel[edi,0x160,2..8]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=geom_008943a0+span_00551227+mask_005520a0+ring_00555b28+state_00552126+rows_0055218f")
