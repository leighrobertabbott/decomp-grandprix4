#include <gp4/gp4.h>

// ebp passes of a stepping loop over a point (ebx, ecx) that advances by (esi, edi): each pass
// looks up the surface class of the point (0x0040c43b), feeds the class's level to the slew-rate
// limiter (0x0040b7e8) and adds the resulting level / target to one of two accumulator pairs,
// chosen by comparing the remaining pass count with [0x6227ac].
// ebp, esi and edi are preserved; ebx and ecx advance; eax = the last [0x622784]; edx = the
// last class. The last flag writer is "dec ebp" (1 -> 0) with CF from the final "add ecx, edi".
// FUNCTION: GP4 0x0040b8cd
void __cdecl FUN_0040b8cd(gp4::Regs* r) {
    uint32_t x = r->ebx;
    uint32_t y = r->ecx;
    const uint32_t stepX = r->esi;
    const uint32_t stepY = r->edi;
    int32_t remaining = static_cast<int32_t>(r->ebp);

    GP4_GLOBAL(int32_t, 0x00622784) = GP4_GLOBAL(int32_t, 0x00622780) << 8;
    GP4_GLOBAL(int32_t, 0x00622788) = GP4_GLOBAL(int32_t, 0x00622790) << 8;

    uint32_t surfaceClass = 0u;
    uint32_t level = 0u;
    uint32_t carry = 0u;
    do {
        gp4::Regs lookup = *r;
        lookup.eax = x;
        lookup.edx = y;
        lookup.ebx = x;
        gp4::call_regs(0x0040c43b, lookup);
        surfaceClass = lookup.eax;

        gp4::Regs limiter = lookup;
        limiter.eax = GP4_ARRAY(uint32_t, 0x006225a8)[surfaceClass] << 8;
        limiter.edx = surfaceClass;
        gp4::call_regs(0x0040b7e8, limiter);

        int32_t decay = GP4_GLOBAL(int32_t, 0x00622788);
        if (decay < 0x10) {
            decay = 0;
        }
        level = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00622784));
        if (remaining > GP4_GLOBAL(int32_t, 0x006227ac)) {
            GP4_GLOBAL(int32_t, 0x006227bc) += decay;
            GP4_GLOBAL(int32_t, 0x006227b4) += static_cast<int32_t>(level);
        } else {
            GP4_GLOBAL(int32_t, 0x006227c0) += decay;
            GP4_GLOBAL(int32_t, 0x006227b8) += static_cast<int32_t>(level);
        }

        x += stepX;
        const uint32_t nextY = y + stepY;
        carry = nextY < y ? 1u : 0u;
        y = nextY;
        --remaining;
    } while (remaining != 0);

    r->eax = level;
    r->ebx = x;
    r->ecx = y;
    r->edx = surfaceClass;
    // dec ebp (1 -> 0): ZF = PF = 1, SF = AF = OF = 0; CF kept from "add ecx, edi".
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u | carry;
}
GP4_IMPL(0x0040b8cd, FUN_0040b8cd, "regs(ebx:u32, ecx:u32, esi:u32, edi:u32, ebp:u32[1..40]) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=map_0040c43b")
