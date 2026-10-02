#include <gp4/gp4.h>

// FUNCTION: GP4 0x00404f33
void __cdecl FUN_00404f33(gp4::Regs* r) {
    uint8_t* source = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t scaledSource = static_cast<uint32_t>(GP4_FIELD(uint16_t, source, 0xb2)) << 8;
    const uint32_t record = GP4_GLOBAL(uint32_t, 0x00602d88);

    uint32_t flags;
    if (record == 0u) {
        GP4_GLOBAL(uint32_t, 0x006030c8) = scaledSource;
        r->eax = scaledSource;
        // Flags of "shl eax, 8" on a 16-bit value: CF = OF = AF = 0, SF = 0, PF from the zero low byte.
        flags = 4u | (scaledSource == 0u ? 0x40u : 0u);
    } else {
        uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(record));
        const int16_t difference = static_cast<int16_t>(
            static_cast<uint16_t>(GP4_FIELD(uint16_t, body, 0x162) - GP4_FIELD(uint16_t, body, -0x15e)));
        const int64_t first = static_cast<int64_t>(static_cast<int32_t>(difference))
            * GP4_GLOBAL(int32_t, 0x006f2bec);
        const uint32_t firstTerm = static_cast<uint32_t>(static_cast<uint64_t>(first) >> 14);
        const int64_t second = static_cast<int64_t>(static_cast<int32_t>(firstTerm))
            * GP4_GLOBAL(int32_t, 0x006030cc);
        int32_t factor = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(second) >> 15) + 0x4000u);
        if (factor < 0) {
            factor = 0;
        }
        const int64_t third = static_cast<int64_t>(factor) * static_cast<int32_t>(scaledSource);
        const uint64_t bits = static_cast<uint64_t>(third);
        const uint32_t result = static_cast<uint32_t>(bits >> 14);
        GP4_GLOBAL(uint32_t, 0x006030c8) = result;
        r->eax = result;
        r->edx = static_cast<uint32_t>(bits >> 32);
        r->edi = record;
        // Flags come from the final "shrd eax, edx, 14" (pinned oracle model: CF = bit 13,
        // AF = 0, OF = bit31(prev ^ result)).
        const uint32_t previous = static_cast<uint32_t>(bits >> 13);
        uint32_t parity = result & 0xffu;
        parity ^= parity >> 4;
        parity ^= parity >> 2;
        parity ^= parity >> 1;
        flags = static_cast<uint32_t>((bits >> 13) & 1u)
            | ((parity & 1u) == 0u ? 4u : 0u)
            | (result == 0u ? 0x40u : 0u)
            | ((result >> 31) << 7)
            | (((previous ^ result) >> 31) << 11);
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00404f33, FUN_00404f33, "regs(esi:ptr[0xc0]:bytes) -> (eax, edx, edi, cf, pf, af, zf, sf, of); globals=layout=state_00404f33")
