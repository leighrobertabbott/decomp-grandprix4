#include <gp4/gp4.h>
#include <gp4/flags.h>

// Event dispatcher for the viewed car. Does nothing unless the master byte 0x66d7a4 is zero and the
// viewed car [0x679d30] is the player's car [0x679d2c], and the car is not in the (+0x8d bit 0x10,
// +0x84 == 7) state. Then [0x88ff30] is the event number: the byte 0x865fd4[event] is cleared, and
// when the event's mask (the second dword of the 8-byte table entry at 0x8172a8) shares a bit with
// [0x88ff34] the byte is set to 1 and the entry's handler runs with edx = event, ebp = mask,
// esi = car (and [0x88fe12] = 0xff around the call); the handler starts with the flags of the mask
// test. pushal/popal keep every register; the flags are those of the last compare/test (or
// whatever the handler leaves).

// FUNCTION: GP4 0x00514152
void __cdecl FUN_00514152(gp4::Regs* r) {
    const uint8_t master = GP4_GLOBAL(uint8_t, 0x0066d7a4);
    uint32_t flags = gp4::flags::sub8(master, 0u);
    if (master == 0u) {
        const uint32_t car = GP4_GLOBAL(uint32_t, 0x00679d30);
        const uint32_t player = GP4_GLOBAL(uint32_t, 0x00679d2c);
        flags = gp4::flags::sub32(car, player);
        if (car == player) {
            const uint8_t carFlags = GP4_FIELD(uint8_t, car, 0x8d);
            flags = gp4::flags::logic8(carFlags & 0x10u);
            bool skip = false;
            if ((carFlags & 0x10u) != 0u) {
                const uint8_t state = GP4_FIELD(uint8_t, car, 0x84);
                flags = gp4::flags::sub8(state, 7u);
                skip = state == 7u;
            }
            if (!skip) {
                const uint32_t mask = GP4_GLOBAL(uint32_t, 0x0088ff34);
                const uint32_t event = GP4_GLOBAL(uint32_t, 0x0088ff30);
                uint8_t* const seen = GP4_ARRAY(uint8_t, 0x00865fd4) + event;
                const uint32_t* const entry = GP4_ARRAY(uint32_t, 0x008172a8) + event * 2u;
                *seen = 0u;
                const uint32_t hit = entry[1] & mask;
                flags = gp4::flags::logic32(hit);
                if (hit != 0u) {
                    *seen = 1u;
                    GP4_GLOBAL(uint8_t, 0x0088fe12) = 0xffu;
                    gp4::Regs c = *r;
                    c.edx = event;
                    c.ebp = mask;
                    c.esi = car;
                    c.eflags = gp4::flags::merge(r->eflags, flags);
                    gp4::call_regs(entry[0], c);
                    GP4_GLOBAL(uint8_t, 0x0088fe12) = 0u;
                    flags = c.eflags;
                }
            }
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00514152, FUN_00514152, "regs(eax:u32, ebx:u32, ecx:u32, edi:u32) -> (cf, pf, af, zf, sf, of); globals=layout=player_0040f8d9+disp_005141b8+events_00514152")
