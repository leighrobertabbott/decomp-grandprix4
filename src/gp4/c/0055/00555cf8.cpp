#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stream-decoder opcode step: esi points at a 16-bit code. When the mode byte [0x0086857c] is 2 the
// code is used as a slot number (ecx): 0x00555d74 marks the slot on the record in ebx and, for the slot
// pairs 9<->0x18 and 8<->0x17, 0x00555d42 then moves the partner slot (ecx = partner) onto the same
// record. The work sits inside pushal/popal, so every GPR is restored. In both modes esi advances by 2
// and the flags are those of that `add esi, 2`.
// FUNCTION: GP4 0x00555cf8
void __cdecl FUN_00555cf8(gp4::Regs* r) {
    const uint32_t cursor = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) == 2u) {
        gp4::Regs c = *r;
        c.ecx = *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(cursor));
        gp4::call_regs(0x00555d74, c);

        uint32_t partner = 0u;
        bool paired = true;
        if (c.ecx == 9u) {
            partner = 0x18u;
        } else if (c.ecx == 8u) {
            partner = 0x17u;
        } else if (c.ecx == 0x18u) {
            partner = 9u;
        } else if (c.ecx == 0x17u) {
            partner = 8u;
        } else {
            paired = false;
        }
        if (paired) {
            c.eax = partner;
            c.ecx = partner;
            gp4::call_regs(0x00555d42, c);
        }
    }
    r->esi = cursor + 2u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(cursor, 2u));
}
GP4_IMPL(0x00555cf8, FUN_00555cf8, "regs(esi:ptr[8]:u16{=8,8,9,9,0x17,0x17,0x18,0x18,0,1,2,7,0xa,0x10,0x16,0x19,0x1f}, ebx:rec[0x8943a0,0x160,1..25]) -> (esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=walk_00555c49+mode_0086857c")
