#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of the 16-bit CMP left, right.
static inline uint32_t cmpFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left - right);
    return (left < right ? 0x001u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x8000u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Flags of the 32-bit CMP left, right.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x001u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

static inline uint32_t ringStart() { return GP4_GLOBAL(uint32_t, 0x00891500); }

// Runs the per-segment routine 0x0041ed6b on the segment in ebx (it returns eax/ecx/edx and clobbers flags).
static inline void visitSegment(gp4::Regs& c) {
    gp4::call_regs(0x0041ed6b, c);
}

// Walks the 0x160-byte segment ring from the car's segment [0x603d24] towards the record in edi, at most
// ecx (4) steps, visiting each record with 0x41ed6b. Returns the flags of the last CMP executed.
static uint32_t walkRing(gp4::Regs& c) {
    uint32_t flags = cmpFlags32(c.edi, GP4_GLOBAL(uint32_t, 0x00603d24));
    if (c.edi == GP4_GLOBAL(uint32_t, 0x00603d24)) {
        return flags;
    }
    c.ebx = GP4_GLOBAL(uint32_t, 0x00603d24);

    flags = cmpFlags32(c.edi, ringStart());
    if (c.edi >= ringStart()) {
        // The target is inside the pit lane (or past its start).
        flags = cmpFlags32(c.ebx, ringStart());
        if (c.ebx < ringStart()) {
            return flags;
        }
        flags = cmpFlags32(c.edi, c.ebx);
        if (c.edi < c.ebx) {
            for (;;) {
                flags = cmpFlags32(c.ebx, ringStart());
                if (c.ebx <= ringStart()) {
                    return flags;
                }
                c.ebx -= 0x160u;
                visitSegment(c);
                flags = cmpFlags32(c.ebx, c.edi);
                if (c.ebx == c.edi) {
                    return flags;
                }
                if (--c.ecx == 0u) {
                    return flags;
                }
            }
        }
        for (;;) {
            visitSegment(c);
            c.ebx += 0x160u;
            flags = cmpFlags32(c.ebx, GP4_GLOBAL(uint32_t, 0x00866ef8));
            if (c.ebx == GP4_GLOBAL(uint32_t, 0x00866ef8)) {
                return flags;
            }
            flags = cmpFlags32(c.ebx, c.edi);
            if (c.ebx == c.edi) {
                return flags;
            }
            if (--c.ecx == 0u) {
                return flags;
            }
        }
    }

    // The target is on the main lap (before the pit start).
    flags = cmpFlags32(c.ebx, ringStart());
    if (c.ebx >= ringStart()) {
        return flags;
    }
    flags = cmpFlags32(c.edi, c.ebx);
    if (c.edi < c.ebx) {
        for (;;) {
            flags = cmpFlags32(c.ebx, ringStart());
            if (c.ebx >= ringStart()) {
                c.ebx -= 0x160u;
                flags = cmpFlags32(c.ebx, ringStart());
                if (c.ebx < ringStart()) {
                    c.ebx = ringStart();
                }
            } else {
                c.ebx -= 0x160u;
                flags = cmpFlags32(c.ebx, GP4_GLOBAL(uint32_t, 0x00891504));
                if (c.ebx < GP4_GLOBAL(uint32_t, 0x00891504)) {
                    c.ebx += GP4_GLOBAL(uint32_t, 0x00866ec4);
                }
            }
            visitSegment(c);
            flags = cmpFlags32(c.ebx, c.edi);
            if (c.ebx == c.edi) {
                return flags;
            }
            if (--c.ecx == 0u) {
                return flags;
            }
        }
    }
    for (;;) {
        visitSegment(c);
        c.ebx += 0x160u;
        flags = cmpFlags32(c.ebx, GP4_GLOBAL(uint32_t, 0x00866ecc));
        if (!(c.ebx < GP4_GLOBAL(uint32_t, 0x00866ecc))) {
            flags = cmpFlags32(c.ebx, ringStart());
            if (c.ebx < ringStart()) {
                c.ebx -= GP4_GLOBAL(uint32_t, 0x00866ec4);
            } else {
                flags = cmpFlags32(c.ebx, GP4_GLOBAL(uint32_t, 0x00866ef8));
                if (!(c.ebx < GP4_GLOBAL(uint32_t, 0x00866ef8))) {
                    c.ebx = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
                }
            }
        }
        flags = cmpFlags32(c.ebx, c.edi);
        if (c.ebx == c.edi) {
            return flags;
        }
        if (--c.ecx == 0u) {
            return flags;
        }
    }
}

// When the car's two lateral words (+0x90, +0x94) are within 0x200 of each other (signed 16-bit |a-b| < 0x200),
// brings every segment record between the current ring position [0x603d24] and the record edi up to date with
// 0x41ed6b, four steps at most (the pit-lane and main-lap rings wrap differently). eax ends with the absolute
// word difference in its low half (upper half kept); ebx the last record visited; ecx, edx and eax after calls
// come from the callee. eax/ebx/ecx/edx keep their inputs on the early exits.
// FUNCTION: GP4 0x0041ee1f
void __cdecl FUN_0041ee1f(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    uint16_t span = static_cast<uint16_t>(GP4_FIELD(uint16_t, car, 0x90) - GP4_FIELD(uint16_t, car, 0x94));
    if (static_cast<int16_t>(span) < 0) {
        span = static_cast<uint16_t>(0u - span);
    }
    c.eax = (c.eax & 0xffff0000u) | span;
    uint32_t flags = cmpFlags16(span, 0x200u);
    if (static_cast<int16_t>(span) < 0x200) {
        c.ecx = 4u;
        flags = walkRing(c);
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041ee1f, FUN_0041ee1f, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:ptr[1056]:layout=car_0042a278, edi:rec[0x8943a0,0x160,0..31]) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=map_0040c43b+surf_0041f02b+ring_0040c0ae+ring_008943a0+ptr_00603d24+walk_0041ee1f")
