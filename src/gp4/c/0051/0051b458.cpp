#include <gp4/gp4.h>
#include <gp4/flags.h>

// Expands the palette script at 0x7c4bf9 into the 0x100-entry table at 0x7c535c. Entries are
// counted in the word [0x933a62]: a negative script byte b skips -b entries (esi advances 1), a
// non-negative one starts an 8-byte gradient record handled by 0x51b4d4 (which advances esi and
// adds `count` entries), until the word reaches 0x100. The number of script items consumed goes to
// [0x7c4d58]. Then the 0x28 bytes at 0x7c4bd1 are copied as the third word (byte << 8) of the
// 0x28 six-byte entries at 0x7c535c (the first two words cleared). pushal/popal keep every
// register; the flags are those of the last `add edi, 6` (edi = 0x7c5446).
// FUNCTION: GP4 0x0051b458
void __cdecl FUN_0051b458(gp4::Regs* r) {
    GP4_GLOBAL(uint16_t, 0x00933a62) = 0;
    uint32_t script = 0x007c4bf9u;
    uint32_t items = 0;
    do {
        const int8_t b = GP4_FIELD(int8_t, script, 0);
        if (b < 0) {
            GP4_GLOBAL(uint16_t, 0x00933a62) =
                static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x00933a62) + static_cast<uint8_t>(-b));
            script += 1u;
        } else {
            gp4::Regs c = {};
            c.esi = script;
            gp4::call_regs(0x0051b4d4, c);
            script = c.esi;
        }
        items += 1u;
    } while (GP4_GLOBAL(uint16_t, 0x00933a62) < 0x100u);
    GP4_GLOBAL(uint32_t, 0x007c4d58) = items;

    const uint8_t* src = GP4_ARRAY(uint8_t, 0x007c4bd1);
    uint16_t* dst = GP4_ARRAY(uint16_t, 0x007c535c);
    for (uint32_t i = 0; i < 0x28u; ++i) {
        dst[0] = 0;
        dst[1] = 0;
        dst[2] = static_cast<uint16_t>(static_cast<uint32_t>(src[i]) << 8);
        dst += 3;
    }

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(0x007c5446u, 6u));
}
GP4_IMPL(0x0051b458, FUN_0051b458, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=pal_0051b458")
