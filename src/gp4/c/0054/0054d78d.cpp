#include <gp4/gp4.h>

// `mov edi, 0x823100 ; call 0x0054d798 ; ret`: expands the text script with id esi (string pointer
// from the table 0x835846, via the interpreter 0x0054d798) into the static buffer at 0x00823100.
// Every other register and the flags come back from the callee.
// FUNCTION: GP4 0x0054d78d
void __cdecl FUN_0054d78d(gp4::Regs* r) {
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&GP4_GLOBAL(uint8_t, 0x00823100)));
    gp4::call_regs(0x0054d798, *r);
}
GP4_IMPL(0x0054d78d, FUN_0054d78d, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:u32[0..7], ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798")
