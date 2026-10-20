#include <gp4/gp4.h>

// Track-segment record (0x160 bytes at 0x008943a0): only the fields this routine reads or writes.
struct Unknown_0043e8bc_Segment {
    uint8_t Unknown00[0x6b];
    uint8_t Level6B;
    uint8_t Unknown6C[4];
    uint8_t Flags70;
    uint8_t Unknown71[0xca - 0x71];
    int16_t UnknownCA;
    int16_t UnknownCC;
    int16_t UnknownCE;
};
static_assert(sizeof(Unknown_0043e8bc_Segment) == 0xd0, "layout");

static inline Unknown_0043e8bc_Segment* segmentAt(uint32_t address) {
    return reinterpret_cast<Unknown_0043e8bc_Segment*>(static_cast<uintptr_t>(address));
}

// Forward neighbour in the ring: +0x160; at or above the lap end [0x866ecc] it is wrapped back by
// [0x866ec4] when below the pit start [0x891500], kept below the pit end [0x866ef8], else the last
// record before the pit end.
static inline uint32_t nextSegment(uint32_t position) {
    position += 0x160u;
    if (position >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
        if (position < GP4_GLOBAL(uint32_t, 0x00891500)) {
            position -= GP4_GLOBAL(uint32_t, 0x00866ec4);
        } else if (position >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
            position = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
        }
    }
    return position;
}

// One of the level-walking routines 0x0043e768 / 0x0043e7bd / 0x0043e812 / 0x0043e867:
// al = level, edi = the segment.
static inline void callWalker(uintptr_t address, const gp4::Regs* entry, uint32_t level, uint32_t position, uint32_t neighbour) {
    gp4::Regs call = *entry;
    call.st_in = 0;
    call.st_out = 0;
    call.eax = level;
    call.ebx = neighbour;
    call.ecx = 0;
    call.edi = position;
    gp4::call_regs(address, call);
}

// FUNCTION: GP4 0x0043e8bc
// Banking level of every track segment between 0x008943a0 and the lap end [0x00866ecc], then smoothing.
// 1) Level +0x6b = min(6, |word +0xcc| clamped to 0x4000, * 6 >> 14); when the word +0xce is below
//    -0x1000 and the word +0xca is below 0x2800 the level may instead come from |+0xce| * 6 >> 14
//    (flag bit 0 of +0x70 set when that wins).
// 2) For each segment and its ring neighbour whose levels differ by more than 1, the lower one is raised
//    (0x0043e7bd walks forward, 0x0043e768 backward).
// 3) A segment and its neighbour that both have a level, no flag bit and word +0xcc of opposite sign: the
//    segment's level is cleared and 0x0043e812 / 0x0043e867 re-run from it.
// pushal/popal around everything; the flags are those of the final `cmp edi, [0x866ecc]` (equal).
void __cdecl FUN_0043e8bc(gp4::Regs* r) {
    uint32_t position = 0x008943a0u;
    do {
        Unknown_0043e8bc_Segment* segment = segmentAt(position);
        int32_t magnitude = segment->UnknownCC;
        if (magnitude < 0) {
            magnitude = -magnitude;
        }
        if (magnitude > 0x4000) {
            magnitude = 0x4000;
        }
        int32_t level = static_cast<int32_t>((static_cast<uint32_t>(magnitude) * 3u << 1) >> 14);
        int32_t bank = segment->UnknownCE;
        if (bank < -0x1000 && segment->UnknownCA < 0x2800) {
            bank = -bank;
            const int32_t scaled = static_cast<int32_t>((static_cast<uint32_t>(bank) * 3u << 1) >> 14);
            if (level < scaled) {
                level = scaled;
                segment->Flags70 |= 1;
            }
        }
        if (level > 6) {
            level = 6;
        }
        segment->Level6B = static_cast<uint8_t>(level);
        position += 0x160u;
    } while (position != GP4_GLOBAL(uint32_t, 0x00866ecc));

    position = 0x008943a0u;
    do {
        Unknown_0043e8bc_Segment* segment = segmentAt(position);
        const uint32_t neighbourAddress = nextSegment(position);
        Unknown_0043e8bc_Segment* neighbour = segmentAt(neighbourAddress);
        const uint8_t difference = static_cast<uint8_t>(segment->Level6B - neighbour->Level6B);
        if (difference != 0u) {
            int8_t magnitude = static_cast<int8_t>(difference);
            if (magnitude < 0) {
                magnitude = static_cast<int8_t>(-magnitude);
            }
            if (magnitude > 1) {
                const int8_t other = static_cast<int8_t>(neighbour->Level6B);
                if (other > static_cast<int8_t>(segment->Level6B)) {
                    callWalker(0x0043e768, r, static_cast<uint8_t>(other), position, neighbourAddress);
                } else {
                    callWalker(0x0043e7bd, r, segment->Level6B, position, neighbourAddress);
                }
            }
        }
        position += 0x160u;
    } while (position != GP4_GLOBAL(uint32_t, 0x00866ecc));

    position = 0x008943a0u;
    do {
        Unknown_0043e8bc_Segment* segment = segmentAt(position);
        if ((segment->Flags70 & 1u) == 0u && segment->Level6B != 0u) {
            uint16_t sign = static_cast<uint16_t>(segment->UnknownCC);
            const uint32_t neighbourAddress = nextSegment(position);
            Unknown_0043e8bc_Segment* neighbour = segmentAt(neighbourAddress);
            if ((neighbour->Flags70 & 1u) == 0u && neighbour->Level6B != 0u) {
                sign ^= static_cast<uint16_t>(neighbour->UnknownCC);
                if (static_cast<int16_t>(sign) < 0) {
                    segment->Level6B = 0;
                    callWalker(0x0043e812, r, 0, position, neighbourAddress);
                    callWalker(0x0043e867, r, 0, position, neighbourAddress);
                }
            }
        }
        position += 0x160u;
    } while (position != GP4_GLOBAL(uint32_t, 0x00866ecc));

    // cmp edi, [0x866ecc] with equal operands: PF | ZF
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0043e8bc, FUN_0043e8bc, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+segw_0043c608+segflag_00510b48+seg_0043e8bc")
