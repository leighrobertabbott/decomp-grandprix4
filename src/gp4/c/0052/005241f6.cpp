#include <gp4/gp4.h>
#include <gp4/flags.h>

// Resets a block of display/replay state: clears the bytes/words/dwords 0x00679b4e, 0x00865c14,
// 0x00852d64, 0x00852dbc, 0x00852dc0, 0x00852d44, 0x00852d4c, 0x00934e94, 0x006f2c28, sets
// [0x00852d8c] = 0x80 and the word [0x00852d68] = 0x6000, copies the clock [0x0066d814] to
// [0x00679b08], then runs the state loader wrapper 0x00520be3 and the counter reset 0x00525270.
// pushal/popal: no register changes; the flags are those 0x00520be3 leaves (0x00525270 keeps them).
// FUNCTION: GP4 0x005241f6
void __cdecl FUN_005241f6(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x00679b4e) = 0u;
    GP4_GLOBAL(uint32_t, 0x00865c14) = 0u;
    GP4_GLOBAL(uint8_t, 0x00852d8c) = 0x80u;
    GP4_GLOBAL(uint32_t, 0x00852d64) = 0u;
    GP4_GLOBAL(uint16_t, 0x00852d68) = 0x6000u;
    GP4_GLOBAL(uint32_t, 0x00852dbc) = 0u;
    GP4_GLOBAL(uint32_t, 0x00852dc0) = 0u;
    GP4_GLOBAL(uint32_t, 0x00852d44) = 0u;
    GP4_GLOBAL(uint32_t, 0x00852d4c) = 0u;
    GP4_GLOBAL(uint32_t, 0x00934e94) = 0u;
    GP4_GLOBAL(uint8_t, 0x006f2c28) = 0u;
    GP4_GLOBAL(uint32_t, 0x00679b08) = GP4_GLOBAL(uint32_t, 0x0066d814);

    gp4::Regs c = *r;
    gp4::call_regs(0x00520be3, c);
    gp4::call_regs(0x00525270, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x005241f6, FUN_005241f6, "regs(edi:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slot_00520b1b+copy_0063b814+links_0066fef0+ptrs_0066d818+ring_008943a0+records_008943a0+segw_00441e25+words_008943a0+ptr_0086850c+idx_00520b65+idx_00520be3")
