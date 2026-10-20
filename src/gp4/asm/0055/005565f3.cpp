#include <gp4/gp4.h>
#include <gp4/x87.h>
#include <gp4/flags.h>

using gp4::x87::Ext;

// ebx == 0 arm: the 0x60 lane vector (0x00553ab7) and its scale (0x005539f0).
static __declspec(noinline) void CallLaneA(gp4::Regs* c) {
    gp4::call_regs(0x00553ab7, *c);
    gp4::call_regs(0x005539f0, *c);
}

// ebx != 0 arm: the 0x62 lane vector (0x00553ad4) and its scale (0x00553a26).
static __declspec(noinline) void CallLaneB(gp4::Regs* c) {
    gp4::call_regs(0x00553ad4, *c);
    gp4::call_regs(0x00553a26, *c);
}

// Bearing and distance from the lane vector 0x934df0/0x934df4 (set by the callees from the record edi) to
// the point (eax, edx): eax = round(atan2(eax - X, edx - Z) * [0x6f2be4]) (an angle in 2^31/pi units),
// edx = round(sqrt(dx^2 + dz^2)). Closer than 0x555 the angle is replaced by [edi+0xec] -/+ 0x40000000
// (ebx == 0 / != 0). ecx = ecx - (the callee's ecx); ebp is saved and restored; the flags are those of
// that `sub eax, ecx`.
// FUNCTION: GP4 0x005565f3
void __cdecl FUN_005565f3(gp4::Regs* r) {
    const uint32_t x_in = r->eax;
    const uint32_t z_in = r->edx;
    const uint32_t count_in = r->ecx;
    const uint32_t selector = r->ebx;
    const uint32_t record = r->edi;

    gp4::Regs c = *r;
    if (selector == 0u) {
        CallLaneA(&c);
    } else {
        CallLaneB(&c);
    }

    const uint32_t dx = x_in - GP4_GLOBAL(uint32_t, 0x00934df0);
    GP4_GLOBAL(uint32_t, 0x006f2bf4) = dx;
    const Ext fx = gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4));
    const uint32_t dz = z_in - GP4_GLOBAL(uint32_t, 0x00934df4);
    GP4_GLOBAL(uint32_t, 0x006f2bf4) = dz;
    const Ext fz = gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4));
    const int32_t bearing =
        gp4::x87::fistp32(gp4::x87::fpatan(fx, fz) * gp4::x87::ext(GP4_GLOBAL(double, 0x006f2be4)));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = bearing;
    uint32_t result = static_cast<uint32_t>(bearing);

    GP4_GLOBAL(uint32_t, 0x006f2bf4) = dx;
    const Ext sx = gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4));
    const Ext sx2 = sx * sx;
    GP4_GLOBAL(uint32_t, 0x006f2bf4) = dz;
    const Ext sz = gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4));
    const Ext sz2 = sz * sz;
    const int32_t distance = gp4::x87::fistp32(gp4::x87::fsqrt(sx2 + sz2));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = distance;

    if (static_cast<uint32_t>(distance) < 0x555u) {
        result = GP4_FIELD(uint32_t, record, 0xec);
        if (selector == 0u) {
            result -= 0x40000000u;
        } else {
            result += 0x40000000u;
        }
    }

    r->eax = result;
    r->edx = static_cast<uint32_t>(distance);
    r->ecx = count_in - c.ecx;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(count_in, c.ecx));
}
GP4_IMPL(0x005565f3, FUN_005565f3, "regs(eax:i32{-0x5000,0x5000,-0x5200,0x5200,-0x4e00,0x4e00,-0x5100,0x5100,0,0x7fffffff,-0x80000000}, ebx:u32{=0,0,1,0xffffffff}, ecx:u32, edx:i32[-0x500..0x500], esi:u32, ebp:u32, edi:ptr[0x158]:layout=lane_005565f3) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=multiply_0040ce7b; cases=400")
