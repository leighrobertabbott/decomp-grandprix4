#include <gp4/gp4.h>
#include <gp4/flags.h>

// Refreshes the six key-name captions of the controls screen for the table selected by eax. Remembers eax
// in 0x82469e; then for each of the six pairs of dwords at 0x8246b2 (offset into the 0x34-byte key table
// 0x7ab5b4 + eax * 0x34, text index): the caption buffer is the text looked up by 0x54d1ad (esi), cleared to
// fifteen spaces; when the key code word at the offset is non-zero its name is fetched with
// MapVirtualKeyA(code, 0) (thunk 0x5a496c) and GetKeyNameTextA(((scan | code & 0x2100) << 16) | 1,
// 0x8246a2, 0x10) (thunk 0x5a4966) and at most fifteen characters of it are copied over the spaces.
// pushal/popal restore every register; the flags are those of the closing `dec ecx` (ecx 1 -> 0, CF kept
// from the `add ebx, 8` before it).
// FUNCTION: GP4 0x0052e131
void __cdecl FUN_0052e131(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x0082469e) = r->eax;
    uint32_t entry = 0x008246b2u;
    uint32_t left = 6u;
    uint32_t flags;
    for (;;) {
        gp4::Regs c = *r;
        c.esi = GP4_FIELD(uint32_t, entry, 4);
        gp4::call_regs(0x0054d1ad, c);
        const uint32_t caption = c.esi;
        for (uint32_t i = 0u; i < 0xfu; ++i) {
            GP4_FIELD(uint8_t, caption, i) = 0x20u;
        }
        uint32_t keycode_address = GP4_GLOBAL(uint32_t, 0x0082469e) * 0x34u;
        keycode_address += 0x007ab5b4u;
        keycode_address += GP4_FIELD(uint32_t, entry, 0);
        const uint32_t code = GP4_FIELD(uint16_t, keycode_address, 0);
        if (code != 0u) {
            const uint32_t modifiers = code & 0x2100u;
            const uint32_t scan = gp4::stdcall_<0x005a496c, uint32_t>(code, 0u);
            const uint32_t lparam = ((scan | modifiers) << 16) | 1u;
            gp4::stdcall_<0x005a4966, uint32_t>(lparam, 0x008246a2u, 0x10u);
            uint32_t from = 0x008246a2u;
            uint32_t to = caption;
            uint32_t room = 0xfu;
            while (room != 0u && GP4_FIELD(uint8_t, from, 0) != 0u) {
                GP4_FIELD(uint8_t, to, 0) = GP4_FIELD(uint8_t, from, 0);
                ++from;
                ++to;
                --room;
            }
        }
        flags = gp4::flags::dec32(left, gp4::flags::add32(entry, 8u));
        entry += 8u;
        --left;
        if (left == 0u) {
            break;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0052e131, FUN_0052e131, "regs(eax:u32[0..3]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 MapVirtualKeyA")
