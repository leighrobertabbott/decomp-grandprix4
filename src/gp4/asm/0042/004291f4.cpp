#include <gp4/gp4.h>

// The paired conditional calls are kept apart: the compiler would otherwise merge each pair into one call with a
// computed address and the shape audit would not see the original callees.
static __declspec(noinline) void blendFromBand(gp4::Regs& s) {
    gp4::call_regs(0x00403c4c, s);
}

static __declspec(noinline) void blendFromFloor(gp4::Regs& s) {
    gp4::call_regs(0x00403b65, s);
}

static __declspec(noinline) void compareWithBand(gp4::Regs& s) {
    gp4::call_regs(0x00403bb6, s);
}

static __declspec(noinline) void compareWithFloor(gp4::Regs& s) {
    gp4::call_regs(0x00403ab9, s);
}

// FUNCTION: GP4 0x004291f4
// eax = a scale (kept in ebp for the blends), ecx = the mode handed to 0x403938 as edx, edx is kept and used as
// the selector of the blend. 0x403850 first refreshes the byte DAT_00601ba4; with it set the ring record
// [0x866f00] is used, otherwise [0x866f04] - 0x160. The sign-extended word +0x16 of that record goes through
// 0x403938 (mode ecx), then the blend 0x403b65 (0x403c4c when DAT_00601ba6 is set) and the add 0x429100, and
// the sum becomes ecx for the final 0x403ab9 (0x403bb6 when DAT_00601ba6 is set) with eax = 0 and edi = the ring
// record [0x891500] / [0x866ef8] - 0x160. ebx, ebp and edi are restored; eax and edx come from that last
// call, ecx is 1, esi and the flags come out of the callees.
void __cdecl FUN_004291f4(gp4::Regs* r) {
    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    s.ebp = r->eax;
    gp4::call_regs(0x00403850, s);

    uint32_t record;
    if (GP4_GLOBAL(uint8_t, 0x00601ba4) != 0u) {
        record = GP4_GLOBAL(uint32_t, 0x00866f00);
    } else {
        record = GP4_GLOBAL(uint32_t, 0x00866f04) - 0x160u;
    }
    s.edi = record;

    const uint32_t savedEdx = s.edx;
    s.eax = static_cast<uint32_t>(static_cast<int32_t>(
        *reinterpret_cast<const int16_t*>(static_cast<uintptr_t>(record + 0x16u))));
    s.edx = s.ecx;
    gp4::call_regs(0x00403938, s);
    s.edx = savedEdx;

    if (GP4_GLOBAL(uint8_t, 0x00601ba6) != 0u) {
        blendFromBand(s);
    } else {
        blendFromFloor(s);
    }
    gp4::call_regs(0x00429100, s);
    s.ecx = s.eax;

    if (GP4_GLOBAL(uint8_t, 0x00601ba4) != 0u) {
        record = GP4_GLOBAL(uint32_t, 0x00891500);
    } else {
        record = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
    }
    s.edi = record;
    s.eax = 0;

    if (GP4_GLOBAL(uint8_t, 0x00601ba6) != 0u) {
        compareWithBand(s);
    } else {
        compareWithFloor(s);
    }

    r->eax = s.eax;
    r->ecx = 1;
    r->edx = s.edx;
    r->esi = s.esi;
    r->eflags = (r->eflags & ~0x8d5u) | (s.eflags & 0x8d5u);
}
GP4_IMPL(0x004291f4, FUN_004291f4, "regs(eax:i32[-0x4000..0x4000], ebx:u32, ecx:u32[0..9], edx:i32[-0x40000..0x40000], esi:u32, edi:rec[0x8943a0,0x160,0..29]) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403850+ring_008943a0+records_008943a0+div_00403bb6+ringw_004291f4")
