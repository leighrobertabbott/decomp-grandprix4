#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t low = value & 0xffu;
    low ^= low >> 4;
    low ^= low >> 2;
    low ^= low >> 1;
    return (low & 1u) == 0u ? 0x4u : 0u;
}

// FUNCTION: GP4 0x0043cb8f
void __cdecl FUN_0043cb8f(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    const uint32_t index = r->edx >> 1;

    // al = rec[1] + table[index]; clamp to signed 1..20 (cmp al,0x14 / jle, cmp al,1 / jge).
    int8_t value = static_cast<int8_t>(static_cast<uint8_t>(rec[1] + GP4_ARRAY(uint8_t, 0x0066d802)[index]));
    if (value > 0x14)
        value = 0x14;

    // popal does not restore flags: the last flag writer is `cmp al, 1` on the upper-clamped value.
    const uint32_t a = static_cast<uint8_t>(value);
    const uint32_t diff = (a - 1u) & 0xffu;
    const uint32_t flags = (a < 1u ? 0x1u : 0u)
        | parityFlag(diff)
        | (((a ^ 1u ^ diff) & 0x10u) != 0u ? 0x10u : 0u)
        | (diff == 0u ? 0x40u : 0u)
        | (diff & 0x80u)
        | ((((a ^ 1u) & (a ^ diff)) & 0x80u) != 0u ? 0x800u : 0u);

    if (value < 1)
        value = 1;
    rec[1] = static_cast<uint8_t>(value);

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043cb8f, FUN_0043cb8f, "regs(ebx:ptr[0x40]:bytes, edx:u32[0..21]) -> (cf, pf, af, zf, sf, of); globals=layout=adj_0066d802")
