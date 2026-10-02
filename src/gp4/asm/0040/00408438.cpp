#include <gp4/gp4.h>

// FUNCTION: GP4 0x00408438
void __cdecl FUN_00408438(gp4::Regs* r) {
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t flags;
    uint32_t result;

    if ((record[0xe5] & 1u) == 0u) {
        // test byte [esi+0xe5], 1 gave zero: ZF and PF set, CF/SF/OF/AF clear
        flags = 0x44u;
        result = 0u;
    } else {
        int32_t a = static_cast<int32_t>(*reinterpret_cast<const int16_t*>(record + 0x16));
        if (a < 0) a = -a;
        int32_t b = static_cast<int32_t>(*reinterpret_cast<const int16_t*>(record + 0x12));
        if (b < 0) b = -b;
        if (a < b) a = b;

        // cmp eax, 0x1555 decides the result and leaves the final flags
        const uint32_t lhs = static_cast<uint32_t>(a);
        const uint32_t rhs = 0x1555u;
        const uint32_t diff = lhs - rhs;
        uint32_t f = 0u;
        if (lhs < rhs) f |= 0x1u;                                   // CF
        uint32_t low = diff & 0xffu;
        low ^= low >> 4;
        low ^= low >> 2;
        low ^= low >> 1;
        if ((low & 1u) == 0u) f |= 0x4u;                            // PF
        if (((lhs ^ rhs ^ diff) & 0x10u) != 0u) f |= 0x10u;         // AF
        if (diff == 0u) f |= 0x40u;                                 // ZF
        if ((diff & 0x80000000u) != 0u) f |= 0x80u;                 // SF
        if ((((lhs ^ rhs) & (lhs ^ diff)) & 0x80000000u) != 0u) f |= 0x800u;  // OF
        flags = f;
        result = (a >= 0x1555) ? 0xffffffffu : 0u;
    }
    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00408438, FUN_00408438, "regs(esi:ptr[0xed]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
