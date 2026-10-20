#include <gp4/gp4.h>
#include <gp4/flags.h>

// Maps the signed word in ax through two word tables (indexed by esi) and two Q14 scale factors to a
// 4-bit value in 0..15, and shifts it into the low nibble of ebx (`ebx = (ebx << 4) | value`).
//   p   = ax * int16 [0x89123e + esi*2]          (32-bit product)
//   dx  = bits 14..29 of p, plus the word [0x891236 + esi*2]
//   q   = (sext(dx) * [0x811924]) >> 14          (64-bit product, low 32 bits kept)
//   r   = (q * [0x891214]) >> 14   (edx keeps the high half of this last 64-bit product)
//   value = clamp((r >> 7) + [0x891218], 0, 15)
// Leaves eax = value, edx = the high half of the second 64-bit product, and the flags of the final
// `or bl, al`.
// FUNCTION: GP4 0x0055485e
void __cdecl FUN_0055485e(gp4::Regs* r) {
    const uint32_t index = r->esi;
    const int16_t scale = GP4_ARRAY(int16_t, 0x0089123e)[index];
    const int16_t bias = GP4_ARRAY(int16_t, 0x00891236)[index];

    const int32_t product = static_cast<int32_t>(static_cast<int16_t>(r->eax)) * static_cast<int32_t>(scale);
    const uint16_t dx = static_cast<uint16_t>(static_cast<uint16_t>(static_cast<uint32_t>(product) >> 14) +
                                              static_cast<uint16_t>(bias));

    const int64_t first = static_cast<int64_t>(static_cast<int32_t>(static_cast<int16_t>(dx))) *
                          static_cast<int64_t>(static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00811924)));
    const int32_t q = static_cast<int32_t>(static_cast<uint32_t>(first >> 14));

    const int64_t second = static_cast<int64_t>(q) *
                           static_cast<int64_t>(static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00891214)));
    const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 32);
    int32_t value = static_cast<int32_t>(static_cast<uint32_t>(second >> 14));

    value >>= 7;
    value = static_cast<int32_t>(static_cast<uint32_t>(value) + GP4_GLOBAL(uint32_t, 0x00891218));
    if (value > 0xf) {
        value = 0xf;
    }
    if (value < 0) {
        value = 0;
    }

    const uint32_t ebx = r->ebx << 4;
    const uint8_t bl = static_cast<uint8_t>(static_cast<uint8_t>(ebx) | static_cast<uint8_t>(value));

    r->eax = static_cast<uint32_t>(value);
    r->edx = high;
    r->ebx = (ebx & 0xffffff00u) | bl;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(bl));
}
GP4_IMPL(0x0055485e, FUN_0055485e, "regs(eax:u32, esi:u32[0..3], ebx:u32) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
