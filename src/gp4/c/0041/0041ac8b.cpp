#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t widthMask(unsigned bits) {
    return bits >= 32u ? 0xffffffffu : ((1u << bits) - 1u);
}

// Flags of `cmp`/`sub`/`neg` (neg is 0 - value) at the given operand width.
static inline uint32_t subFlags(uint32_t left, uint32_t right, unsigned bits) {
    const uint32_t mask = widthMask(bits);
    const uint32_t sign = 1u << (bits - 1u);
    left &= mask;
    right &= mask;
    const uint32_t result = (left - right) & mask;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & sign) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & sign) != 0u ? 0x800u : 0u);
}

// Flags of `test`/`and`/`or` at the given operand width (CF = OF = AF = 0).
static inline uint32_t logicFlags(uint32_t result, unsigned bits) {
    result &= widthMask(bits);
    return parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & (1u << (bits - 1u))) != 0u ? 0x80u : 0u);
}

static inline void setFlags(gp4::Regs* r, uint32_t flags) {
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}

// Calls the original asm helper with the current registers and takes back eax, edx and the flags.
static inline void callHelper(gp4::Regs* r, uint32_t address) {
    gp4::Regs callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    gp4::call_regs(address, callee);
    r->eax = callee.eax;
    r->edx = callee.edx;
    r->ebx = callee.ebx;
    r->eflags = callee.eflags;
}

// After the lane change was considered: sets bit 4 of byte +0x6f ("wants to change") or moves the
// level at +0x72 by one step and clears it, depending on byte DAT_00622424. Both end with the
// flags of the last "or"/"and" on +0x6f.
// FUNCTION: GP4 0x0041ac8b
void __cdecl FUN_0041ac8b(gp4::Regs* r) {
    uint8_t* self = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t& flags6f = GP4_FIELD(uint8_t, self, 0x6f);
    int8_t& level = GP4_FIELD(int8_t, self, 0x72);

    bool direct = false;   // the "test [esi+0x6f], 1" / limit tests send us to 0x41ad4d
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x00622424);
    setFlags(r, logicFlags(mode, 8));
    bool skipGate = mode != 0u;
    if (!skipGate) {
        const uint8_t gate = GP4_FIELD(uint8_t, self, 0x8d) & 0x10u;
        setFlags(r, logicFlags(gate, 8));
        if (gate != 0u) {
            const uint8_t kind = GP4_FIELD(uint8_t, self, 0x84);
            setFlags(r, subFlags(kind, 7u, 8));
            if (kind == 7u) {
                return;
            }
        }
    }

    flags6f = static_cast<uint8_t>(flags6f & 0xefu);
    setFlags(r, logicFlags(flags6f, 8));
    const uint8_t blocked = GP4_FIELD(uint8_t, self, 0xe5) & 2u;
    setFlags(r, logicFlags(blocked, 8));
    if (blocked != 0u) {
        return;
    }

    const uint8_t tracking = flags6f & 1u;
    setFlags(r, logicFlags(tracking, 8));
    if (tracking == 0u) {
        direct = true;
    } else {
        r->eax = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, self, 0x24a)));
        setFlags(r, subFlags(r->eax, GP4_GLOBAL(uint32_t, 0x0060411c), 32));
        if (static_cast<int32_t>(r->eax) >= GP4_GLOBAL(int32_t, 0x0060411c)) {
            direct = true;
        }
    }

    if (!direct) {
        setFlags(r, subFlags(static_cast<uint8_t>(level), 1u, 8));
        if (level < 1) {
            flags6f = static_cast<uint8_t>(flags6f | 0x10u);
            setFlags(r, logicFlags(flags6f, 8));
            return;
        }

        r->eax = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, self, 0x62)));
        setFlags(r, subFlags(r->eax, GP4_GLOBAL(uint32_t, 0x0060131c), 32));
        if (static_cast<int32_t>(r->eax) < GP4_GLOBAL(int32_t, 0x0060131c)) {
            return;
        }

        const uint8_t overridden = GP4_FIELD(uint8_t, self, 0xe5) & 0x80u;
        setFlags(r, logicFlags(overridden, 8));
        if (overridden != 0u) {
            const uint8_t armed = GP4_GLOBAL(uint8_t, 0x00622424);
            setFlags(r, logicFlags(armed, 8));
            if (armed == 0u) {
                return;
            }
            callHelper(r, 0x0041afb1);
            if ((r->eflags & gp4::CF) != 0u) {
                return;
            }
        }

        // mov ax, [esi+0x9a]; test ax, ax; jns; neg ax
        uint32_t ax = GP4_FIELD(uint16_t, self, 0x9a);
        r->eax = (r->eax & 0xffff0000u) | ax;
        setFlags(r, logicFlags(ax, 16));
        if ((ax & 0x8000u) != 0u) {
            setFlags(r, subFlags(0u, ax, 16));
            ax = (0u - ax) & 0xffffu;
            r->eax = (r->eax & 0xffff0000u) | ax;
        }
        const uint32_t limit = GP4_GLOBAL(uint16_t, 0x0060383c);
        setFlags(r, subFlags(ax, limit, 16));
        if (static_cast<int16_t>(ax) < static_cast<int16_t>(limit)) {
            return;
        }

        const uint8_t current = static_cast<uint8_t>(level);
        r->edx = (r->edx & 0xffffff00u) | current;
        const uint8_t previous = GP4_FIELD(uint8_t, self, 0x1cd);
        setFlags(r, subFlags(current, previous, 8));
        if (current == previous) {
            return;
        }

        const uint8_t armed = GP4_GLOBAL(uint8_t, 0x00622424);
        setFlags(r, logicFlags(armed, 8));
        if (armed == 0u) {
            flags6f = static_cast<uint8_t>(flags6f | 0x10u);
            setFlags(r, logicFlags(flags6f, 8));
        } else {
            level = static_cast<int8_t>(level + 1);
            flags6f = static_cast<uint8_t>(flags6f & 0xefu);
            setFlags(r, logicFlags(flags6f, 8));
        }
        return;
    }

    // 0x41ad4d: look the level up in the table at 0x007c28bc (row = group of byte +0x7c).
    const uint32_t position = GP4_FIELD(uint16_t, self, 0x62);
    r->eax = (r->eax & 0xffff0000u) | position;
    uint32_t magnitude = static_cast<uint8_t>(level);
    r->edx = magnitude;
    setFlags(r, logicFlags(magnitude, 8));
    if ((magnitude & 0x80u) != 0u) {
        setFlags(r, subFlags(0u, magnitude, 8));
        magnitude = (0u - magnitude) & 0xffu;
        r->edx = magnitude;
    }
    int32_t group = static_cast<int32_t>(GP4_FIELD(uint8_t, self, 0x7c) & 0x3fu) - 1;
    if (group > 0x15) {
        group = 0x15;
    }
    r->ebx = static_cast<uint32_t>(group) << 4;
    const uint32_t entry = GP4_FIELD(uint16_t, GP4_ARRAY(uint8_t, 0x007c28bc), r->ebx + magnitude * 2u);
    setFlags(r, subFlags(position, entry, 16));
    if (static_cast<int16_t>(position) >= static_cast<int16_t>(entry)) {
        return;
    }

    const uint8_t changing = flags6f & 2u;
    setFlags(r, logicFlags(changing, 8));
    if (changing == 0u) {
        const uint8_t overridden = GP4_FIELD(uint8_t, self, 0xe5) & 0x80u;
        setFlags(r, logicFlags(overridden, 8));
        if (overridden != 0u) {
            return;
        }
    }
    setFlags(r, subFlags(static_cast<uint8_t>(level), 1u, 8));
    if (level <= 1) {
        return;
    }
    callHelper(r, 0x0041af84);
    if ((r->eflags & gp4::CF) != 0u) {
        return;
    }
    const uint8_t armed = GP4_GLOBAL(uint8_t, 0x00622424);
    setFlags(r, logicFlags(armed, 8));
    if (armed == 0u) {
        flags6f = static_cast<uint8_t>(flags6f | 0x10u);
        setFlags(r, logicFlags(flags6f, 8));
    } else {
        level = static_cast<int8_t>(level - 1);
        flags6f = static_cast<uint8_t>(flags6f & 0xefu);
        setFlags(r, logicFlags(flags6f, 8));
    }
}
GP4_IMPL(0x0041ac8b, FUN_0041ac8b, "regs(eax:u32, edx:u32, ebx:u32, esi:ptr[0x250]:bytes) -> (eax, edx, ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
