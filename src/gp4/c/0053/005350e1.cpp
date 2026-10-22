#include <gp4/gp4.h>
#include <gp4/flags.h>

// Follows `count` next links (+0) from the static chain cell `head` (no null checks, as the original).
static inline uint32_t Walk(uint32_t head, uint32_t count) {
    uint32_t node = head;
    for (uint32_t i = 0u; i != count; ++i) {
        node = GP4_FIELD(uint32_t, node, 0);
    }
    return node;
}

// pushal/popal wrapper refreshing two network-session menu chains. 0x535239 advances the player
// index [0x7ab4b4]; the flag mask for the chain 0x847dfd ORs in the dword [0x829262 + 4k] of every
// player k whose byte [0xb1bb74 + k] is zero (all four when [0x7aafd3] is nonzero) plus 0x100 when
// [0x82915a] is zero, and 0x53e4db applies it. Node 9 of that chain gets max([0x82915a], 1) - 1 at
// +0x2a. Node 3 of the chain 0x8479ae gets 0x710 ([0x7aafd3] nonzero) or 0x70f at +0x12 (bit 0x40 of
// +0x1f set when it changes); the mask for that chain is 0x1fe0 for a nonzero [0x7aafd3], else built
// from [0x7ab4b4] (4: 0x804; 0/1: 0x200 and [0x7ab4b8] = 0; not 1: 0x1000) and [0x7ab1a0] == 0
// (0x400). Node 11 gets the session title ("GP3 2000 Network Session" when [0x7ab4b8] is nonzero,
// else "GP3 2000 2 Player Session") at +0xe with bit 0x40 of +0x1f, and node 12 the word mask of
// the zero player bytes at +0x30. Every register comes back; the flags are those of the final
// `cmp ecx, 4` (equal).
// FUNCTION: GP4 0x005350e1
void __cdecl FUN_005350e1(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00535239, c);

    const uint8_t* const players = GP4_ARRAY(uint8_t, 0x00b1bb74);
    const uint32_t* const playerMasks = GP4_ARRAY(uint32_t, 0x00829262);
    uint32_t mask = 0u;
    for (uint32_t k = 0u; k != 4u; ++k) {
        if (players[k] == 0u || GP4_GLOBAL(uint8_t, 0x007aafd3) != 0u) {
            mask |= playerMasks[k];
        }
    }
    const uint32_t sessions = GP4_GLOBAL(uint32_t, 0x0082915a);
    uint32_t arith = gp4::flags::sub32(sessions, 0u);
    if (sessions == 0u) {
        mask |= 0x100u;
        arith = gp4::flags::logic32(mask);
    }
    c.eax = mask;
    c.ecx = 4u;
    c.esi = 0x00847dfdu;
    c.eflags = gp4::flags::merge(c.eflags, arith);
    gp4::call_regs(0x0053e4db, c);

    const uint32_t ninth = Walk(0x00847dfdu, 9u);
    const uint32_t count = GP4_GLOBAL(uint32_t, 0x0082915a);
    const uint32_t last = (count < 1u ? 1u : count) - 1u;
    GP4_FIELD(uint32_t, ninth, 0x2a) = last;

    const uint32_t mode = (GP4_GLOBAL(uint8_t, 0x007aafd3) != 0u) ? 0x710u : 0x70fu;
    const uint32_t third = Walk(0x008479aeu, 3u);
    arith = gp4::flags::sub32(mode, GP4_FIELD(uint32_t, third, 0x12));
    if (mode != GP4_FIELD(uint32_t, third, 0x12)) {
        GP4_FIELD(uint32_t, third, 0x12) = mode;
        const uint8_t flags = static_cast<uint8_t>(GP4_FIELD(uint8_t, third, 0x1f) | 0x40u);
        GP4_FIELD(uint8_t, third, 0x1f) = flags;
        arith = gp4::flags::logic8(flags);
    }

    uint32_t menuMask = 0u;
    if (GP4_GLOBAL(uint8_t, 0x007aafd3) != 0u) {
        menuMask = 0x1fe0u;
        arith = gp4::flags::logic32(menuMask);
    } else {
        const uint32_t player = GP4_GLOBAL(uint32_t, 0x007ab4b4);
        if (player == 4u) {
            menuMask = 0x804u;
        } else if (player == 0u || player == 1u) {
            menuMask = 0x200u;
            GP4_GLOBAL(uint32_t, 0x007ab4b8) = 0u;
        }
        arith = gp4::flags::sub32(GP4_GLOBAL(uint32_t, 0x007ab4b4), 1u);
        if (GP4_GLOBAL(uint32_t, 0x007ab4b4) != 1u) {
            menuMask |= 0x1000u;
        }
        const uint8_t local = GP4_GLOBAL(uint8_t, 0x007ab1a0);
        arith = gp4::flags::sub8(local, 0u);
        if (local == 0u) {
            menuMask |= 0x400u;
            arith = gp4::flags::logic32(menuMask);
        }
    }
    c.eax = menuMask;
    c.ecx = 0u;
    c.edx = mode;
    c.esi = 0x008479aeu;
    c.eflags = gp4::flags::merge(c.eflags, arith);
    gp4::call_regs(0x0053e4db, c);

    const uint32_t eleventh = Walk(0x008479aeu, 11u);
    const char* title = GP4_ARRAY(const char, 0x007ab4f4);
    if (GP4_GLOBAL(uint32_t, 0x007ab4b8) != 0u) {
        title = GP4_ARRAY(const char, 0x007ab514);
    }
    GP4_FIELD(uint32_t, eleventh, 0x0e) = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(title));
    GP4_FIELD(uint8_t, eleventh, 0x1f) |= 0x40u;

    const uint32_t twelfth = GP4_FIELD(uint32_t, eleventh, 0);
    uint32_t present = 0u;
    uint32_t bit = 1u;
    for (uint32_t k = 0u; k != 4u; ++k) {
        if (players[k] == 0u) {
            present |= bit;
        }
        bit <<= 1;
    }
    GP4_FIELD(uint16_t, twelfth, 0x30) = static_cast<uint16_t>(present);

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(4u, 4u));
}
GP4_IMPL(0x005350e1, FUN_005350e1, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=idx_00535239+sel_0053aab9+mode_007aafd3")
