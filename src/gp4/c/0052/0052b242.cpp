#include <gp4/gp4.h>
#include <gp4/flags.h>

static __declspec(noinline) uint32_t nextFlags(uint32_t before, uint32_t inFlags) {
    return gp4::flags::inc32(before, inFlags);
}

// Builds a quoted line in the text buffer 0x00823100: the script text 0x900 (0x0054d798), then
// `"`, the token at [0x0081d410] (0x0052b35a), `"`, the string at 0x0081d5f9 (0x0054d7c0) and the
// script text 0x8ff, and hands the buffer to the (empty) routine 0x0054e232. The flag byte 0x0081d420
// is set to 0xff. Registers and flags come back from the last callee (the `inc edi` flags between the
// calls are kept for the callees that see them).
// FUNCTION: GP4 0x0052b242
void __cdecl FUN_0052b242(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    c.edi = 0x00823100u;
    c.esi = 0x900u;
    gp4::call_regs(0x0054d798, c);

    GP4_FIELD(uint8_t, c.edi, 0) = 0x22u;
    c.eflags = gp4::flags::merge(c.eflags, nextFlags(c.edi, c.eflags));
    c.edi += 1u;
    c.esi = GP4_GLOBAL(uint32_t, 0x0081d410);
    gp4::call_regs(0x0052b35a, c);

    GP4_FIELD(uint8_t, c.edi, 0) = 0x22u;
    c.eflags = gp4::flags::merge(c.eflags, nextFlags(c.edi, c.eflags));
    c.edi += 1u;
    c.esi = 0x0081d5f9u;
    gp4::call_regs(0x0054d7c0, c);

    c.esi = 0x8ffu;
    gp4::call_regs(0x0054d798, c);
    c.esi = 0x00823100u;
    gp4::call_regs(0x0054e232, c);
    GP4_GLOBAL(uint8_t, 0x0081d420) = 0xffu;

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x0052b242, FUN_0052b242, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=script_all_00835846; gfields={0x0081d410:ptr[0x40]:u8{=0x41,0x5a,0x61,0x30,0x39,0x20,0x3a,0x3d,0}, 0x0081d5f9*64/1:u8{=0,1,2,3,5,6,7,8,9,0xa,0xb,0xc,0xd,0xe,0xf,0x10,0x11,0x12,0x13,0x14,0x20,0x41,0x7f,0x80,0xff}}")
