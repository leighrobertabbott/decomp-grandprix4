#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040b955
// Runs the stepping routine 0x0040b8cd over the four corner points (-0x400, -0x400),
// (-0x400, 0x400), (0x400, 0x400) and (0x400, -0x400) of the square of half-width 0x400.
// esi/edi (step) and ebp (pass count) are passed through; ebx/ecx are reloaded before each
// call; eax, ebx, ecx, edx and the flags are those of the last call.
void __cdecl FUN_0040b955(gp4::Regs* r) {
    // Flags of "neg ebx" / "neg ecx" on +-0x400: CF, PF and SF set.
    const uint32_t kNegFlags = 0x85u;
    gp4::Regs call = *r;

    call.ebx = static_cast<uint32_t>(-0x400);
    call.ecx = static_cast<uint32_t>(-0x400);
    call.eflags = (call.eflags & ~0x8d5u) | kNegFlags;
    gp4::call_regs(0x0040b8cd, call);

    call.ebx = static_cast<uint32_t>(-0x400);
    call.ecx = 0x400u;
    call.eflags = (call.eflags & ~0x8d5u) | kNegFlags;
    gp4::call_regs(0x0040b8cd, call);

    call.ebx = 0x400u;
    call.ecx = 0x400u;
    gp4::call_regs(0x0040b8cd, call);

    call.ebx = 0x400u;
    call.ecx = static_cast<uint32_t>(-0x400);
    call.eflags = (call.eflags & ~0x8d5u) | kNegFlags;
    gp4::call_regs(0x0040b8cd, call);

    r->eax = call.eax;
    r->ebx = call.ebx;
    r->ecx = call.ecx;
    r->edx = call.edx;
    r->eflags = call.eflags;
}
GP4_IMPL(0x0040b955, FUN_0040b955, "regs(esi:u32, edi:u32, ebp:u32[1..40]) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=map_0040c43b")
