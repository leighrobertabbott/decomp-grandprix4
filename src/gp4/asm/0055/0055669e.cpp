#include <gp4/gp4.h>
#include <gp4/x87.h>
#include <gp4/flags.h>

using gp4::x87::Ext;

static __declspec(noinline) void RecordNegativeSide(gp4::Regs& c) {
    gp4::call_regs(0x00553ab7, c);
    gp4::call_regs(0x005539f0, c);
}

static __declspec(noinline) void RecordPositiveSide(gp4::Regs& c) {
    gp4::call_regs(0x00553ad4, c);
    gp4::call_regs(0x00553a26, c);
}

// Rotated offset of a track record (edi): picks the lateral vector of the record (ebx = 0: the word at
// +0x60 negated, 0x00553ab7 then 0x005539f0; otherwise the word at +0x62, 0x00553ad4 then 0x00553a26),
// which leaves the base position in [0x00934df0]/[0x00934df4] and ecx = scaled term + [edi + 0xf4].
// Then the input vector (length edx) is rotated by the angle eax * [0x006f2bdc] (fsincos, each product
// rounded with fistp through [0x006f2bf4]): eax = round(sin * edx) + [0x00934df0],
// edx = round(cos * edx) + [0x00934df4], ecx = the incoming ecx + the ecx left by the record routine.
// ebp is saved and restored; the flags are those of the final `add eax, ecx` (incoming ecx + callee ecx).
// FUNCTION: GP4 0x0055669e
void __cdecl FUN_0055669e(gp4::Regs* r) {
    const uint32_t angleIn = r->eax;
    const uint32_t lengthIn = r->edx;
    const uint32_t ecxIn = r->ecx;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    if (r->ebx == 0u) {
        RecordNegativeSide(c);
    } else {
        RecordPositiveSide(c);
    }

    GP4_GLOBAL(int32_t, 0x006f2bf4) = static_cast<int32_t>(angleIn);
    const Ext theta = gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4)) * gp4::x87::ext(GP4_GLOBAL(double, 0x006f2bdc));
    Ext sine;
    Ext cosine;
    gp4::x87::fsincos(theta, &sine, &cosine);

    GP4_GLOBAL(int32_t, 0x006f2bf4) = static_cast<int32_t>(lengthIn);
    const int32_t across = gp4::x87::fistp32(sine * gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4)));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = across;
    const uint32_t resultEax = static_cast<uint32_t>(across) + GP4_GLOBAL(uint32_t, 0x00934df0);

    GP4_GLOBAL(int32_t, 0x006f2bf4) = static_cast<int32_t>(lengthIn);
    const int32_t down = gp4::x87::fistp32(cosine * gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4)));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = down;
    const uint32_t resultEdx = static_cast<uint32_t>(down) + GP4_GLOBAL(uint32_t, 0x00934df4);

    r->eax = resultEax;
    r->edx = resultEdx;
    r->ecx = ecxIn + c.ecx;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(ecxIn, c.ecx));
}
GP4_IMPL(0x0055669e, FUN_0055669e, "regs(eax:u32, ebx:u32{0,1}, ecx:u32, edx:i32[-1000000..1000000], edi:ptr[0x158]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:f64+layout=multiply_0040ce7b")
