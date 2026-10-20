#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit logic result (test / or): CF = OF = AF = 0.
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u);
}

// Arithmetic flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// The working car record at 0x00639204 (ebx): only the fields this routine reads.
struct Unknown_00639204 {
    uint8_t Unknown00[0x1c];
    uint32_t Unknown1C;      // track-segment record the working car stands on
    uint8_t Unknown20[0x66 - 0x20];
    int16_t Unknown66;       // position inside that segment (Q14 fraction)
};

static __declspec(noinline) const Unknown_00639204* workingRecord() {
    return GP4_ARRAY(Unknown_00639204, 0x00639204);
}

// FUNCTION: GP4 0x00412c29
// Re-places the car in esi (and its segment pointers +0x1c/+0x1b8) on the track when the pending
// advance [0x00601c28] is nonzero and the working record's segment [0x00639220] lies below the pit
// start [0x00891500]. eax/edx/ecx are saved in [0x00601c2c/30/34]. If the track distance
// (0x004062b1) from the working segment to edi is positive and clamped to 16 records plus the lag
// still reaches the advance, nothing moves and the saved eax/edx/ecx are returned. Otherwise the
// advance plus the working record's fraction (+0x66) is walked one 0x160-byte segment per 0x4000
// (with the lap wrap [0x00866ec4] / pit end [0x00866ef8] rules), the remainder is stored at +0x66 and
// the blend 0x004035c3 recomputes +0x90; eax/edx are the two words, ecx the saved ecx. ebx is
// restored; ebp = [0x00639220]; flags are those of the last compare or of 0x004035c3.
void __cdecl FUN_00412c29(gp4::Regs* r) {
    const uint32_t advance = GP4_GLOBAL(uint32_t, 0x00601c28);
    if (advance == 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | logicFlags32(advance);
        return;
    }

    const Unknown_00639204* work = workingRecord();
    uint32_t ebp = work->Unknown1C;
    const uint32_t pit = GP4_GLOBAL(uint32_t, 0x00891500);
    if (!(ebp < pit)) {
        r->ebp = ebp;
        r->eflags = (r->eflags & ~0x8d5u) | cmpFlags32(ebp, pit);
        return;
    }

    GP4_GLOBAL(uint32_t, 0x00601c2c) = r->eax;
    GP4_GLOBAL(uint32_t, 0x00601c30) = r->edx;
    GP4_GLOBAL(uint32_t, 0x00601c34) = r->ecx;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ebx = 0x00639204u;
    c.ebp = ebp;
    c.eax = r->edi;
    c.edx = ebp;
    gp4::call_regs(0x004062b1, c);

    uint32_t flags = logicFlags32(c.eax);
    const int32_t distance = static_cast<int32_t>(c.eax);
    if (distance > 0) {
        int32_t remainder;
        int32_t records = gp4::idiv64(static_cast<int64_t>(distance), 0x160, &remainder);
        if (records > 0x10) {
            records = 0x10;
        }
        uint32_t reach = static_cast<uint32_t>(records) << 14;
        reach += 0u - static_cast<uint32_t>(static_cast<int32_t>(work->Unknown66));
        reach += GP4_GLOBAL(uint32_t, 0x00601c30);
        flags = cmpFlags32(reach, GP4_GLOBAL(uint32_t, 0x00601c28));
        if (static_cast<int32_t>(reach) >= GP4_GLOBAL(int32_t, 0x00601c28)) {
            r->eax = GP4_GLOBAL(uint32_t, 0x00601c2c);
            r->edx = GP4_GLOBAL(uint32_t, 0x00601c30);
            r->ecx = GP4_GLOBAL(uint32_t, 0x00601c34);
            r->ebp = ebp;
            r->eflags = (r->eflags & ~0x8d5u) | flags;
            return;
        }
    }

    uint32_t edi = ebp;
    uint32_t fraction = static_cast<uint32_t>(static_cast<int32_t>(work->Unknown66)) + GP4_GLOBAL(uint32_t, 0x00601c28);
    while (static_cast<int32_t>(fraction) >= 0x4000) {
        edi += 0x160u;
        if (!(edi < GP4_GLOBAL(uint32_t, 0x00866ecc))) {
            if (edi < GP4_GLOBAL(uint32_t, 0x00891500)) {
                edi -= GP4_GLOBAL(uint32_t, 0x00866ec4);
            } else if (!(edi < GP4_GLOBAL(uint32_t, 0x00866ef8))) {
                edi = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
            }
        }
        fraction -= 0x4000u;
    }

    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    GP4_FIELD(uint16_t, car, 0x66) = static_cast<uint16_t>(fraction);
    GP4_FIELD(uint32_t, car, 0x1c) = edi;
    GP4_FIELD(uint32_t, car, 0x1b8) = edi;
    GP4_GLOBAL(uint8_t, 0x00601ba6) = 0xffu;

    gp4::Regs blend = *r;
    blend.st_in = 0;
    blend.st_out = 0;
    blend.ebx = 0x00639204u;
    blend.ebp = ebp;
    blend.edi = edi;
    blend.edx = fraction;
    blend.eax = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, car, 0x66)));
    gp4::call_regs(0x004035c3, blend);

    uint8_t* after = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(blend.esi));
    GP4_FIELD(uint16_t, after, 0x90) = static_cast<uint16_t>(blend.eax);
    GP4_GLOBAL(uint8_t, 0x00601ba6) = 0u;

    r->eax = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, after, 0x90)));
    r->edx = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, after, 0x66)));
    r->ecx = GP4_GLOBAL(uint32_t, 0x00601c34);
    r->esi = blend.esi;
    r->edi = blend.edi;
    r->ebp = blend.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (blend.eflags & 0x8d5u);
}
GP4_IMPL(0x00412c29, FUN_00412c29, "regs(eax:u32, ecx:u32, edx:u32, ebp:u32, esi:ptr[0x29c]:layout=car_00431ffb+car_004035c3, edi:rec[0x8943a0,0x160,0..17]) -> (eax, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+state_00412c29")
