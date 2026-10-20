#include <gp4/gp4.h>
#include <gp4/flags.h>

// Loads edi from the pointer global [0x00838c14] (the text-edit record of the keyboard handler), runs the key
// handler 0x0054dd7a with it and then the empty routine 0x0054decf. The registers and flags are those the
// handler leaves (the empty routine changes nothing).
// FUNCTION: GP4 0x0054dd69
void __cdecl FUN_0054dd69(gp4::Regs* r) {
    r->edi = GP4_GLOBAL(uint32_t, 0x00838c14);
    gp4::call_regs(0x0054dd7a, *r);
    gp4::call_regs(0x0054decf, *r);
}
GP4_IMPL(0x0054dd69, FUN_0054dd69, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:u32, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
