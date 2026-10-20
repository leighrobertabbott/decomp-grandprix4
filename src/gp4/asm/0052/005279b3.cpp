#include <gp4/gp4.h>
#include <gp4/flags.h>

// Builds the reverse map of the 22 per-driver slot bytes at 0x007c2a48: for every slot i whose byte
// is nonzero, the byte at 0x007c2b0e + ((slot - 1) & 0x3f) is set to i. pushal/popal keeps every
// register; the flags are those of the final `inc edx` (0x15 -> 0x16) with the carry left clear by
// the `cmp al, 0` / `and eax, 0x3f` before it, so they are the same in every case.
// FUNCTION: GP4 0x005279b3
void __cdecl FUN_005279b3(gp4::Regs* r) {
    const uint8_t* slots = GP4_ARRAY(const uint8_t, 0x007c2a48);
    uint8_t* map = GP4_ARRAY(uint8_t, 0x007c2b0e);
    for (uint32_t i = 0u; i != 0x16u; ++i) {
        const uint32_t slot = slots[i];
        if (slot != 0u) {
            map[(slot - 1u) & 0x3fu] = static_cast<uint8_t>(i);
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::inc32(0x15u, 0u));
}
GP4_IMPL(0x005279b3, FUN_005279b3, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=drv22_0052d402")
