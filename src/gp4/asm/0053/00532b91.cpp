#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sets the current directory with 0x005367b9 (al = 1 picks the root string), then loads the file
// named by esi into the buffer edx (capacity -1 = unlimited) with 0x005364c5. The pushal/popal
// around the first call restores every register, leaving only its flags (overwritten by the second
// call). eax then holds the dword [0x0081ea30]; when the load failed (CF set) the byte
// [0x007a93e6] is set to 0xff. edi = edx and ecx = -1 on return; the flags are those of 0x005364c5.
// FUNCTION: GP4 0x00532b91
void __cdecl FUN_00532b91(gp4::Regs* r) {
    r->edi = r->edx;
    r->ecx = 0xffffffffu;
    gp4::Regs c = *r;
    c.eax = 1u;
    gp4::call_regs(0x005367b9, c);
    r->eflags = c.eflags;
    gp4::call_regs(0x005364c5, *r);
    r->eax = GP4_GLOBAL(uint32_t, 0x0081ea30);
    if ((r->eflags & static_cast<uint32_t>(gp4::flags::CF)) != 0u) {
        GP4_GLOBAL(uint8_t, 0x007a93e6) = 0xffu;
    }
}
GP4_IMPL(0x00532b91, FUN_00532b91, "regs(edx:u32, esi:ptr[0x40]:bytes) -> (eax, ecx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
