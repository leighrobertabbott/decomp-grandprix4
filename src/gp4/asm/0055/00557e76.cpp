#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies one 4-byte entry {word value, word extra} at esi into ten slot tables (pushal/popal around
// everything): for every set bit n (0..9) of the selector mask in eax, the word at esi goes to
// 0x00874a34[n] and 0x00874a98[n], the byte tables 0x00875918[k] / 0x00875920[k] (k = value - 1
// clamped to 0..7, signed 16-bit) are widened into 0x00874a48[n]/0x00874aac[n] and
// 0x00874a5c[n]/0x00874ac0[n], and the word at esi + 2 goes to 0x00874a84[n] and 0x00874ae8[n].
// Every register comes back unchanged; the flags are those of the last `add ebx, 2` (18 + 2).
// FUNCTION: GP4 0x00557e76
void __cdecl FUN_00557e76(gp4::Regs* r) {
    uint32_t mask = r->eax;
    const uint8_t* entry = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));

    for (uint32_t slot = 0u; slot < 10u; ++slot) {
        const bool set = (mask & 1u) != 0u;
        mask >>= 1;
        if (set) {
            const uint16_t value = *reinterpret_cast<const uint16_t*>(entry);
            GP4_ARRAY(uint16_t, 0x00874a34)[slot] = value;
            GP4_ARRAY(uint16_t, 0x00874a98)[slot] = value;

            int16_t index = static_cast<int16_t>(value - 1);
            if (index > 7) {
                index = 7;
            }
            if (index < 0) {
                index = 0;
            }
            const uint16_t first = GP4_ARRAY(uint8_t, 0x00875918)[static_cast<uint16_t>(index)];
            GP4_ARRAY(uint16_t, 0x00874a48)[slot] = first;
            GP4_ARRAY(uint16_t, 0x00874aac)[slot] = first;
            const uint16_t second = GP4_ARRAY(uint8_t, 0x00875920)[static_cast<uint16_t>(index)];
            GP4_ARRAY(uint16_t, 0x00874a5c)[slot] = second;
            GP4_ARRAY(uint16_t, 0x00874ac0)[slot] = second;

            const uint16_t extra = *reinterpret_cast<const uint16_t*>(entry + 2);
            GP4_ARRAY(uint16_t, 0x00874a84)[slot] = extra;
            GP4_ARRAY(uint16_t, 0x00874ae8)[slot] = extra;
        }
    }

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(18u, 2u));
}
GP4_IMPL(0x00557e76, FUN_00557e76, "regs(eax:u32, esi:ptr[0x10]:i16[-3..12]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=steps_00875918")
