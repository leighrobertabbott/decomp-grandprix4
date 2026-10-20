#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) left by `cmp word, word`.
static inline uint32_t cmpFlags16(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x004317ae
void __cdecl FUN_004317ae(gp4::Regs* r) {
    const uint8_t* block = reinterpret_cast<const uint8_t*>(r->esi);
    uint32_t record = *reinterpret_cast<const uint32_t*>(block + 0x1c);
    uint32_t remaining = 0x14;
    uint32_t flags;
    bool found;

    for (;;) {
        // advance one 0x160-byte record, wrapping inside the record array
        record += 0x160;
        if (record >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
            if (record < GP4_GLOBAL(uint32_t, 0x00891500)) {
                record -= GP4_GLOBAL(uint32_t, 0x00866ec4);
            } else if (record >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
                record = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160;
            }
        }
        const uint16_t word = *reinterpret_cast<const uint16_t*>(record + 0x14);
        flags = cmpFlags16(word, 0xff00u);
        if (static_cast<int16_t>(word) < static_cast<int16_t>(0xff00)) {
            found = true;
            break;
        }
        if (--remaining == 0) {
            found = false;
            break;
        }
    }

    r->eax = record;
    r->ecx = remaining;
    flags = (flags & ~1u) | (found ? 1u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004317ae, FUN_004317ae, "regs(esi:u32[0x66fef0..0x66fef0]) -> (eax, ecx, cf, pf, af, zf, sf, of); globals=layout=ring_004317ae")
