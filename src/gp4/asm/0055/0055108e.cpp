#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0055108e
// Interpolates the position edi between two track segment records A (index ebx) and B (index
// ecx), both looked up in the pointer table 0x875940 (pointers into the 0x160-byte records at
// 0x8943a0) with the word table 0x871b72 as per-slot offsets. The blend factor (Q14) is
// q = ((edi - A) << 14) / (B - A), where B is moved on by the wrap [0x866ec4] when it is the
// first record and a zero difference becomes 0x160; q is stored in 0x874b04. The first word of
// the records gives 0x874afe = high word of ((wordB - wordA + offsetB - offsetA) * low word of
// q << 2, dx:ax shifted through shl/rcl) + offsetA + wordA, kept in dx; unless the word +0x78 of
// the record edi has any of the bits 0x2f0, the second and third words are blended the same way
// into 0x875e6c and 0x875e74: (((B - A) * q >> 14) + A) << 6. Same computation as 0x00550fc7
// with the second set of globals. Leaves esi = offsetA, ebx = A, ecx = B, ebp = the summed
// word/offset difference; the flags are those of the final `test` or `shl eax, 6`.
void __cdecl FUN_0055108e(gp4::Regs* r) {
    const uint32_t indexA = r->ebx;
    const uint32_t indexB = r->ecx;
    const uint32_t edi = r->edi;

    const int32_t offsetA = GP4_ARRAY(int16_t, 0x00871b72)[indexA];
    const int32_t offsetB = GP4_ARRAY(int16_t, 0x00871b72)[indexB];
    const uint32_t offsetDelta = static_cast<uint32_t>(offsetB - offsetA);

    const uint32_t ptrB = GP4_ARRAY(uint32_t, 0x00875940)[indexB];
    const uint32_t ptrA = GP4_ARRAY(uint32_t, 0x00875940)[indexA];
    const int16_t* recB = reinterpret_cast<const int16_t*>(static_cast<uintptr_t>(ptrB));
    const int16_t* recA = reinterpret_cast<const int16_t*>(static_cast<uintptr_t>(ptrA));

    const int32_t wordB = recB[0];
    const int32_t wordA = recA[0];
    const uint16_t wordDelta = static_cast<uint16_t>(wordB - wordA);
    const uint32_t ebp = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(wordDelta))) + offsetDelta;

    uint32_t span = ptrB;
    if (span == 0x008943a0u) {
        span += GP4_GLOBAL(uint32_t, 0x00866ec4);
    }
    span -= ptrA;
    if (span == 0u) {
        span = 0x160u;
    }
    const uint32_t dividend = (edi - ptrA) << 14;
    int32_t remainder = 0;
    const int32_t quotient = gp4::idiv64(static_cast<int64_t>(static_cast<int32_t>(dividend)),
                                         static_cast<int32_t>(span), &remainder);
    GP4_GLOBAL(uint32_t, 0x00874b04) = static_cast<uint32_t>(quotient);

    // imul word: dx:ax = bp * low word of q, then shifted left by two through dx:ax.
    const int32_t product16 = static_cast<int32_t>(static_cast<int16_t>(ebp & 0xffffu)) *
                              static_cast<int32_t>(static_cast<int16_t>(static_cast<uint32_t>(quotient) & 0xffffu));
    const uint32_t shifted = static_cast<uint32_t>(product16) << 2;
    uint32_t eax = (static_cast<uint32_t>(quotient) & 0xffff0000u) | (shifted & 0xffffu);
    uint32_t edx = (static_cast<uint32_t>(remainder) & 0xffff0000u) | (shifted >> 16);
    edx += static_cast<uint32_t>(offsetA);
    const uint16_t low = static_cast<uint16_t>(edx + static_cast<uint32_t>(wordA));
    edx = (edx & 0xffff0000u) | low;
    GP4_GLOBAL(uint16_t, 0x00874afe) = low;

    const uint16_t flagWord = *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(edi + 0x78u));
    uint32_t arith = gp4::flags::logic16(flagWord & 0x2f0u);
    if ((flagWord & 0x2f0u) == 0u) {
        const uint16_t d1 = static_cast<uint16_t>(recB[2] - recA[2]);
        const int64_t p1 = static_cast<int64_t>(static_cast<int32_t>(static_cast<int16_t>(d1))) *
                           static_cast<int64_t>(quotient);
        eax = static_cast<uint32_t>(static_cast<uint64_t>(p1) >> 14);
        edx = static_cast<uint32_t>(static_cast<int32_t>(recA[2]));
        eax += edx;
        eax <<= 6;
        GP4_GLOBAL(uint32_t, 0x00875e6c) = eax;

        const uint16_t d2 = static_cast<uint16_t>(recB[4] - recA[4]);
        const int64_t p2 = static_cast<int64_t>(static_cast<int32_t>(static_cast<int16_t>(d2))) *
                           static_cast<int64_t>(quotient);
        const uint32_t before = static_cast<uint32_t>(static_cast<uint64_t>(p2) >> 14) +
                                static_cast<uint32_t>(static_cast<int32_t>(recA[4]));
        edx = static_cast<uint32_t>(static_cast<int32_t>(recA[4]));
        eax = before << 6;
        GP4_GLOBAL(uint32_t, 0x00875e74) = eax;
        arith = gp4::flags::logic32(eax);
        if (((before >> 26) & 1u) != 0u) {
            arith |= gp4::flags::CF;
        }
        if ((((before << 5) ^ (before << 6)) >> 31) != 0u) {
            arith |= gp4::flags::OF;
        }
    }

    r->eax = eax;
    r->ebx = ptrA;
    r->ecx = ptrB;
    r->edx = edx;
    r->esi = static_cast<uint32_t>(offsetA);
    r->ebp = ebp;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0055108e, FUN_0055108e, "regs(ebx:u32[0..7], ecx:u32[0..7], edi:rec[0x8943a0,0x160,0..7]) -> (eax, ebx, ecx, edx, esi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=geom_008943a0+span_00551227")
