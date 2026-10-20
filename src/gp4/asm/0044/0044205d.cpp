#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// FUNCTION: GP4 0x0044205d
// Writes one record into the buffer at edi: the mask dword (edx), the kind byte (eax) and, for kind 1 or 2, one
// byte per set mask bit (lowest bit first): byte +0 (kind 1) or byte +1 (kind 2) of entry `bit` of the table
// behind the pointer global 0x0062ff68. edi leaves rounded up to the next multiple of 4. The flags are those of
// the final `and edi, -4` (CF = OF = AF = 0). eax, edx and every other register are restored.
void __cdecl FUN_0044205d(gp4::Regs* r) {
    const uint32_t kind = r->eax;
    const uint32_t mask = r->edx;
    uint8_t* out = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    GP4_FIELD(uint32_t, out, 0) = mask;
    out += 4;
    *out = static_cast<uint8_t>(kind);
    out += 1;

    if (kind == 1u || kind == 2u) {
        const uint8_t* table = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0062ff68)));
        uint32_t rest = mask;
        uint32_t bit = 0;
        do {
            const uint32_t low = rest & 1u;
            rest >>= 1;
            if (low != 0u) {
                const uint8_t* entry = table + bit * 4u;
                *out = (kind == 1u) ? entry[0] : entry[1];
                out += 1;
            }
            bit += 1;
        } while (rest != 0u);
    }

    const uint32_t end = (static_cast<uint32_t>(reinterpret_cast<uintptr_t>(out)) + 3u) & 0xfffffffcu;
    const uint32_t flags = parityFlag(end) | (end == 0u ? 0x40u : 0u) | ((end & 0x80000000u) != 0u ? 0x80u : 0u);
    r->edi = end;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0044205d, FUN_0044205d, "regs(eax:u32[0..3], edx:u32[0..0x3fffff], edi:ptr[0x40]:bytes) -> (edi, cf, pf, af, zf, sf, of); globals=layout=state_00433ff8")
