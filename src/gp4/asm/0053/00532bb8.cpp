#include <gp4/gp4.h>
#include <gp4/flags.h>

// Loads the file named by esi into the buffer edx (capacity -1 = unlimited) with 0x005364c5. eax
// then holds the dword [0x0081ea30] whatever the outcome; when the load failed (CF set) the byte
// [0x007a93e6] is set to 0xff. edi = edx and ecx = -1 on return; the flags are those of 0x005364c5.
// FUNCTION: GP4 0x00532bb8
void __cdecl FUN_00532bb8(gp4::Regs* r) {
    r->edi = r->edx;
    r->ecx = 0xffffffffu;
    gp4::call_regs(0x005364c5, *r);
    r->eax = GP4_GLOBAL(uint32_t, 0x0081ea30);
    if ((r->eflags & static_cast<uint32_t>(gp4::flags::CF)) != 0u) {
        GP4_GLOBAL(uint8_t, 0x007a93e6) = 0xffu;
    }
}
GP4_IMPL(0x00532bb8, FUN_00532bb8, "regs(edx:u32, esi:ptr[0x40]:bytes) -> (eax, ecx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
