#include <gp4/gp4.h>
#include <gp4/flags.h>

// Selects the 3-dword pair/triple published at 0x00811a94 / 0x00811a98 / 0x00811a9c: with the flag byte
// [0x66d7a4] bits 0x80 set, 0x40 clear and 0x20 clear the entry [0x679bd8] (0..31) of the parallel tables
// 0x006f37ec / 0x006f386c / 0x006f38ec is copied; in every other state they are reset to 0, 0, 0xffff.
// eax/ebx/esi are saved and restored; the flags are those of the last `test byte [0x66d7a4], mask`
// executed (0x80, then 0x40, then 0x20 on the path taken).
// FUNCTION: GP4 0x005252cf
void __cdecl FUN_005252cf(gp4::Regs* r) {
    const uint32_t state = GP4_GLOBAL(uint8_t, 0x0066d7a4);
    uint32_t mask = 0x80u;
    bool copy = false;
    if ((state & 0x80u) != 0u) {
        mask = 0x40u;
        if ((state & 0x40u) == 0u) {
            mask = 0x20u;
            copy = (state & 0x20u) == 0u;
        }
    }

    if (copy) {
        const uint32_t index = GP4_GLOBAL(uint32_t, 0x00679bd8);
        GP4_GLOBAL(uint32_t, 0x00811a94) = GP4_ARRAY(uint32_t, 0x006f37ec)[index];
        GP4_GLOBAL(uint32_t, 0x00811a98) = GP4_ARRAY(uint32_t, 0x006f386c)[index];
        GP4_GLOBAL(uint32_t, 0x00811a9c) = GP4_ARRAY(uint32_t, 0x006f38ec)[index];
    } else {
        GP4_GLOBAL(uint32_t, 0x00811a94) = 0u;
        GP4_GLOBAL(uint32_t, 0x00811a9c) = 0xffffu;
        GP4_GLOBAL(uint32_t, 0x00811a98) = 0u;
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(state & mask));
}
GP4_IMPL(0x005252cf, FUN_005252cf, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ptrtab_00524fa4")
