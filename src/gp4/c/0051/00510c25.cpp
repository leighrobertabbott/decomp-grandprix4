#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: takes the ring record for the index [0xa69878] (0x160 bytes per record) relative
// to the pit start [0x891500] (or [0x891504] when [0xa6987c] is nonzero), lets 0x00553d3d copy the
// three dwords at +0xf0/+0xf4/+0xf8 of that record to 0x870640/44/48 and then stores those three in
// 0xa6987c/0xa69880/0xa69884. popal restores every register, so the only output is the flags of
// the last flag writer, the `add edi, [base]`.
// FUNCTION: GP4 0x00510c25
void __cdecl FUN_00510c25(gp4::Regs* r) {
    const uint32_t record_offset = GP4_GLOBAL(uint32_t, 0x00a69878) * 0x160u;
    uint32_t base;
    if (GP4_GLOBAL(uint32_t, 0x00a6987c) == 0u) {
        base = GP4_GLOBAL(uint32_t, 0x00891500);
    } else {
        base = GP4_GLOBAL(uint32_t, 0x00891504);
    }
    gp4::Regs c = {};
    c.edi = record_offset + base;
    gp4::call_regs(0x00553d3d, c);
    GP4_GLOBAL(uint32_t, 0x00a6987c) = GP4_GLOBAL(uint32_t, 0x00870640);
    GP4_GLOBAL(uint32_t, 0x00a69880) = GP4_GLOBAL(uint32_t, 0x00870644);
    GP4_GLOBAL(uint32_t, 0x00a69884) = GP4_GLOBAL(uint32_t, 0x00870648);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(record_offset, base));
}
GP4_IMPL(0x00510c25, FUN_00510c25, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+idx_005162fd+idx_00510a72")
