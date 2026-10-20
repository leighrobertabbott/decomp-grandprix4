#include <gp4/gp4.h>
#include <gp4/flags.h>

// 0x20-byte record of the array at 0x822300 ([0x82fefb] entries). The caller treats +0 as a
// pointer to an object whose bytes +0x22..+0x25 receive the four ids found here (0xf9 = none).
struct Unknown_822300 {
    uint32_t Unknown00;
    uint32_t Unknown04;   // id copied into the results
    int32_t Unknown08;    // span A low
    int32_t Unknown0c;    // span B low
    int32_t Unknown10;    // span A high
    int32_t Unknown14;    // span B high
    int32_t Unknown18;    // position compared when span B overlaps
    int32_t Unknown1c;    // position compared when span A overlaps
};

// Overlap length of [lo, hi] (scanned record) with [refLo, refHi] (edi record), 0 when disjoint.
// `eax` receives the last value the original leaves in eax on that path.
static inline uint32_t overlapLength(int32_t lo, int32_t hi, int32_t refLo, int32_t refHi, uint32_t& eax) {
    if (refLo > lo) {
        eax = static_cast<uint32_t>(hi);
        if (refLo >= hi) {
            return 0u;
        }
        if (refHi <= hi) {
            eax = static_cast<uint32_t>(refHi) - static_cast<uint32_t>(refLo);
        } else {
            eax = static_cast<uint32_t>(hi) - static_cast<uint32_t>(refLo);
        }
        return eax;
    }
    eax = static_cast<uint32_t>(lo);
    if (refHi <= lo) {
        return 0u;
    }
    if (refHi >= hi) {
        eax = static_cast<uint32_t>(hi) - static_cast<uint32_t>(lo);
    } else {
        eax = static_cast<uint32_t>(refHi) - static_cast<uint32_t>(lo);
    }
    return eax;
}

// Keeps the nearest record ahead (delta > 0x10) and behind (delta < -0x10) along one axis;
// ties on distance prefer the larger overlap (unsigned). Globals per direction: id, distance, overlap.
static inline void considerAxis(uint32_t delta, uint32_t overlap, uint32_t id, uint32_t& eax,
                                uint32_t* aheadId, uint32_t* aheadDist, uint32_t* aheadOverlap,
                                uint32_t* behindId, uint32_t* behindDist, uint32_t* behindOverlap) {
    eax = delta;
    if (overlap == 0u) {
        return;
    }
    if (static_cast<int32_t>(delta) > 0x10) {
        if (static_cast<int32_t>(delta) < static_cast<int32_t>(*aheadDist)
            || (delta == *aheadDist && overlap > *aheadOverlap)) {
            *aheadDist = delta;
            *aheadOverlap = overlap;
            eax = id;
            *aheadId = id;
        }
    } else if (static_cast<int32_t>(delta) < -0x10) {
        const uint32_t distance = 0u - delta;
        eax = distance;
        if (static_cast<int32_t>(distance) < static_cast<int32_t>(*behindDist)
            || (distance == *behindDist && overlap > *behindOverlap)) {
            *behindDist = distance;
            *behindOverlap = overlap;
            eax = id;
            *behindId = id;
        }
    }
}

// FUNCTION: GP4 0x0053e6d0
// For the record at edi, scans the [0x82fefb] records at 0x822300 and finds the nearest
// overlapping neighbour on each side of both axes (initial distances 800 and 600).
void __cdecl FUN_0053e6d0(gp4::Regs* r) {
    const Unknown_822300* self = reinterpret_cast<const Unknown_822300*>(static_cast<uintptr_t>(r->edi));
    GP4_GLOBAL(uint32_t, 0x0082fecb) = 0u;
    GP4_GLOBAL(uint32_t, 0x0082fecf) = 800u;
    GP4_GLOBAL(uint32_t, 0x0082fed3) = 0u;
    GP4_GLOBAL(uint32_t, 0x0082fed7) = 0u;
    GP4_GLOBAL(uint32_t, 0x0082fedb) = 800u;
    GP4_GLOBAL(uint32_t, 0x0082fedf) = 0u;
    GP4_GLOBAL(uint32_t, 0x0082fee3) = 0u;
    GP4_GLOBAL(uint32_t, 0x0082fee7) = 600u;
    GP4_GLOBAL(uint32_t, 0x0082feeb) = 0u;
    GP4_GLOBAL(uint32_t, 0x0082feef) = 0u;
    GP4_GLOBAL(uint32_t, 0x0082fef3) = 600u;
    GP4_GLOBAL(uint32_t, 0x0082fef7) = 0u;

    uint32_t eax = r->eax;
    uint32_t ebx = r->ebx;
    const Unknown_822300* other = GP4_ARRAY(const Unknown_822300, 0x00822300);
    uint32_t index = 0u;
    do {
        if (other != self) {
            GP4_GLOBAL(uint32_t, 0x0082feff) = 0u;
            const uint32_t overlapA = overlapLength(other->Unknown08, other->Unknown10,
                                                    self->Unknown08, self->Unknown10, eax);
            GP4_GLOBAL(uint32_t, 0x0082feff) = overlapA;
            GP4_GLOBAL(uint32_t, 0x0082ff03) = 0u;
            const uint32_t overlapB = overlapLength(other->Unknown0c, other->Unknown14,
                                                    self->Unknown0c, self->Unknown14, eax);
            GP4_GLOBAL(uint32_t, 0x0082ff03) = overlapB;

            ebx = GP4_GLOBAL(uint32_t, 0x0082ff03);
            considerAxis(static_cast<uint32_t>(self->Unknown18) - static_cast<uint32_t>(other->Unknown18),
                         ebx, other->Unknown04, eax,
                         &GP4_GLOBAL(uint32_t, 0x0082fecb), &GP4_GLOBAL(uint32_t, 0x0082fecf),
                         &GP4_GLOBAL(uint32_t, 0x0082fed3), &GP4_GLOBAL(uint32_t, 0x0082fed7),
                         &GP4_GLOBAL(uint32_t, 0x0082fedb), &GP4_GLOBAL(uint32_t, 0x0082fedf));
            ebx = GP4_GLOBAL(uint32_t, 0x0082feff);
            considerAxis(static_cast<uint32_t>(self->Unknown1c) - static_cast<uint32_t>(other->Unknown1c),
                         ebx, other->Unknown04, eax,
                         &GP4_GLOBAL(uint32_t, 0x0082fee3), &GP4_GLOBAL(uint32_t, 0x0082fee7),
                         &GP4_GLOBAL(uint32_t, 0x0082feeb), &GP4_GLOBAL(uint32_t, 0x0082feef),
                         &GP4_GLOBAL(uint32_t, 0x0082fef3), &GP4_GLOBAL(uint32_t, 0x0082fef7));
        }
        ++other;
        ++index;
    } while (index != GP4_GLOBAL(uint32_t, 0x0082fefb));

    r->eax = eax;
    r->ebx = ebx;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(other));
    // Exit compare `cmp ecx, [0x82fefb]` with equal operands.
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(index, index));
}
GP4_IMPL(0x0053e6d0, FUN_0053e6d0, "regs(edi:rec[0x822300,0x20,0..15], eax:u32, ebx:u32) -> (eax, ebx, esi, cf, pf, af, zf, sf, of); globals=layout=nav_0053e6d0")
