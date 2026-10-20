#include <gp4/gp4.h>
#include <gp4/flags.h>

// Picks the display event bit for the viewed car. Nothing happens unless the master byte
// [0x0066d7a4] is zero and esi (a car record) is the viewed car [0x00679d2c]. Car flag byte +0x82
// and the mode byte [0x007c2ea9] (0x80, 0x44, sign, bit 6) select one event bit; when 0x0040822c
// sets CF it comes from the table 0x00893e80 indexed by [0x0066d8d8] instead. The bit goes to
// [0x0088ff34] and its index (`mov edx, -1; inc edx; shr eax, 1; jne`, 0 for a zero word) to
// [0x0088ff38]; eax ends 0 and edx = the index. Flags: the early-out `test`/`cmp`, else those of
// the final `shr eax, 1` (ZF/PF set, CF = the shifted-out bit, OF = AF = 0).
// FUNCTION: GP4 0x00514981
void __cdecl FUN_00514981(gp4::Regs* r) {
    const uint8_t master = GP4_GLOBAL(uint8_t, 0x0066d7a4);
    if (master != 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(master));
        return;
    }
    const uint32_t viewed = GP4_GLOBAL(uint32_t, 0x00679d2c);
    if (r->esi != viewed) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(r->esi, viewed));
        return;
    }

    const uint8_t* car = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t event;
    if ((car[0x82] & 0x40u) != 0u) {
        const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
        if (mode == 0x80u) {
            event = 0x2000u;
        } else if (GP4_GLOBAL(uint8_t, 0x007c2ea9) == 0x44u) {
            event = 0x40000u;
        } else {
            event = 0x10000u;
        }
    } else if ((car[0x82] & 0x20u) != 0u) {
        const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
        if (mode == 0x80u) {
            event = 0x4000u;
        } else if (GP4_GLOBAL(uint8_t, 0x007c2ea9) == 0x44u) {
            event = 0x80000u;
        } else {
            event = 0x20000u;
        }
    } else {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        gp4::call_regs(0x0040822c, c);
        if ((c.eflags & gp4::flags::CF) != 0u) {
            event = GP4_ARRAY(uint32_t, 0x00893e80)[GP4_GLOBAL(uint32_t, 0x0066d8d8)];
        } else if ((car[0x82] & 2u) != 0u && (car[0x82] & 8u) != 0u) {
            if (static_cast<int8_t>(GP4_GLOBAL(uint8_t, 0x007c2ea9)) < 0) {
                event = 2u;
            } else if (GP4_GLOBAL(uint8_t, 0x007c2ea9) == 0x44u) {
                event = 0x100000u;
            } else {
                event = 0x400u;
            }
        } else if ((car[0x82] & 2u) != 0u && (car[0x82] & 4u) != 0u) {
            event = 0x20u;
        } else if ((car[0x82] & 2u) != 0u && (car[0x82] & 0x80u) != 0u) {
            event = 0x1000u;
        } else {
            event = 2u;
            if (static_cast<int8_t>(GP4_GLOBAL(uint8_t, 0x007c2ea9)) >= 0) {
                event = 0x8000u;
                if ((GP4_GLOBAL(uint8_t, 0x007c2ea9) & 0x40u) == 0u) {
                    event = 1u;
                }
            }
        }
    }

    GP4_GLOBAL(uint32_t, 0x0088ff34) = event;
    uint32_t index = 0xffffffffu;
    uint32_t rest = event;
    do {
        ++index;
        rest >>= 1;
    } while (rest != 0u);
    GP4_GLOBAL(uint32_t, 0x0088ff38) = index;

    r->eax = 0u;
    r->edx = index;
    r->eflags = gp4::flags::merge(r->eflags,
        gp4::flags::ZF | gp4::flags::PF | (event != 0u ? gp4::flags::CF : 0u));
}
GP4_IMPL(0x00514981, FUN_00514981, "regs(esi:ptr[0xf0]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=player_0040f8d9+bits_005174cb+state_00432a8b+links_00521d6f")
