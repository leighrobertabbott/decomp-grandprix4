#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// OF/SF/ZF/AF/PF of `inc value` (CF is not touched by inc).
static inline uint32_t incFlags32(uint32_t before) {
    const uint32_t result = before + 1u;
    return byteParity(result)
        | (((before ^ 1u ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | (result == 0x80000000u ? 0x800u : 0u);
}

// Scans the [0x0066d8ec] slot flag bytes at 0x0066d8f4 for entries with bit 7 set: the first one without
// bit 6 goes to [0x0066d896] and the first one with bit 6 to [0x0066d87d] (both cleared beforehand), the
// counts end up in eax (bit 6 clear) and edi (bit 6 set). Then clears the words/dword 0x0066d894,
// 0x0066d8c8, 0x007c2e9c, 0x0066d898, 0x0066d83c and copies the word 0x007c2eb8 to 0x0066d89a. ebx ends one
// past the scanned bytes, ecx is 0 and dl holds the last byte stored to the two targets; the flags are those
// of the final `inc ebx` (CF stays clear from the `test`).
// FUNCTION: GP4 0x004422dc
void __cdecl FUN_004422dc(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x0066d87d) = 0;
    GP4_GLOBAL(uint8_t, 0x0066d896) = 0;

    uint32_t remaining = GP4_GLOBAL(uint32_t, 0x0066d8ec);
    uint32_t unmarked = 0;
    uint32_t marked = 0;
    uint32_t pointer = 0x0066d8f4u;
    uint32_t edx = r->edx;

    do {
        const uint8_t flags = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(pointer));
        if ((flags & 0x80u) != 0u) {
            if ((flags & 0x40u) != 0u) {
                if (marked == 0u) {
                    edx = (edx & 0xffffff00u) | flags;
                    GP4_GLOBAL(uint8_t, 0x0066d87d) = flags;
                }
                marked += 1u;
            } else {
                if (unmarked == 0u) {
                    edx = (edx & 0xffffff00u) | flags;
                    GP4_GLOBAL(uint8_t, 0x0066d896) = flags;
                }
                unmarked += 1u;
            }
        }
        pointer += 1u;
        remaining -= 1u;
    } while (remaining != 0u);

    GP4_GLOBAL(uint16_t, 0x0066d894) = 0;
    GP4_GLOBAL(uint16_t, 0x0066d8c8) = 0;
    GP4_GLOBAL(uint16_t, 0x007c2e9c) = 0;
    GP4_GLOBAL(uint16_t, 0x0066d898) = 0;
    GP4_GLOBAL(uint32_t, 0x0066d83c) = 0;
    GP4_GLOBAL(uint16_t, 0x0066d89a) = GP4_GLOBAL(uint16_t, 0x007c2eb8);

    r->eax = unmarked;
    r->edi = marked;
    r->ebx = pointer;
    r->ecx = 0u;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | incFlags32(pointer - 1u);
}
GP4_IMPL(0x004422dc, FUN_004422dc, "regs(edx:u32) -> (eax, ebx, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=order_00675958")
