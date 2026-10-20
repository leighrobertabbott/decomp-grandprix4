#include <gp4/gp4.h>

// eax indexes the 16-byte entries of the table at 0x00868598 / 0x0086c598 and ecx is the position
// looked up (interpolated by 0x4031ed): a position at or below the first limit uses entry slot +0,
// one within 0x18 of the second limit slot +4, otherwise slot +8 (and DAT_00627848 receives the
// fraction left in DAT_00627844). esi and ecx are saved and restored; eax and the flags are those
// of 0x4031ed, edx is 1 (slots +0 / +4) or 5 (slot +8).
// FUNCTION: GP4 0x004032cb
void __cdecl FUN_004032cb(gp4::Regs* r) {
    const int32_t position = static_cast<int32_t>(r->ecx);
    const uint32_t entry = r->eax * 0x10u;

    GP4_GLOBAL(uint32_t, 0x00627848) = 0u;

    gp4::Regs callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    uint32_t edx;
    if (position <= GP4_FIELD(int32_t, GP4_ARRAY(uint8_t, 0x0086859c), entry)) {
        callee.esi = entry;
        callee.ecx = static_cast<uint32_t>(position);
        gp4::call_regs(0x004031ed, callee);
        edx = 1u;
        GP4_GLOBAL(uint8_t, 0x00625aba) = 0xffu;
    } else if (position - 0x18 <= GP4_FIELD(int32_t, GP4_ARRAY(uint8_t, 0x008685a0), entry)) {
        callee.esi = entry + 4u;
        callee.ecx = static_cast<uint32_t>(position - 0x18);
        gp4::call_regs(0x004031ed, callee);
        edx = 1u;
        GP4_GLOBAL(uint8_t, 0x00625aba) = 0xffu;
    } else {
        callee.esi = entry + 8u;
        callee.ecx = static_cast<uint32_t>(position - 0x18);
        gp4::call_regs(0x004031ed, callee);
        GP4_GLOBAL(uint32_t, 0x00627848) = GP4_GLOBAL(uint32_t, 0x00627844);
        edx = 5u;
    }

    r->eax = callee.eax;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | (callee.eflags & 0x8d5u);
}
GP4_IMPL(0x004032cb, FUN_004032cb, "regs(eax:u32[0..0x3f], ecx:i32[-64..64]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
