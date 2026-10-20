#include <gp4/gp4.h>
#include <gp4/x87.h>

// The scaled angle, `fild dword ; fmul qword [0x006f2bdc]`, stored as a double.
static inline double scaledAngle(int32_t angle) {
    return gp4::x87::narrow(gp4::x87::ext_i32(angle) * gp4::x87::ext(GP4_GLOBAL(double, 0x006f2bdc)));
}

// FUNCTION: GP4 0x004226ae
// Sweeps the sphere of directions for the bounding routine: the outer angle [0x00608304] starts at
// 2^31 / ([0x00608318]) / 2 - 0x40000000 and advances by 2^32 / [0x00608318] for [0x00608318] / 2
// rings; each ring has max(|cos(angle)| * [0x00608318] >> 14, 12) samples of the second angle
// [0x00608308] (step 2^32 / samples, starting at half a step), the third angle [0x0060830c] is 0.
// For every sample the three angles (times the dword-to-radian scale [0x006f2bdc]) are stored as
// doubles in the body 0x00622f68, expanded to sin / cos and a rotation matrix (0x00401d80,
// 0x00401da2), and the entries measured and counted (0x00422463, 0x00422557). Registers carry
// whatever the last calls left (esi, edi, ebx, ebp, edx = the body), eax = [0x00608310]; the flags
// are those of the final `dec [0x00608320]` (ZF, PF) with CF from the last `add [0x00608304], eax`.
void __cdecl FUN_004226ae(gp4::Regs* r) {
    gp4::Regs state = *r;
    state.st_in = 0;
    state.st_out = 0;

    uint32_t remainder;
    GP4_GLOBAL(uint32_t, 0x0060830c) = 0;
    const uint32_t ringStep = gp4::div64(0x100000000ull, GP4_GLOBAL(uint32_t, 0x00608318), &remainder);
    GP4_GLOBAL(uint32_t, 0x00608310) = ringStep;
    GP4_GLOBAL(uint32_t, 0x00608304) = static_cast<uint32_t>(static_cast<int32_t>(ringStep) >> 1) + 0xc0000000u;
    GP4_GLOBAL(uint32_t, 0x00608320) = GP4_GLOBAL(uint32_t, 0x00608318) >> 1;

    uint32_t carry;
    do {
        uint16_t angle = static_cast<uint16_t>(GP4_GLOBAL(uint32_t, 0x00608304) >> 16);
        if (static_cast<int16_t>(angle) < 0) {
            angle = static_cast<uint16_t>(0u - angle);
        }
        const uint32_t byteOffset = (static_cast<uint32_t>(angle) >> 2) & 0xfffeu;
        int32_t cosine = GP4_FIELD(int16_t, GP4_ARRAY(uint8_t, 0x007c5c5c), byteOffset);
        if (cosine < 0) {
            cosine = -cosine;
        }
        const int64_t product = static_cast<int64_t>(cosine) * GP4_GLOBAL(int32_t, 0x00608318);
        int32_t samples = static_cast<int32_t>(static_cast<uint64_t>(product) >> 14);
        if (samples < 0xc) {
            samples = 0xc;
        }
        GP4_GLOBAL(int32_t, 0x0060831c) = samples;

        const uint32_t sampleStep = gp4::div64(0x100000000ull, static_cast<uint32_t>(samples), &remainder);
        GP4_GLOBAL(uint32_t, 0x00608314) = sampleStep;
        GP4_GLOBAL(uint32_t, 0x00608308) = static_cast<uint32_t>(static_cast<int32_t>(sampleStep) >> 1);
        GP4_GLOBAL(int32_t, 0x00608324) = samples;
        state.eax = static_cast<uint32_t>(samples);

        do {
            uint8_t* body = GP4_ARRAY(uint8_t, 0x00622f68);
            state.edx = 0x00622f68u;
            GP4_FIELD(double, body, 0x10) = scaledAngle(GP4_GLOBAL(int32_t, 0x00608304));
            GP4_FIELD(double, body, 0x18) = scaledAngle(GP4_GLOBAL(int32_t, 0x00608308));
            GP4_FIELD(double, body, 0x20) = scaledAngle(GP4_GLOBAL(int32_t, 0x0060830c));
            gp4::call_regs(0x00401d80, state);
            gp4::call_regs(0x00401da2, state);
            gp4::call_regs(0x00422463, state);
            gp4::call_regs(0x00422557, state);
            state.eax = GP4_GLOBAL(uint32_t, 0x00608314);
            GP4_GLOBAL(uint32_t, 0x00608308) += state.eax;
            GP4_GLOBAL(int32_t, 0x00608324) -= 1;
        } while (GP4_GLOBAL(int32_t, 0x00608324) != 0);

        state.eax = GP4_GLOBAL(uint32_t, 0x00608310);
        const uint32_t before = GP4_GLOBAL(uint32_t, 0x00608304);
        GP4_GLOBAL(uint32_t, 0x00608304) = before + state.eax;
        carry = (before + state.eax) < before ? 1u : 0u;
        GP4_GLOBAL(uint32_t, 0x00608320) -= 1;
    } while (GP4_GLOBAL(uint32_t, 0x00608320) != 0);

    r->eax = state.eax;
    r->ebx = state.ebx;
    r->ecx = state.ecx;
    r->edx = state.edx;
    r->esi = state.esi;
    r->edi = state.edi;
    r->ebp = state.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u | carry;
}
GP4_IMPL(0x004226ae, FUN_004226ae, "regs(ebp:u32) -> (eax, ebx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=state_0042458b+state_0042257b")
