#include <gp4/gp4.h>
#include <gp4/flags.h>

// Resets the replay/leaderboard state: runs the block copier 0x00541dd0, clears the dword
// [0x00b1bb40], sets [0x008307a4] = 0x66, initialises the 22 slots of 0x1c bytes at 0x00b1b8d8
// (dword +0, bytes +4, +0x14, +0x15 cleared, +0x16 = 0xff), stores the performance counter
// (0x0054e733) + 0x60 << 32 into the 22 qwords at 0x0083002b, fills the 22 bytes at 0x007c2a1c with
// 0xff, runs 0x0053f7ca and 0x0053ee80 and ends with [0x0082fffc] = 0xff, [0x0082fffd] = 0.
// Registers: eax = counter low dword with its low byte 0xff (rep stosb), edx = counter high + 0x60
// (the `adc` after `add eax, 0`), ebx and ecx and the flags are 0x0053ee80's, edi = 0x007c2a32;
// esi and ebp are untouched.
// FUNCTION: GP4 0x0053f740
void __cdecl FUN_0053f740(gp4::Regs* r) {
    gp4::cdecl_<0x00541dd0, void>();
    GP4_GLOBAL(uint32_t, 0x00b1bb40) = 0u;
    GP4_GLOBAL(uint32_t, 0x008307a4) = 0x66u;

    uint8_t* slot = GP4_ARRAY(uint8_t, 0x00b1b8d8);
    for (int32_t i = 0x16; i != 0; --i) {
        GP4_FIELD(uint32_t, slot, 0) = 0u;
        GP4_FIELD(uint8_t, slot, 4) = 0u;
        GP4_FIELD(uint8_t, slot, 0x15) = 0u;
        GP4_FIELD(uint8_t, slot, 0x14) = 0u;
        GP4_FIELD(uint8_t, slot, 0x16) = 0xffu;
        slot += 0x1c;
    }

    gp4::Regs q = *r;
    q.st_in = 0;
    q.st_out = 0;
    gp4::call_regs(0x0054e733, q);
    const uint32_t low = q.eax;
    const uint32_t high = q.edx + 0x60u;
    uint8_t* stamp = GP4_ARRAY(uint8_t, 0x0083002b);
    for (int32_t i = 0x16; i != 0; --i) {
        GP4_FIELD(uint32_t, stamp, 0) = low;
        GP4_FIELD(uint32_t, stamp, 4) = high;
        stamp += 8;
    }

    uint8_t* mark = GP4_ARRAY(uint8_t, 0x007c2a1c);
    for (int32_t i = 0x16; i != 0; --i) {
        *mark++ = 0xffu;
    }

    gp4::cdecl_<0x0053f7ca, void>();
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0053ee80, c);
    GP4_GLOBAL(uint8_t, 0x0082fffc) = 0xffu;
    GP4_GLOBAL(uint8_t, 0x0082fffd) = 0u;

    r->eax = (low & 0xffffff00u) | 0xffu;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = high;
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(mark));
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0053f740, FUN_0053f740, "regs() -> (eax, ebx, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=qpc_0054e733")
