#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sound-channel release. eax = channel k (0..15), or -1 for every channel.
// k: clears bit k of the two channel masks 0x0088f704 / 0x0088f700 (btr with a register bit offset, so
// the dword is picked by k >> 5), passes the status dword 0x0088f948[k] to the empty stdcall stub
// 0x0050a060 and zeroes the flag byte 0x0088f988[k]; eax and the flags are those of `cmp eax, -1` with
// CF replaced by the old bit of 0x0088f700.
// -1: zeroes both masks, calls the empty routine 0x005ae990 and clears the 16 flag bytes 0x0088f988[0..15];
// leaves eax = 0x10 and the flags of the final `cmp eax, 0x10`.

static inline uint32_t* MaskWord(uint32_t* base, uint32_t bit) {
    return base + (static_cast<int32_t>(bit) >> 5);
}

// FUNCTION: GP4 0x00518ba7
void __cdecl FUN_00518ba7(gp4::Regs* r) {
    const uint32_t channel = r->eax;

    if (channel == 0xffffffffu) {
        GP4_GLOBAL(uint32_t, 0x0088f704) = 0u;
        GP4_GLOBAL(uint32_t, 0x0088f700) = 0u;
        gp4::cdecl_<0x005ae990, void>();
        for (uint32_t i = 0u; i != 0x10u; ++i) {
            GP4_ARRAY(uint8_t, 0x0088f988)[i] = 0u;
        }
        r->eax = 0x10u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0x10u, 0x10u));
        return;
    }

    const uint32_t bit = 1u << (channel & 31u);
    *MaskWord(&GP4_ARRAY(uint32_t, 0x0088f704)[0], channel) &= ~bit;
    uint32_t* mask = MaskWord(&GP4_ARRAY(uint32_t, 0x0088f700)[0], channel);
    const uint32_t old_bit = (*mask & bit) != 0u ? 1u : 0u;
    *mask &= ~bit;
    gp4::stdcall_<0x0050a060, void>(GP4_ARRAY(uint32_t, 0x0088f948)[channel]);
    GP4_ARRAY(uint8_t, 0x0088f988)[channel] = 0u;

    const uint32_t flags = (gp4::flags::sub32(channel, 0xffffffffu) & ~static_cast<uint32_t>(gp4::flags::CF)) | old_bit;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00518ba7, FUN_00518ba7, "regs(eax:i32[-1..15]) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
