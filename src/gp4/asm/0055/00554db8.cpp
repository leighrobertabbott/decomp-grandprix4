#include <gp4/gp4.h>
#include <gp4/flags.h>

// Signed byte of the 64 KiB table at 0x0096a3d8.
static __declspec(noinline) int32_t tableByte(uint32_t index) {
    return GP4_ARRAY(int8_t, 0x0096a3d8)[index];
}

// Brightness level 0..15 of a packed 3-byte value (eax bytes 0..2): each byte is combined with one of the
// selector bytes [0x9e1668], [0x9e1669], [0x9e166a] (the high byte of a 16-bit table index) and looked up in the
// signed byte table 0x96a3d8; the three entries are summed in 8 bits (dl), sign-extended, scaled by Q14
// factors [0x811924] and [0x891214] (imul/shrd 14 each), offset by [0x891218] and clamped to 0..15. The six
// pushed registers are restored; the flags are those of the final `cmp eax, 0` (after the clamp to 15).
// FUNCTION: GP4 0x00554db8
void __cdecl FUN_00554db8(gp4::Regs* r) {
    const uint32_t packed = r->eax;
    const uint32_t index0 = (static_cast<uint32_t>(GP4_GLOBAL(uint8_t, 0x009e1668)) << 8) | (packed & 0xffu);
    const uint32_t index1 = (static_cast<uint32_t>(GP4_GLOBAL(uint8_t, 0x009e1669)) << 8) | ((packed >> 8) & 0xffu);
    const uint32_t index2 = (static_cast<uint32_t>(GP4_GLOBAL(uint8_t, 0x009e166a)) << 8) | ((packed >> 16) & 0xffu);

    const uint8_t sum = static_cast<uint8_t>(static_cast<uint8_t>(tableByte(index0)) + static_cast<uint8_t>(tableByte(index1))
                                             + static_cast<uint8_t>(tableByte(index2)));
    const int32_t level = static_cast<int8_t>(sum);

    const int64_t first = static_cast<int64_t>(level) * static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00811924));
    const uint32_t scaled = (static_cast<uint32_t>(first) >> 14) | (static_cast<uint32_t>(first >> 32) << 18);

    const int64_t second = static_cast<int64_t>(static_cast<int32_t>(scaled)) * static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00891214));
    uint32_t value = (static_cast<uint32_t>(second) >> 14) | (static_cast<uint32_t>(second >> 32) << 18);
    value += GP4_GLOBAL(uint32_t, 0x00891218);

    if (static_cast<int32_t>(value) > 0xf) {
        value = 0xfu;
    }
    const uint32_t flags = gp4::flags::sub32(value, 0u);
    if (static_cast<int32_t>(value) < 0) {
        value = 0u;
    }
    r->eax = value;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00554db8, FUN_00554db8, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
