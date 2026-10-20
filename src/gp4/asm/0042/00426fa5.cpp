#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of the 32-bit `left - right` (cmp).
static inline uint32_t cmpFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) >> 20);
}

// If the record at esi has bit 1 of [+0x8d] and bit 4 of [+0x9c] set and its link [+0x1c] is at or above the
// limit 0x00891500, the link's own pointer at +0x44 is run through FUN_0040460a with (esi[0], esi[4]) as the input
// vector and the word [esi+0x186] becomes the high 24 bits of the rotated x (0x934df0 >> 8); otherwise it
// copies the word [esi+0x90].
// FUNCTION: GP4 0x00426fa5
void __cdecl FUN_00426fa5(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t mask = 0x8d5u;

    uint32_t eax = (r->eax & 0xffff0000u) | GP4_FIELD(const uint16_t, rec, 0x90);
    uint32_t flags;

    if ((GP4_FIELD(const uint8_t, rec, 0x8d) & 2u) == 0u) {
        flags = 0x44u;  // TEST result 0: ZF, PF
    } else if ((GP4_FIELD(const uint8_t, rec, 0x9c) & 0x10u) == 0u) {
        flags = 0x44u;
    } else {
        const uint32_t link = GP4_FIELD(const uint32_t, rec, 0x1c);
        r->edi = link;
        const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00891500);
        flags = cmpFlags(link, limit);
        if (link >= limit) {
            gp4::Regs c = *r;
            c.st_in = 0;
            c.st_out = 0;
            c.edi = GP4_FIELD(const uint32_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(link)), 0x44);
            GP4_GLOBAL(uint32_t, 0x00623e30) = GP4_FIELD(const uint32_t, rec, 0);
            GP4_GLOBAL(uint32_t, 0x00623e38) = GP4_FIELD(const uint32_t, rec, 4);
            gp4::call_regs(0x0040460a, c);
            const uint32_t value = GP4_GLOBAL(uint32_t, 0x00934df0);
            eax = (uint32_t)((int32_t)value >> 8);
            flags = ((value >> 7) & 1u) | parityFlag(eax) | (eax == 0u ? 0x40u : 0u) | ((eax >> 24) & 0x80u);
            r->edx = c.edx;
        }
    }

    GP4_FIELD(uint16_t, rec, 0x186) = (uint16_t)eax;
    r->eax = eax;
    r->eflags = (r->eflags & ~mask) | flags;
}
GP4_IMPL(0x00426fa5, FUN_00426fa5, "regs(eax:u32, esi:ptr[0x18e]:layout=car_00426fa5) -> (eax, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=pit_00426fa5")
