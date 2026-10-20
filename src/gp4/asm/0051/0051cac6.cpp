#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies up to sixteen slots. The sixteen words at 0x811aa8 each name a source slot (0xffff = none); the
// slot i that is used is copied to destination slot 0x42d + i in the word tables 0x9e1b20 and 0x9e2920
// and the dword table 0xa6c600 (each pair of pushes/pops of the original moves the same word), then the
// empty routine 0x51cb4b is called. Bit i of the mask stored to 0x811a8c is set for each used slot.
// pushal/popal restore every register; the flags are those of the closing `cmp ecx, 0x10` (always 0x10 - 0x10).
static __declspec(noinline) void CopySlot(uint32_t from, uint32_t to) {
    GP4_ARRAY(uint16_t, 0x009e1b20)[to] = GP4_ARRAY(uint16_t, 0x009e1b20)[from];
    GP4_ARRAY(uint16_t, 0x009e2920)[to] = GP4_ARRAY(uint16_t, 0x009e2920)[from];
    GP4_ARRAY(uint32_t, 0x00a6c600)[to] = GP4_ARRAY(uint32_t, 0x00a6c600)[from];
}

// FUNCTION: GP4 0x0051cac6
void __cdecl FUN_0051cac6(gp4::Regs* r) {
    const uint16_t* slots = GP4_ARRAY(uint16_t, 0x00811aa8);
    uint32_t mask = 0u;
    for (uint32_t i = 0u; i < 0x10u; ++i) {
        const uint32_t from = slots[i];
        if (from != 0xffffu) {
            mask |= 1u << i;
            CopySlot(from, 0x42du + i);
            gp4::cdecl_<0x0051cb4b, void>();
        }
    }
    GP4_GLOBAL(uint32_t, 0x00811a8c) = mask;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0x10u, 0x10u));
}
GP4_IMPL(0x0051cac6, FUN_0051cac6, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=copy_0051cac6")
