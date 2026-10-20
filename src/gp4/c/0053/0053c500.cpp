#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu mouse-state step: copies the pointer position [0x007a9460]/[0x007a9464] into ebx/eax and
// into [0x0082fd94]/[0x0082fd98], then, from the button bits ([0x007a9469] & 3), the latch byte
// [0x0082fd6b] bit 2, the countdown byte [0x007ab003] and a hit test (0x0053dd5f) against the
// remembered item [0x0082fd88], picks an event code 2..8 into edx and [0x0082fd9c], sets bit 2 of
// [0x0082fd6a], calls 0x0054ef3e and returns CF = 0; without an event CF = 1.
// FUNCTION: GP4 0x0053c500
void __cdecl FUN_0053c500(gp4::Regs* r) {
    const uint32_t x = GP4_GLOBAL(uint32_t, 0x007a9460);
    const uint32_t y = GP4_GLOBAL(uint32_t, 0x007a9464);
    GP4_GLOBAL(uint32_t, 0x0082fd94) = x;
    GP4_GLOBAL(uint32_t, 0x0082fd98) = y;
    r->ebx = x;
    r->eax = y;
    const uint32_t buttons = GP4_GLOBAL(uint8_t, 0x007a9469) & 3u;
    r->edx = (r->edx & 0xffffff00u) | buttons;
    uint32_t flags = gp4::flags::logic8(buttons);
    uint32_t code = 0u;
    bool event = false;
    if (buttons != 0u) {
        const uint32_t latch = GP4_GLOBAL(uint8_t, 0x0082fd6b) & 2u;
        flags = gp4::flags::logic8(latch);
        if (latch == 0u) {
            GP4_GLOBAL(uint8_t, 0x0082fd6b) = 2u;
            GP4_GLOBAL(uint8_t, 0x007ab003) = 0x0au;
            code = 2u;
            event = true;
        } else {
            const uint32_t countdown = GP4_GLOBAL(uint8_t, 0x007ab003);
            flags = gp4::flags::sub8(countdown, 0u);
            if (countdown == 0u) {
                GP4_GLOBAL(uint8_t, 0x007ab003) = 2u;
                gp4::call_regs(0x0053dd5f, *r);
                const bool same = (r->eflags & gp4::flags::CF) == 0u &&
                                  r->edx == GP4_GLOBAL(uint32_t, 0x0082fd88);
                code = same ? 3u : 4u;
                event = true;
            } else {
                gp4::call_regs(0x0053dd5f, *r);
                const bool same = (r->eflags & gp4::flags::CF) == 0u &&
                                  r->edx == GP4_GLOBAL(uint32_t, 0x0082fd88);
                const uint32_t held = GP4_GLOBAL(uint8_t, 0x0082fd84);
                flags = gp4::flags::sub8(held, 0xffu);
                if (same) {
                    if (held != 0xffu) {
                        code = 8u;
                        event = true;
                    }
                } else {
                    if (held == 0xffu) {
                        code = 7u;
                        event = true;
                    }
                }
            }
        }
    } else {
        const uint32_t latch = GP4_GLOBAL(uint8_t, 0x0082fd6b) & 2u;
        flags = gp4::flags::logic8(latch);
        if (latch != 0u) {
            GP4_GLOBAL(uint8_t, 0x0082fd6b) = 0u;
            gp4::call_regs(0x0053dd5f, *r);
            const bool same = (r->eflags & gp4::flags::CF) == 0u &&
                              r->edx == GP4_GLOBAL(uint32_t, 0x0082fd88);
            code = same ? 5u : 6u;
            event = true;
        } else {
            const uint32_t item = GP4_GLOBAL(uint32_t, 0x0082fd88);
            flags = gp4::flags::sub32(item, 0u);
            if (item == 0u) {
                gp4::call_regs(0x0053dd5f, *r);
                flags = r->eflags;
            }
        }
    }
    if (event) {
        r->edx = code;
        GP4_GLOBAL(uint32_t, 0x0082fd9c) = code;
        const uint32_t state = GP4_GLOBAL(uint8_t, 0x0082fd6a) | 2u;
        GP4_GLOBAL(uint8_t, 0x0082fd6a) = static_cast<uint8_t>(state);
        flags = gp4::flags::logic8(state);
        gp4::cdecl_<0x0054ef3e, void>();
        flags &= ~gp4::flags::CF;       // clc
    } else {
        flags |= gp4::flags::CF;        // stc
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0053c500, FUN_0053c500, "regs(eax:u32, ebx:u32, edx:u32, esi:u32) -> (eax, ebx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=menu_0084300a+state_0053d9a6+mouse_0053c500")
