#include <gp4/gp4.h>
#include <gp4/flags.h>

// Kept as separate call sites so both original callee addresses stay literals (the compiler would
// otherwise merge them into one call with a selected address).
static __declspec(noinline) void callNegated(gp4::Regs& c) {
    gp4::call_regs(0x00553b21, c);
}
static __declspec(noinline) void callPlain(gp4::Regs& c) {
    gp4::call_regs(0x00553b79, c);
}

// Q14 lerp of (ecx, [0x934df0], [0x934df4]) towards the values of the neighbouring track record.
// eax is the signed fraction: 0 returns at once (flags of `or eax, eax`). Otherwise the record edi
// steps one record forward (eax > 0: lap end [0x866ecc] / pit start [0x891500] wrap by [0x866ec4] /
// pit end [0x866ef8] clamp) or backward (eax < 0: pit start clamp / lap start [0x891504] wrap), and
// 0x00553b21 (dl == -1) or 0x00553b79 computes that record's ecx and pair 0x934df0/0x934df4. Each
// of the three is then sar 5 and blended: old + ((new - old) * |eax| >> 14). eax = ecx = the blended
// ecx, edx = the high dword of its product; ebx/esi/edi/ebp are restored; the flags are those of the
// final add.
// FUNCTION: GP4 0x00550ed5
void __cdecl FUN_00550ed5(gp4::Regs* r) {
    const uint32_t fraction = r->eax;
    if (fraction == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(0u));
        return;
    }
    const int32_t selector = static_cast<int8_t>(r->edx & 0xffu);
    uint32_t weight = fraction;
    if (static_cast<int32_t>(weight) < 0) {
        weight = 0u - weight;
    }

    uint32_t edi = r->edi;
    if (static_cast<int32_t>(fraction) >= 0) {
        edi += 0x160u;
        if (edi < GP4_GLOBAL(uint32_t, 0x00866ecc)) {
        } else if (edi < GP4_GLOBAL(uint32_t, 0x00891500)) {
            edi -= GP4_GLOBAL(uint32_t, 0x00866ec4);
        } else if (edi < GP4_GLOBAL(uint32_t, 0x00866ef8)) {
        } else {
            edi = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
        }
    } else if (edi < GP4_GLOBAL(uint32_t, 0x00891500)) {
        edi -= 0x160u;
        if (edi < GP4_GLOBAL(uint32_t, 0x00891504)) {
            edi += GP4_GLOBAL(uint32_t, 0x00866ec4);
        }
    } else {
        edi -= 0x160u;
        if (edi < GP4_GLOBAL(uint32_t, 0x00891500)) {
            edi = GP4_GLOBAL(uint32_t, 0x00891500);
        }
    }

    const uint32_t oldEcx = r->ecx;
    const uint32_t oldX = GP4_GLOBAL(uint32_t, 0x00934df0);
    const uint32_t oldY = GP4_GLOBAL(uint32_t, 0x00934df4);

    gp4::Regs c = *r;
    c.esi = static_cast<uint32_t>(selector);
    c.ebp = weight;
    c.edi = edi;
    if (selector == -1) {
        callNegated(c);
    } else {
        callPlain(c);
    }
    GP4_GLOBAL(int32_t, 0x00934df0) = GP4_GLOBAL(int32_t, 0x00934df0) >> 5;
    GP4_GLOBAL(int32_t, 0x00934df4) = GP4_GLOBAL(int32_t, 0x00934df4) >> 5;
    const uint32_t target = static_cast<uint32_t>(static_cast<int32_t>(c.ecx) >> 5);
    const int64_t w = static_cast<int32_t>(weight);

    const int64_t py = static_cast<int64_t>(static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00934df4) - oldY)) * w;
    GP4_GLOBAL(uint32_t, 0x00934df4) = static_cast<uint32_t>(py >> 14) + oldY;

    const int64_t px = static_cast<int64_t>(static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00934df0) - oldX)) * w;
    GP4_GLOBAL(uint32_t, 0x00934df0) = static_cast<uint32_t>(px >> 14) + oldX;

    const int64_t pc = static_cast<int64_t>(static_cast<int32_t>(target - oldEcx)) * w;
    const uint32_t shifted = static_cast<uint32_t>(pc >> 14);
    const uint32_t result = shifted + oldEcx;

    r->eax = result;
    r->ecx = result;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(pc) >> 32);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(shifted, oldEcx));
}
GP4_IMPL(0x00550ed5, FUN_00550ed5, "regs(eax:i32{0,0x4000,-0x4000,0x2000,-0x2000,1,-1}, edx:u32{0xff,0xffffffff,0,1,0x7f}, ecx:i32, ebx:u32[0..0x1fc], edi:rec[0x8943a0,0x160,0..29]) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=ring_008943a0+state_0040342a+ring_00406ac7+tbl_00868598+multiply_0040ce7b+tail_00550ed5")
