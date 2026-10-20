#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies the 410 bytes 0x009619f8..0x00961b92 to 0x00961b9c in chunks of ten (rep movsb inside a
// pushal/popal pair, so every register comes back unchanged). The loop ends when the source
// pointer reaches 0x00961b92; the last flag writer is that `cmp esi, 0x961b92` of equal values.
// FUNCTION: GP4 0x00559008
void __cdecl FUN_00559008(gp4::Regs* r) {
    const uint8_t* src = GP4_ARRAY(uint8_t, 0x009619f8);
    uint8_t* dst = GP4_ARRAY(uint8_t, 0x00961b9c);
    const uint8_t* const end = GP4_ARRAY(uint8_t, 0x00961b92);

    do {
        for (uint32_t count = 10u; count != 0u; --count) {
            *dst++ = *src++;
        }
    } while (src < end);

    const uint32_t esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(src));
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(esi, 0x00961b92u));
}
GP4_IMPL(0x00559008, FUN_00559008, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=copy_0055748e")
