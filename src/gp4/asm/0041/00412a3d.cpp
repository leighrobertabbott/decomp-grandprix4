#include <gp4/gp4.h>

// Flags of an 8-bit and/or/test result (CF = OF = AF = 0).
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    uint32_t parity = result ^ (result >> 4);
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return ((parity & 1u) == 0u ? 4u : 0u) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Clears `mask` in the destination byte, then sets it back when the source byte has it.
// Returns the flags of the last flag-writing instruction (and / test / or).
static inline uint32_t copyMaskedBit(uint8_t* dst, const uint8_t* src, uint32_t mask) {
    uint32_t value = static_cast<uint32_t>(*dst) & ~mask;
    *dst = static_cast<uint8_t>(value);
    uint32_t flags = logicFlags8(value);
    const uint32_t probe = static_cast<uint32_t>(*src) & mask;
    flags = logicFlags8(probe);
    if (probe != 0u) {
        value = static_cast<uint32_t>(*dst) | mask;
        *dst = static_cast<uint8_t>(value);
        flags = logicFlags8(value);
    }
    return flags;
}

// FUNCTION: GP4 0x00412a3d
// Copies selected flag bits and the 8 leading bytes from the record at esi into the record at ebx
// (flag bits 0x9c:2, 0xcb:0x10, 0xfe:0x40/0x10, 0x163:0x80/0x40, 0x122:0x80; byte 0xfd; dwords 0, 4).
// al receives the byte at [esi+0xfd]; the flags are those of the last and/or/test executed.
void __cdecl FUN_00412a3d(gp4::Regs* r) {
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint8_t* src = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t flags;

    flags = copyMaskedBit(dst + 0x9c, src + 0x9c, 0x02u);
    flags = copyMaskedBit(dst + 0xcb, src + 0xcb, 0x10u);

    uint32_t value = static_cast<uint32_t>(dst[0xfe]) & 0xbfu;
    dst[0xfe] = static_cast<uint8_t>(value);
    flags = logicFlags8(value);
    uint32_t probe = static_cast<uint32_t>(src[0xfe]) & 0x40u;
    flags = logicFlags8(probe);
    if (probe != 0u) {
        value = static_cast<uint32_t>(dst[0xfe]) | 0x40u;
        dst[0xfe] = static_cast<uint8_t>(value);
        value &= 0xefu;
        dst[0xfe] = static_cast<uint8_t>(value);
        flags = logicFlags8(value);
        probe = static_cast<uint32_t>(src[0xfe]) & 0x10u;
        flags = logicFlags8(probe);
        if (probe != 0u) {
            value = static_cast<uint32_t>(dst[0xfe]) | 0x10u;
            dst[0xfe] = static_cast<uint8_t>(value);
            flags = logicFlags8(value);
        }
    }

    value = static_cast<uint32_t>(dst[0x163]) & 0x7fu;
    dst[0x163] = static_cast<uint8_t>(value);
    value = static_cast<uint32_t>(dst[0x122]) & 0x7fu;
    dst[0x122] = static_cast<uint8_t>(value);
    flags = logicFlags8(value);
    probe = static_cast<uint32_t>(src[0x163]) & 0x80u;
    flags = logicFlags8(probe);
    if (probe != 0u) {
        value = static_cast<uint32_t>(dst[0x163]) | 0x80u;
        dst[0x163] = static_cast<uint8_t>(value);
        flags = logicFlags8(value);
        probe = static_cast<uint32_t>(src[0x122]) & 0x80u;
        flags = logicFlags8(probe);
        if (probe != 0u) {
            value = static_cast<uint32_t>(dst[0x122]) | 0x80u;
            dst[0x122] = static_cast<uint8_t>(value);
            flags = logicFlags8(value);
        }
        value = static_cast<uint32_t>(dst[0x163]) & 0xbfu;
        dst[0x163] = static_cast<uint8_t>(value);
        flags = logicFlags8(value);
        probe = static_cast<uint32_t>(src[0x163]) & 0x40u;
        flags = logicFlags8(probe);
        if (probe != 0u) {
            value = static_cast<uint32_t>(dst[0x163]) | 0x40u;
            dst[0x163] = static_cast<uint8_t>(value);
            flags = logicFlags8(value);
        }
    }

    const uint8_t carried = src[0xfd];
    dst[0xfd] = carried;
    GP4_FIELD(uint32_t, dst, 0) = GP4_FIELD(uint32_t, src, 0);
    GP4_FIELD(uint32_t, dst, 4) = GP4_FIELD(uint32_t, src, 4);

    r->eax = (r->eax & 0xffffff00u) | carried;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00412a3d, FUN_00412a3d, "regs(eax:u32, ebx:ptr[0x16b]:bytes, esi:ptr[0x16b]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
