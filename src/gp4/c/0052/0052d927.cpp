#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stores the current key bindings of the controls screen: the flag byte [0x82fd49] is set to 0xff and the
// six words at 0x7ab55a are copied (`rep movsw`) to the 0x34-byte entry [0x7ab544] of the table at
// 0x7ab5c6; the key captions of that entry are then refreshed by 0x0052e131 (eax = the entry index, which is
// preserved by its pushal/popal). esi / edi end just past the copied words, ecx at 0; the flags are the
// callee's.
// FUNCTION: GP4 0x0052d927
void __cdecl FUN_0052d927(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x0082fd49) = 0xffu;
    const uint32_t entry = GP4_GLOBAL(uint8_t, 0x007ab544);
    uint16_t* dst = reinterpret_cast<uint16_t*>(static_cast<uintptr_t>(entry * 0x34u + 0x007ab5c6u));
    const uint16_t* src = GP4_ARRAY(const uint16_t, 0x007ab55a);
    for (uint32_t n = 0u; n != 6u; ++n) {
        dst[n] = src[n];
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = GP4_GLOBAL(uint8_t, 0x007ab544);
    c.ecx = 0u;
    c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(src + 6));
    c.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(dst + 6));
    gp4::call_regs(0x0052e131, c);

    r->eax = c.eax;
    r->ecx = c.ecx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0052d927, FUN_0052d927, "regs() -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 MapVirtualKeyA")
