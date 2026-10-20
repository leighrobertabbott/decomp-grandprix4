#include <gp4/gp4.h>
#include <gp4/flags.h>

// Builds the shade tables of the palette builder 0x527592 for its two 32-entry colour maps.
// For i < 32: 0x948658[i] = 0x948958[0x948bc0[i]] and 0x948758[i] = 0x948958[0x948be0[i]] (a 256-byte remap
// table indexed by the colour byte). Each map then grows seven more 32-byte rows behind it (0x948678.. and
// 0x948778..): the source cursor keeps running from the map into the rows just written, every dword is
// masked with 0x1f1f1f1f and or-ed with the row's level 0x20, 0x40, ... 0xe0 repeated in all four bytes.
// pushal/popal keep every register; the flags are those of the final `dec ebp` of the second expansion
// (CF kept from the `add edx, 0x20202020` that wrapped the level to zero).

// Expands one map: seven rows of eight dwords; returns the flags of the last `dec ebp`.
static __declspec(noinline) uint32_t ExpandRows(const uint32_t* source, uint32_t* destination) {
    uint32_t level = 0x20202020u;
    uint32_t flags = 0u;
    for (uint32_t pass = 7u; pass != 0u; --pass) {
        for (uint32_t k = 8u; k != 0u; --k) {
            *destination++ = (*source++ & 0x1f1f1f1fu) | level;
        }
        flags = gp4::flags::add32(level, 0x20202020u);
        level += 0x20202020u;
        flags = gp4::flags::dec32(pass, flags);
    }
    return flags;
}

// FUNCTION: GP4 0x00527853
void __cdecl FUN_00527853(gp4::Regs* r) {
    const uint8_t* remap = GP4_ARRAY(const uint8_t, 0x00948958);
    const uint8_t* indexA = GP4_ARRAY(const uint8_t, 0x00948bc0);
    const uint8_t* indexB = GP4_ARRAY(const uint8_t, 0x00948be0);
    uint8_t* mapA = GP4_ARRAY(uint8_t, 0x00948658);
    uint8_t* mapB = GP4_ARRAY(uint8_t, 0x00948758);
    for (uint32_t i = 0u; i != 0x20u; ++i) {
        mapA[i] = remap[indexA[i]];
        mapB[i] = remap[indexB[i]];
    }

    ExpandRows(GP4_ARRAY(const uint32_t, 0x00948658), GP4_ARRAY(uint32_t, 0x00948678));
    const uint32_t flags = ExpandRows(GP4_ARRAY(const uint32_t, 0x00948758), GP4_ARRAY(uint32_t, 0x00948778));
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00527853, FUN_00527853, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
