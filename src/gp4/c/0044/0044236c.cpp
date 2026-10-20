#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 8-bit `cmp left, right`.
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    left &= 0xffu;
    right &= 0xffu;
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 0x01u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// Picks the first two slot flag bytes (of the [0x0066d8ec] bytes at 0x0066d8f4) that have bit 7 set into
// [0x0066d896] and [0x0066d87e] (both cleared first), swaps them unless the first equals the byte
// [0x007c2e78], then clears the words/dword 0x0066d894, 0x0066d8c8, 0x007c2e9c, 0x0066d898, 0x0066d83c and
// copies the word 0x007c2eb8 to 0x0066d89a. eax ends as the count of flagged slots with its low byte
// replaced by the first pick, ebx one past the scanned bytes, ecx and edi 0, dl the last byte moved
// through dl; the flags are those of `cmp al, [0x007c2e78]`.
// FUNCTION: GP4 0x0044236c
void __cdecl FUN_0044236c(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x0066d896) = 0;
    GP4_GLOBAL(uint8_t, 0x0066d87e) = 0;

    const uint32_t count = GP4_GLOBAL(uint32_t, 0x0066d8ec);
    uint32_t remaining = count;
    uint32_t flagged = 0;
    uint32_t edx = r->edx;
    const uint8_t* slot = GP4_ARRAY(const uint8_t, 0x0066d8f4);
    do {
        const uint8_t value = *slot;
        if ((value & 0x80u) != 0u) {
            if (flagged == 0u) {
                edx = (edx & 0xffffff00u) | value;
                GP4_GLOBAL(uint8_t, 0x0066d896) = value;
            } else if (flagged == 1u) {
                edx = (edx & 0xffffff00u) | value;
                GP4_GLOBAL(uint8_t, 0x0066d87e) = value;
            }
            flagged += 1u;
        }
        ++slot;
        --remaining;
    } while (remaining != 0u);

    const uint8_t first = GP4_GLOBAL(uint8_t, 0x0066d896);
    const uint8_t expected = GP4_GLOBAL(uint8_t, 0x007c2e78);
    if (first != expected) {
        const uint8_t second = GP4_GLOBAL(uint8_t, 0x0066d87e);
        edx = (edx & 0xffffff00u) | second;
        GP4_GLOBAL(uint8_t, 0x0066d896) = second;
        GP4_GLOBAL(uint8_t, 0x0066d87e) = first;
    }

    GP4_GLOBAL(uint16_t, 0x0066d894) = 0;
    GP4_GLOBAL(uint16_t, 0x0066d8c8) = 0;
    GP4_GLOBAL(uint16_t, 0x007c2e9c) = 0;
    GP4_GLOBAL(uint16_t, 0x0066d898) = 0;
    GP4_GLOBAL(uint32_t, 0x0066d83c) = 0;
    GP4_GLOBAL(uint16_t, 0x0066d89a) = GP4_GLOBAL(uint16_t, 0x007c2eb8);

    r->eax = (flagged & 0xffffff00u) | first;
    r->ebx = 0x0066d8f4u + count;
    r->ecx = 0u;
    r->edx = edx;
    r->edi = 0u;
    r->eflags = (r->eflags & ~0x8d5u) | cmpFlags8(first, expected);
}
GP4_IMPL(0x0044236c, FUN_0044236c, "regs(edx:u32) -> (eax, ebx, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=order_00675958")
