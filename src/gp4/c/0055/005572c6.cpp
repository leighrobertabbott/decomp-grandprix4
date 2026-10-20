#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stream command handler (mode byte 0x0086857c): in any mode but 1 the cursor esi just skips a
// dword. In mode 1 it reads two words at esi (ax then dx, esi advances by 2 each), keeps them in
// ax/dx and stores them at 0x0086855e (ax) and 0x00868560 (dx). The flags are those of the last
// `add esi, ...` in both paths.
// FUNCTION: GP4 0x005572c6
void __cdecl FUN_005572c6(gp4::Regs* r) {
    const uint32_t esi = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 1u) {
        r->esi = esi + 4u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi, 4u));
        return;
    }
    const uint16_t first = GP4_FIELD(uint16_t, esi, 0);
    const uint16_t second = GP4_FIELD(uint16_t, esi, 2);
    r->eax = (r->eax & 0xffff0000u) | first;
    r->edx = (r->edx & 0xffff0000u) | second;
    GP4_GLOBAL(uint16_t, 0x00868560) = second;
    GP4_GLOBAL(uint16_t, 0x0086855e) = first;
    r->esi = esi + 4u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi + 2u, 2u));
}
GP4_IMPL(0x005572c6, FUN_005572c6, "regs(eax:u32, edx:u32, esi:ptr[8]:bytes) -> (eax, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_0086857c")
