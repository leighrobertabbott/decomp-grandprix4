#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Reads one stream value: a sign-extended word, or a 32-bit value (word high, word low) when
// the 0x4000 flag is set, then shifted left by 3 unless the 0x1000 flag is set.
static inline int32_t readValue(const uint16_t*& p, uint32_t header) {
    int32_t value = static_cast<int16_t>(*p++);
    if ((header & 0x4000u) != 0u) {
        value = static_cast<int32_t>((static_cast<uint32_t>(value) << 16) | *p++);
    }
    if ((header & 0x1000u) == 0u) {
        value = static_cast<int32_t>(static_cast<uint32_t>(value) << 3);
    }
    return value;
}

// Decodes one record of a packed stream at esi: a header word (low 11 bits = step count at
// 0x0062ac48, flags 0x8000 / 0x4000 / 0x2000 / 0x1000), an optional pair of words (0x0062ac5c/5e),
// a word that becomes 0x0062ac58 (scaled by 4) or 0x0062ac5a, a start value (0x0062ac64) and an
// optional end value giving the per-step delta (0x0062ac68). CF = 1 and nothing else decoded
// when the header is zero. esi advances past the record; ebp's low word holds the header.
// FUNCTION: GP4 0x0040aadb
void __cdecl FUN_0040aadb(gp4::Regs* r) {
    const uint16_t* p = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t header = *p++;
    const uint32_t count = header & 0x7ffu;
    GP4_GLOBAL(uint32_t, 0x0062ac48) = count;
    r->ebp = (r->ebp & 0xffff0000u) | header;

    if (header == 0u) {
        // last flag writer "or bp, bp" (zero); stc
        r->eax = count;
        r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p));
        r->eflags = (r->eflags & ~0x8d5u) | 0x45u;
        return;
    }

    if ((header & 0x8000u) != 0u) {
        GP4_GLOBAL(uint16_t, 0x0062ac5c) = p[0];
        GP4_GLOBAL(uint16_t, 0x0062ac5e) = p[1];
        p += 2;
    }
    const uint16_t low = *p++;
    GP4_GLOBAL(int32_t, 0x0062ac68) = 0;

    const int32_t start = readValue(p, header);
    GP4_GLOBAL(int32_t, 0x0062ac64) = start;

    uint32_t eax = static_cast<uint32_t>(start);
    uint32_t edx = r->edx;
    if ((header & 0x2000u) != 0u) {
        const int32_t end = readValue(p, header);
        const int32_t diff = static_cast<int32_t>(static_cast<uint32_t>(end) -
                                                  static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x0062ac64)));
        int32_t rem;
        const int32_t delta = gp4::idiv64(static_cast<int64_t>(diff),
                                          static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0062ac48)), &rem);
        GP4_GLOBAL(int32_t, 0x0062ac68) = delta;
        eax = static_cast<uint32_t>(delta);
        edx = static_cast<uint32_t>(rem);
    }

    uint32_t flags;
    if (GP4_GLOBAL(int32_t, 0x0062ac64) == 0) {
        // last flag writer: test [0x62ac64], -1 (zero); clc
        GP4_GLOBAL(uint16_t, 0x0062ac58) = 0;
        GP4_GLOBAL(uint16_t, 0x0062ac5a) = low;
        flags = 0x44u;
    } else {
        // last flag writer: shl cx, 2 (16-bit); clc leaves the others
        GP4_GLOBAL(uint16_t, 0x0062ac5a) = 0;
        const uint32_t shifted = (static_cast<uint32_t>(low) << 2) & 0xffffu;
        GP4_GLOBAL(uint16_t, 0x0062ac58) = static_cast<uint16_t>(shifted);
        flags = parityFlag(shifted)
            | (shifted == 0u ? 0x40u : 0u)
            | ((shifted >> 8) & 0x80u)
            | ((((static_cast<uint32_t>(low) >> 14) ^ (static_cast<uint32_t>(low) >> 13)) & 1u) << 11);
    }

    r->eax = eax;
    r->edx = edx;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p));
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040aadb, FUN_0040aadb, "regs(esi:ptr[0x40]:layout=stream_0040aadb, ebp:u32, edx:u32) -> (eax, edx, esi, ebp, cf, pf, af, zf, sf, of)")
