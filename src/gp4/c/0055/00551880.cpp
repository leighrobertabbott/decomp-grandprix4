#include <gp4/gp4.h>
#include <gp4/flags.h>

// Byte offset into the Q14 cosine word table 0x007c5c5c for a 16-bit angle: |a| >> 2, even.
static inline uint32_t tableOffset(uint16_t angle) {
    uint16_t a = angle;
    if ((a & 0x8000u) != 0u) {
        a = static_cast<uint16_t>(0u - a);
    }
    return (static_cast<uint32_t>(a) >> 2) & 0xfffeu;
}

static inline int32_t tableWord(uint32_t offset) {
    const uint8_t* table = reinterpret_cast<const uint8_t*>(GP4_ARRAY(int16_t, 0x007c5c5c));
    return *reinterpret_cast<const int16_t*>(table + offset);
}

// FUNCTION: GP4 0x00551880
// For the three 16-bit angles at 0x007a8de0/2/4 stores the Q14 table value of the angle and of
// 0x4000 - angle (cosine and sine) as dwords: 0x008755ac/0x008755a8, 0x008755b4/0x008755b0,
// 0x008755bc/0x008755b8. Leaves eax/edx as the last pair (sign-extended) and the flags of the final
// `and edx, 0xfffe`.
void __cdecl FUN_00551880(gp4::Regs* r) {
    const uint16_t a0 = GP4_GLOBAL(uint16_t, 0x007a8de0);
    GP4_GLOBAL(int32_t, 0x008755ac) = tableWord(tableOffset(a0));
    GP4_GLOBAL(int32_t, 0x008755a8) = tableWord(tableOffset(static_cast<uint16_t>(0x4000u - a0)));

    const uint16_t a1 = GP4_GLOBAL(uint16_t, 0x007a8de2);
    GP4_GLOBAL(int32_t, 0x008755b4) = tableWord(tableOffset(a1));
    GP4_GLOBAL(int32_t, 0x008755b0) = tableWord(tableOffset(static_cast<uint16_t>(0x4000u - a1)));

    const uint16_t a2 = GP4_GLOBAL(uint16_t, 0x007a8de4);
    const int32_t first = tableWord(tableOffset(a2));
    GP4_GLOBAL(int32_t, 0x008755bc) = first;
    const uint32_t offset = tableOffset(static_cast<uint16_t>(0x4000u - a2));
    const int32_t second = tableWord(offset);
    GP4_GLOBAL(int32_t, 0x008755b8) = second;

    r->eax = static_cast<uint32_t>(first);
    r->edx = static_cast<uint32_t>(second);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(offset));
}
GP4_IMPL(0x00551880, FUN_00551880, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
