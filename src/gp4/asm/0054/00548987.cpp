#include <gp4/gp4.h>
#include <gp4/flags.h>

// Text-script wrapper (`mov esi, eax; add esi, 0xb90; call 0x54d798; ret`): runs the text interpreter
// 0x0054d798 (string pointer from table 0x835846, expanded into the buffer at edi) on the text id
// esi = eax + 0xb90. Every register and the flags come back from the callee.
// FUNCTION: GP4 0x00548987
void __cdecl FUN_00548987(gp4::Regs* r) {
    const uint32_t id = r->eax;
    r->esi = id + 0xb90u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(id, 0xb90u));
    gp4::call_regs(0x0054d798, *r);
}
GP4_IMPL(0x00548987, FUN_00548987, "regs(eax:u32{=0xfffff470,0xfffff471,0xfffff472,0xfffff473,0xfffff474,0xfffff475,0xfffff476,0xfffff477}, ebx:u32, ecx:u32, edx:u32, edi:ptr[0x100]:bytes, ebp:u32, esi:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798")
