#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `left + right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of the 32-bit `left - right` (sub / cmp).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of a 32-bit `or r, r` / logic op (CF = OF = AF = 0).
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag8(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

struct Unknown_004264c4_Source {
    uint8_t Unknown00[0x66];
    int16_t Unknown66;
    uint8_t Unknown68[0x9a - 0x68];
    int16_t Unknown9A;
    uint8_t Unknown9C[0xda - 0x9c];
    int16_t UnknownDA;
};

// FUNCTION: GP4 0x004264c4
// esi = source record, ebx = car record pointer inside the 0x160-byte record ring.
// Computes a signed Q14 position delta from the source words (+0x9a, +0xda/2, +0x66) and the
// globals; edx returns the delta modulo 0x4000 (always 0..0x3fff for a negative delta it is the
// complement), and ebx is advanced (delta >= 0) or moved back (delta < 0) by (delta >> 14) records
// of 0x160 bytes with wrap-around between the two ring bounds. eax and ecx are preserved; the
// flags are those of the last flag writer (or / cmp / add / sub).
void __cdecl FUN_004264c4(gp4::Regs* r) {
    const Unknown_004264c4_Source* source = reinterpret_cast<const Unknown_004264c4_Source*>(static_cast<uintptr_t>(r->esi));
    uint32_t ebx = r->ebx;

    const int32_t halfWord = static_cast<int32_t>(source->UnknownDA) >> 1;
    const int32_t sum = static_cast<int32_t>(source->Unknown9A) + halfWord;
    const int64_t product = static_cast<int64_t>(sum) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603854));
    uint32_t value = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 12);
    value += static_cast<uint32_t>(static_cast<int32_t>(source->Unknown66));
    value += GP4_GLOBAL(uint32_t, 0x00627af0);
    value -= GP4_GLOBAL(uint32_t, 0x00627af4);

    int32_t limit = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00627aec) - GP4_GLOBAL(uint32_t, 0x00627af8));
    if (limit < 0) {
        limit = 0;
    }
    value -= static_cast<uint32_t>(limit);

    uint32_t records;
    uint32_t remainder;
    uint32_t flags;
    if (static_cast<int32_t>(value) >= 0) {
        records = value >> 14;
        remainder = value & 0x3fffu;
        flags = logicFlags32(records);
        if (records != 0u) {
            records *= 0x160u;
            if (ebx >= GP4_GLOBAL(uint32_t, 0x00891500)) {
                ebx += records;
                flags = subFlags32(ebx, GP4_GLOBAL(uint32_t, 0x00866ef8));
                if (ebx >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
                    ebx = GP4_GLOBAL(uint32_t, 0x00866ef8);
                    flags = subFlags32(ebx, 0x160u);
                    ebx -= 0x160u;
                }
            } else {
                ebx += records;
                flags = subFlags32(ebx, GP4_GLOBAL(uint32_t, 0x00866ecc));
                if (ebx >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
                    const uint32_t wrap = GP4_GLOBAL(uint32_t, 0x00866ec4);
                    flags = subFlags32(ebx, wrap);
                    ebx -= wrap;
                }
            }
        }
    } else {
        value = 0u - value;
        records = static_cast<uint32_t>(static_cast<int32_t>(value) >> 14);
        remainder = value & 0x3fffu;
        if (remainder != 0u) {
            records += 1u;
            remainder = (0u - remainder) + 0x4000u;
        }
        flags = logicFlags32(records);
        if (records != 0u) {
            records *= 0x160u;
            if (ebx >= GP4_GLOBAL(uint32_t, 0x00891500)) {
                flags = subFlags32(ebx - records, GP4_GLOBAL(uint32_t, 0x00891500));
                ebx -= records;
                if (ebx < GP4_GLOBAL(uint32_t, 0x00891500)) {
                    ebx = GP4_GLOBAL(uint32_t, 0x00891500);
                }
            } else {
                flags = subFlags32(ebx - records, GP4_GLOBAL(uint32_t, 0x00891504));
                ebx -= records;
                if (ebx < GP4_GLOBAL(uint32_t, 0x00891504)) {
                    const uint32_t wrap = GP4_GLOBAL(uint32_t, 0x00866ec4);
                    flags = addFlags32(ebx, wrap);
                    ebx += wrap;
                }
            }
        }
    }

    r->ebx = ebx;
    r->edx = remainder;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004264c4, FUN_004264c4, "regs(ebx:u32, esi:ptr[0xe4]:bytes) -> (ebx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
