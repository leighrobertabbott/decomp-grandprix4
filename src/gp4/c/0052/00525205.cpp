#include <gp4/gp4.h>
#include <gp4/flags.h>

// Distance in ring records (0x160 bytes each) from the table entry [0x006f2f6c + [0x00679bd8]*4] to
// [0x00679be0], both of which must lie below the pit start [0x00891500] (unsigned), else the result
// stays -1 / 0x14. The signed byte difference d = to - from is folded when |d| >= [0x00866ed8]
// (|d| = [0x00866ec4] - |d| and d negated); [0x006f3ae0] receives d (or -1) and [0x006f3ae4] the
// record count |d| / 0x160 (or 0x14). Leaves eax = quotient, edx = remainder, ecx = 0x160, ebx = the
// index; on the early exits eax/edx/ebx hold the loaded values. The flags are those of the last
// flag writer (idiv leaves them): `neg eax` when folded, else `cmp ecx, [0x00866ed8]`, or the
// compare against the pit start on the early exits.
// FUNCTION: GP4 0x00525205
void __cdecl FUN_00525205(gp4::Regs* r) {
    GP4_GLOBAL(int32_t, 0x006f3ae0) = -1;
    GP4_GLOBAL(int32_t, 0x006f3ae4) = 0x14;

    const uint32_t index = GP4_GLOBAL(uint32_t, 0x00679bd8);
    const uint32_t from = GP4_ARRAY(uint32_t, 0x006f2f6c)[index];
    uint32_t to = GP4_GLOBAL(uint32_t, 0x00679be0);
    r->ebx = index;
    r->edx = from;
    r->eax = to;

    uint32_t arith = gp4::flags::sub32(from, GP4_GLOBAL(uint32_t, 0x00891500));
    if (from >= GP4_GLOBAL(uint32_t, 0x00891500)) {
        r->eflags = gp4::flags::merge(r->eflags, arith);
        return;
    }
    arith = gp4::flags::sub32(to, GP4_GLOBAL(uint32_t, 0x00891500));
    if (to >= GP4_GLOBAL(uint32_t, 0x00891500)) {
        r->eflags = gp4::flags::merge(r->eflags, arith);
        return;
    }

    const uint32_t diff_flags_in = to;
    to -= from;
    (void)diff_flags_in;
    uint32_t span = to;
    if (static_cast<int32_t>(span) < 0) {
        span = 0u - span;
    }
    const uint32_t half = GP4_GLOBAL(uint32_t, 0x00866ed8);
    arith = gp4::flags::sub32(span, half);
    if (static_cast<int32_t>(span) >= static_cast<int32_t>(half)) {
        span = 0u - span;
        span += GP4_GLOBAL(uint32_t, 0x00866ec4);
        arith = gp4::flags::neg32(to);
        to = 0u - to;
    }
    GP4_GLOBAL(uint32_t, 0x006f3ae0) = to;

    int32_t rem = 0;
    const int32_t quot = gp4::idiv64(static_cast<int64_t>(static_cast<int32_t>(span)), 0x160, &rem);
    GP4_GLOBAL(int32_t, 0x006f3ae4) = quot;

    r->eax = static_cast<uint32_t>(quot);
    r->edx = static_cast<uint32_t>(rem);
    r->ecx = 0x160u;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00525205, FUN_00525205, "regs() -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ptrtab_00524fa4+ptrs_006f2f6c+ring_00525205")
