#include <gp4/gp4.h>
#include <gp4/flags.h>

// Builds the two 256-byte character-case tables: [0x00833d18 + c] = the low byte of CharUpperA(c) (thunk
// 0x005a4978) and [0x00833e18 + c] = the low byte of CharLowerA(c) (thunk 0x005a4972), for c = 0..255
// (a value below 0x10000 is a single character). The stdcall thunks are called with live registers: eax
// (the counter) and edi (the output cursor) are pushed around each call, ecx/edx come back from the API.
// Registers on return: eax = 0x100, edi = 0x00833f18; the flags are those of the final `cmp eax, 0x100`.
// FUNCTION: GP4 0x0054d70c
void __cdecl FUN_0054d70c(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    uint32_t index = 0u;
    uint32_t dest = 0x00833d18u;
    do {
        gp4::Regs call = c;
        call.eax = index;
        call.edi = dest;
        const uint32_t args[1] = {index};
        gp4::call_regs_stdcall(0x005a4978, call, args);
        GP4_FIELD(uint8_t, dest, 0) = static_cast<uint8_t>(call.eax);
        c = call;
        dest += 1u;
        index += 1u;
    } while (index < 0x100u);

    index = 0u;
    dest = 0x00833e18u;
    do {
        gp4::Regs call = c;
        call.eax = index;
        call.edi = dest;
        const uint32_t args[1] = {index};
        gp4::call_regs_stdcall(0x005a4972, call, args);
        GP4_FIELD(uint8_t, dest, 0) = static_cast<uint8_t>(call.eax);
        c = call;
        dest += 1u;
        index += 1u;
    } while (index < 0x100u);

    c.eax = index;
    c.edi = dest;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(index, 0x100u));
    *r = c;
}
GP4_IMPL(0x0054d70c, FUN_0054d70c, "regs() -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); emu=skip:win32 CharUpperA")
