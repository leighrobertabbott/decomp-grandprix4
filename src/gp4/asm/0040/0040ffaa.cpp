#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags left by a 32-bit TEST: CF = OF = AF = 0, PF/ZF/SF from the AND result.
static inline uint32_t testFlags32(uint32_t result) {
    return parityFlag8(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x0040ffaa
// edx = one of four globals picked by the 0x10/0x04/0x20/0x08 bits of [esi+0x40].
void __cdecl FUN_0040ffaa(gp4::Regs* r) {
    void* body = reinterpret_cast<void*>(static_cast<uintptr_t>(r->esi));
    const uint32_t mode = GP4_FIELD(uint32_t, body, 0x40);

    uint32_t value = GP4_GLOBAL(uint32_t, 0x00602ffc);
    uint32_t flags;
    if ((mode & 0x10u) != 0u) {
        flags = testFlags32(mode & 0x4u);
        if ((mode & 0x4u) != 0u) {
            value = GP4_GLOBAL(uint32_t, 0x00603000);
        }
    } else {
        flags = testFlags32(mode & 0x4u);
        if ((mode & 0x4u) == 0u) {
            value = GP4_GLOBAL(uint32_t, 0x00602ff4);
            flags = testFlags32(mode & 0x20u);
            if ((mode & 0x20u) != 0u) {
                flags = testFlags32(mode & 0x8u);
                if ((mode & 0x8u) != 0u) {
                    value = GP4_GLOBAL(uint32_t, 0x00602ff8);
                }
            }
        }
    }

    r->edx = value;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040ffaa, FUN_0040ffaa, "regs(esi:ptr[0x48]:bytes) -> (edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
