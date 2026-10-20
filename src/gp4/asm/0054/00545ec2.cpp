#include <gp4/gp4.h>
#include <gp4/flags.h>

// Text-script wrapper (`mov esi, eax; add esi, 0xba6; call 0x54d798; ret`): runs the text interpreter
// 0x0054d798 on the text id esi = eax + 0xba6 (so eax = id - 0xba6). Every register and the flags come
// back from the callee.
// FUNCTION: GP4 0x00545ec2
void __cdecl FUN_00545ec2(gp4::Regs* r) {
    const uint32_t id = r->eax;
    r->esi = id + 0xba6u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(id, 0xba6u));
    gp4::call_regs(0x0054d798, *r);
}
GP4_IMPL(0x00545ec2, FUN_00545ec2, "regs(eax:u32{=0xfffff45a,0xfffff45b,0xfffff45c,0xfffff45d,0xfffff45e,0xfffff45f,0xfffff460,0xfffff461}, ebx:u32, ecx:u32, edx:u32, edi:ptr[0x100]:bytes, ebp:u32, esi:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798")
