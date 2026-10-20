#include <gp4/gp4.h>

// FUNCTION: GP4 0x00403ab9
// Position of eax between the interpolated edges at edi, as a clamped Q14 ratio via 0x00403d10.
// ecx < eax: edge pair 0x00403b19 (low side), edx = -(max(-min(eax, 0), edge) + 0x100), ebp = -1;
// otherwise: edge pair 0x00403b3f (high side), edx = max(max(eax, 0), edge) + 0x100, ebp = 0.
// Returns eax = ratio and edx = ebp (-1 / 0); ecx and ebp are preserved, the flags are those of
// the final 0x00403d10 call.
void __cdecl FUN_00403ab9(gp4::Regs* r) {
    const int32_t value = static_cast<int32_t>(r->eax);
    const int32_t reference = static_cast<int32_t>(r->ecx);

    gp4::Regs call = *r;
    int32_t edx;
    int32_t ebp;

    if (reference < value) {
        edx = value > 0 ? 0 : value;
        gp4::call_regs(0x00403b19, call);
        ebp = static_cast<int32_t>(call.edx);
        edx = -edx;
        if (edx < ebp) {
            edx = ebp;
        }
        edx = -(edx + 0x100);
        ebp = -1;
    } else {
        edx = value < 0 ? 0 : value;
        gp4::call_regs(0x00403b3f, call);
        ebp = static_cast<int32_t>(call.edx);
        if (edx < ebp) {
            edx = ebp;
        }
        edx += 0x100;
        ebp = 0;
    }

    call.edx = static_cast<uint32_t>(edx);
    gp4::call_regs(0x00403d10, call);

    r->eax = call.eax;
    r->edx = static_cast<uint32_t>(ebp);
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x00403ab9, FUN_00403ab9, "regs(eax:i32[-40000..40000], ecx:i32[-40000..40000], edi:ptr[0x1ca]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
