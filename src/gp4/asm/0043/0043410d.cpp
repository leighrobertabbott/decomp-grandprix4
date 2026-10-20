#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t szpFlags(uint32_t value) {
    return parityFlag(value)
        | (value == 0u ? 0x40u : 0u)
        | ((value & 0x80000000u) != 0u ? 0x80u : 0u);
}

// add left, right
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return szpFlags(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (((left ^ result) & (right ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (result < left ? 1u : 0u);
}

// cmp left, right
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return szpFlags(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

static const uint32_t kEqualFlags = 0x44u;   // ZF | PF of a compare that matched

// FUNCTION: GP4 0x0043410d
void __cdecl FUN_0043410d(gp4::Regs* r) {
    const uint32_t event = r->eax;
    const uint32_t car = r->esi;
    uint32_t eax = event;
    uint32_t edx = r->edx;
    uint32_t flags;

    uint32_t* const timers = &GP4_ARRAY(uint32_t, 0x0063b9e4)[car * 9u];
    const uint32_t now = GP4_GLOBAL(uint32_t, 0x0066d814);

    if (event == 0u) {
        eax = 0u;
        timers[0] = now + 5000u;
        flags = addFlags(now, 5000u);
    } else if (event == 2u) {
        timers[1] = now + 500u;
        timers[2] = now + 1000u;
        edx = car * 4u + GP4_GLOBAL(uint32_t, 0x0062ff68);
        uint8_t* const slot = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(edx));
        slot[0] = 1u;
        slot[1] = 1u;
        timers[3] = now + 0x6d6u;
        eax = 5u;
        timers[5] = now + 2000u;
        flags = addFlags(now, 2000u);
    } else if (event == 3u) {
        eax = 4u;
        timers[4] = now;
        flags = addFlags(now, 0u);
    } else if (event == 4u) {
        timers[1] = now + 500u;
        timers[2] = now + 1000u;
        uint8_t* slot = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(car * 4u + GP4_GLOBAL(uint32_t, 0x0062ff68)));
        slot[0] = 1u;
        slot[1] = 1u;
        timers[3] = now + 0x6d6u;
        eax = 4u;
        timers[4] = now + 6000u;
        const uint32_t offset = car * 4u;
        const uint32_t base = GP4_GLOBAL(uint32_t, 0x0062ff68);
        edx = offset + base;
        slot = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(edx));
        slot[2] = 0u;
        flags = addFlags(offset, base);
    } else if (event == 6u) {
        eax = 4u;
        const uint32_t expiry = timers[4];
        bool expired = true;
        if (expiry != 0u) {
            const uint32_t negated = 0u - expiry;
            flags = addFlags(negated, now);
            expired = ((negated + now) & 0x80000000u) != 0u;
        }
        if (expired) {
            const uint32_t offset = car * 4u;
            const uint32_t base = GP4_GLOBAL(uint32_t, 0x0062ff68);
            edx = offset + base;
            reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(edx))[2] = 0xffu;
            flags = addFlags(offset, base);
        }
    } else if (event == 7u) {
        edx = car * 4u + GP4_GLOBAL(uint32_t, 0x0062ff68);
        reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(edx))[2] = 0u;
        eax = 4u;
        const uint32_t expiry = timers[4];
        bool restart = true;
        if (expiry != 0u) {
            const uint32_t negated = 0u - expiry;
            flags = addFlags(negated, now);
            restart = ((negated + now) & 0x80000000u) == 0u;
        }
        if (restart) {
            timers[4] = now;
            flags = addFlags(now, 0u);
        }
    } else if (event == 8u) {
        eax = 6u;
        timers[6] = now + 0x271u;
        flags = addFlags(now, 0x271u);
    } else if (event == 5u) {
        flags = kEqualFlags;
    } else if (event == 1u) {
        eax = 0u;
        timers[0] = 0u;
        flags = kEqualFlags;
    } else if (event == 9u || event == 10u) {
        flags = kEqualFlags;
    } else if (event == 11u) {
        gp4::Regs regs = *r;
        regs.eax = car;
        regs.edx = 0x19u;
        regs.ecx = car * 0x24u;
        gp4::call_regs(0x004340b1, regs);
        edx = 0x19u;
        eax = regs.eax;
        flags = kEqualFlags;
        if ((eax & 0xffu) != 0xffu) {
            eax = 8u;
            timers[8] = now + 1000u;
            flags = addFlags(now, 1000u);
        }
    } else if (event == 12u) {
        flags = kEqualFlags;
    } else if (event == 13u) {
        eax = 7u;
        timers[7] = now + 1000u;
        flags = addFlags(now, 1000u);
    } else {
        flags = subFlags(event, 0xfu);
    }

    r->eax = eax;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043410d, FUN_0043410d, "regs(eax:u32[0..15], esi:u32[0..21], edx:u32) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=state_00433ff8")
