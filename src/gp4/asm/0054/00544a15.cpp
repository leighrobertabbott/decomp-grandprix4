#include <gp4/gp4.h>

// Formats the text buffer at [0x00831568]: 0x54 lines of 0x50 spaces followed by CR LF (0x0d,
// 0x0a), 0x1ae8 bytes in all. Registers left behind: edi = one past the last byte written,
// ecx = 0 (the `loop` counter ran out), ax = 0x0a0d (the CR LF word of the last stosw; the high
// half of eax is untouched). No instruction here writes the flags.
// FUNCTION: GP4 0x00544a15
void __cdecl FUN_00544a15(gp4::Regs* r) {
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00831568)));
    for (uint32_t line = 0x54u; line != 0u; --line) {
        for (uint32_t col = 0x50u; col != 0u; --col) {
            *dst++ = 0x20u;
        }
        *dst++ = 0x0du;
        *dst++ = 0x0au;
    }
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(dst));
    r->ecx = 0u;
    r->eax = (r->eax & 0xffff0000u) | 0x0a0du;
}
GP4_IMPL(0x00544a15, FUN_00544a15, "regs(eax:u32) -> (eax, ecx, edi); globals=layout=surf_0054654a")
