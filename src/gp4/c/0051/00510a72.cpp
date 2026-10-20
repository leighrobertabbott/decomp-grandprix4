#include <gp4/gp4.h>

// Refreshes the six words 0xa69880..0xa69890 from the car record selected by the index
// [0x00a69878] (clamped to 0..21 by 0x005106f6, record in esi): [0xa69880] = dword +0x3c of the
// record, arithmetic >> 16; the four helper results (0x004082e2 with edx = [0xa6987c], 0x004082ee,
// 0x00408271 with ebx = [0x009beb90], 0x004082cd) go to 0xa69884, 0xa69888, 0xa6988c and 0xa69890.
// pushal/popal wrapper: every GPR is restored, the flags are those of the last flag writer (popal
// does not restore them): the "cmp ebx,-1" inside 0x004082cd.
// FUNCTION: GP4 0x00510a72
void __cdecl FUN_00510a72(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.ecx = GP4_GLOBAL(uint32_t, 0x00a69878);
    gp4::call_regs(0x005106f6, c);
    const uint8_t* record = (const uint8_t*)(uintptr_t)c.esi;
    GP4_GLOBAL(int32_t, 0x00a69880) = *(const int32_t*)(record + 0x3c) >> 16;

    c.edx = GP4_GLOBAL(uint32_t, 0x00a6987c);
    gp4::call_regs(0x004082e2, c);
    GP4_GLOBAL(uint32_t, 0x00a69884) = c.eax;

    gp4::call_regs(0x004082ee, c);
    GP4_GLOBAL(uint32_t, 0x00a69888) = c.eax;

    c.ebx = GP4_GLOBAL(uint32_t, 0x009beb90);
    gp4::call_regs(0x00408271, c);
    GP4_GLOBAL(uint32_t, 0x00a6988c) = c.eax;

    gp4::call_regs(0x004082cd, c);
    GP4_GLOBAL(uint32_t, 0x00a69890) = c.eax;

    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00510a72, FUN_00510a72, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carbg_0066fef0+state_00408271+idx_00510a72")
