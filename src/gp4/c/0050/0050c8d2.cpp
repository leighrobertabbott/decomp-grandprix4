#include <gp4/gp4.h>
#include <gp4/flags.h>

// Hand-written helper: runs the inverse projection 0x00404815 on the pair stored at
// 0x008754ac / 0x008754b4 (edi = record pointer, a hidden input of the callee), stores the pair
// it returns in 0x00934df0 / 0x00934df4, then tests whether
//   ((x >> 8) * [0x0073d198] + 0x2000) >> 14) + 0x80  is below 0x100 (unsigned):
// CF = 0 when it is, CF = 1 when it is not (clc / stc after the cmp). Returns eax = that sum and
// edx = (y + 0x100) >> 9 (arithmetic); the other registers come back from the callee.
// FUNCTION: GP4 0x0050c8d2
void __cdecl FUN_0050c8d2(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.eax = GP4_GLOBAL(uint32_t, 0x008754ac);
    c.edx = GP4_GLOBAL(uint32_t, 0x008754b4);
    gp4::call_regs(0x00404815, c);

    const uint32_t x = c.eax;
    const uint32_t y = c.edx;
    GP4_GLOBAL(uint32_t, 0x00934df0) = x;
    GP4_GLOBAL(uint32_t, 0x00934df4) = y;

    const uint32_t yScaled = static_cast<uint32_t>(static_cast<int32_t>(y + 0x100u) >> 9);
    const int32_t xScaled = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00934df0)) >> 8;
    const int64_t product = static_cast<int64_t>(xScaled) * GP4_GLOBAL(int32_t, 0x0073d198);
    const uint64_t rounded = static_cast<uint64_t>(product) + 0x2000u;
    const uint32_t sum = static_cast<uint32_t>(rounded >> 14) + 0x80u;

    uint32_t arith = gp4::flags::sub32(sum, 0x100u) & ~gp4::flags::CF;
    if (sum >= 0x100u) {
        arith |= gp4::flags::CF;
    }

    r->eax = sum;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = yScaled;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(c.eflags, arith);
}
GP4_IMPL(0x0050c8d2, FUN_0050c8d2, "regs(edi:ptr[0x40]:i16[-3000..3000]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32; gfields={0x008754ac:i32[-1048576..1048576], 0x008754b4:i32[-1048576..1048576]}")
