#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `cmp left, right` (also `sub left, right`).
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of the 8-bit `cmp left, 0`.
static inline uint32_t cmpZeroFlags8(uint32_t left) {
    left &= 0xffu;
    return byteParity(left) | (left == 0u ? 0x40u : 0u) | (left & 0x80u);
}

// Builds one record in the buffer at 0x00630238 (used length [0x00630230], byte limit 0x7c28): a clock
// stamp dword ([0x0066d814]) followed by one 0x44205d entry (index, mask) for every non-zero dword of the
// nine masks behind [0x0062ff64]. The record's length goes to its second dword and to its last dword, or,
// for a five-dword record (a single bare entry), the second dword gets the fourth and the stamp and third
// dwords get bit 31 and the record is shortened by two dwords. [0x00630234] is the "nothing written yet"
// marker (0xff until the first entry). When the buffer is full (used >= 0x7c28) the routine stops at once.
// FUNCTION: GP4 0x004420b2
void __cdecl FUN_004420b2(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x00630234) = 0xffu;
    const uint32_t masks = GP4_GLOBAL(uint32_t, 0x0062ff64);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    for (uint32_t index = 0; index != 9u; ++index) {
        const uint32_t mask = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(masks + index * 4u));
        if (mask == 0u) {
            continue;
        }
        if (GP4_GLOBAL(uint8_t, 0x00630234) != 0u) {
            const uint32_t used = GP4_GLOBAL(uint32_t, 0x00630230);
            if (static_cast<int32_t>(used) >= 0x7c28) {
                r->eax = c.eax;
                r->ebx = masks + index * 4u;
                r->ecx = index;
                r->edx = c.edx;
                r->esi = c.esi;
                r->edi = used;
                r->eflags = (r->eflags & ~0x8d5u) | cmpFlags32(used, 0x7c28u);
                return;
            }
            c.edi = used + 0x00630238u;
            c.esi = c.edi;
            const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
            c.eax = clock;
            *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(c.edi)) = clock;
            c.edi += 8u;
            GP4_GLOBAL(uint8_t, 0x00630234) = 0u;
        }
        c.eax = index;
        c.edx = mask;
        gp4::call_regs(0x0044205d, c);
    }

    r->ebx = masks + 9u * 4u;
    r->ecx = 9u;

    const uint32_t marker = GP4_GLOBAL(uint8_t, 0x00630234);
    if (marker != 0u) {
        r->eax = c.eax;
        r->edx = c.edx;
        r->esi = c.esi;
        r->edi = c.edi;
        r->eflags = (r->eflags & ~0x8d5u) | cmpZeroFlags8(marker);
        return;
    }

    const uint32_t end = c.edi + 4u;
    const uint32_t length = end - c.esi;
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.esi));
    uint32_t edx = c.edx;
    uint32_t top;
    if (length == 0x14u) {
        edx = *reinterpret_cast<uint32_t*>(record + 0xc);
        *reinterpret_cast<uint32_t*>(record + 4) = edx;
        *reinterpret_cast<uint32_t*>(record) |= 0x80000000u;
        *reinterpret_cast<uint32_t*>(record + 8) |= 0x80000000u;
        top = end - 8u;
    } else {
        *reinterpret_cast<uint32_t*>(record + 4) = length;
        *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(end - 4u)) = length;
        top = end;
    }
    const uint32_t used = top - 0x00630238u;
    GP4_GLOBAL(uint32_t, 0x00630230) = used;

    r->eax = used;
    r->edx = edx;
    r->esi = c.esi;
    r->edi = length;
    r->eflags = (r->eflags & ~0x8d5u) | cmpFlags32(top, 0x00630238u);
}
GP4_IMPL(0x004420b2, FUN_004420b2, "regs(eax:u32, edx:u32, esi:u32, edi:u32) -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_004420b2")
