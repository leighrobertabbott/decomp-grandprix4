#include <gp4/gp4.h>
#include <gp4/flags.h>

// True when the signed byte lies strictly between -62 (0xc2) and 62 (0x3e): the loops below redraw
// while `cmp al,0x3e; jge` or `cmp al,0xc2; jle` holds.
static inline bool InRange(uint32_t eax) {
    const int32_t value = (int8_t)(eax & 0xffu);
    return value < 0x3e && value > -0x3e;
}

// FUNCTION: GP4 0x0055ae30
// Re-rolls two signed random bytes (draws of 0x004015dc until the byte lies in -61..61) depending
// on the mode word at [edi]: mode 7 stores the draw in byte 0x008705fe, mode 6 in byte 0x00870600
// (the two tests are independent, so each mode runs at most one loop). eax is left as the last
// draw's result (the callee only changes ax); the flags are those of the last `cmp al, 0xc2` after
// a mode 6 loop, otherwise those of `cmp word [edi], 6`.
void __cdecl FUN_0055ae30(gp4::Regs* r) {
    const uint16_t* mode = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(r->edi));
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    uint32_t arith;

    if (*mode == 7u) {
        do {
            gp4::call_regs(0x004015dc, call);
        } while (!InRange(call.eax));
        GP4_GLOBAL(uint8_t, 0x008705fe) = (uint8_t)call.eax;
    }

    arith = gp4::flags::sub16(*mode, 6u);
    if (*mode == 6u) {
        do {
            gp4::call_regs(0x004015dc, call);
        } while (!InRange(call.eax));
        arith = gp4::flags::sub8(call.eax, 0xc2u);
        GP4_GLOBAL(uint8_t, 0x00870600) = (uint8_t)call.eax;
    }

    r->eax = call.eax;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0055ae30, FUN_0055ae30, "regs(eax:u32, edi:ptr[8]:i16[5..8]) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0062d1b4")
