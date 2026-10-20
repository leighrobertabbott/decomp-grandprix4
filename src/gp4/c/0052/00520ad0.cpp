#include <gp4/gp4.h>
#include <gp4/flags.h>

// Advances the save-slot ring at index [0x00679b44]: runs 0x00510f81, saves the state into the slot
// named by the current position 0x00679b34[index] with 0x00520b1b, raises the upper mark
// 0x00679b24[index] to that position when it is above it, then steps the position by one, wrapping to
// the lower bound 0x00679b9c[index] once it passes the limit 0x00679bac[index]. pushal/popal keeps every
// register; the flags are those of the `cmp position + 1, limit`.
// FUNCTION: GP4 0x00520ad0
void __cdecl FUN_00520ad0(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    gp4::call_regs(0x00510f81, c);

    const uint32_t index = GP4_GLOBAL(uint32_t, 0x00679b44);
    c.edx = index;
    c.eax = static_cast<uint32_t>(GP4_ARRAY(int32_t, 0x00679b34)[index]);
    gp4::call_regs(0x00520b1b, c);

    int32_t position = GP4_ARRAY(int32_t, 0x00679b34)[index];
    if (position > GP4_ARRAY(int32_t, 0x00679b24)[index]) {
        GP4_ARRAY(int32_t, 0x00679b24)[index] = position;
    }
    position = static_cast<int32_t>(static_cast<uint32_t>(position) + 1u);
    const int32_t limit = GP4_ARRAY(int32_t, 0x00679bac)[index];
    const uint32_t flags = gp4::flags::sub32(static_cast<uint32_t>(position), static_cast<uint32_t>(limit));
    if (position > limit) {
        position = GP4_ARRAY(int32_t, 0x00679b9c)[index];
    }
    GP4_ARRAY(int32_t, 0x00679b34)[index] = position;

    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00520ad0, FUN_00520ad0, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slot_00520b1b+copy_0063b814+links_0066fef0+ptrs_0066d818+ring_008943a0+records_008943a0+segw_00441e25+words_008943a0+ptr_0086850c+ring_00520ad0")
