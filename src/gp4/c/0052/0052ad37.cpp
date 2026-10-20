#include <gp4/gp4.h>

// `mov ebp, -1 ; mov edi, 0x823100 ; call 0x0052ad57 ; ret`: formats the time value in eax (below
// 0x3e80000 ms; split into hours / minutes / seconds / thousandths by 0x0052ae08, with every field
// written, as ebp = -1 forces) as text into the static buffer at 0x823100. The callee restores every
// register but edi, which ends on the NUL terminator; ebp comes back as -1.
// FUNCTION: GP4 0x0052ad37
void __cdecl FUN_0052ad37(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.ebp = 0xffffffffu;
    c.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&GP4_GLOBAL(uint8_t, 0x00823100)));
    gp4::call_regs(0x0052ad57, c);
    *r = c;
}
GP4_IMPL(0x0052ad37, FUN_0052ad37, "regs(eax:u32[0..0x4000000]) -> (ebp, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798+script_0052ad37")
