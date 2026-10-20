#include <gp4/gp4.h>
#include <gp4/flags.h>

// Appends the byte at 0x007c2ea9 to the output cursor in edi (al = byte, [edi++] = al).
// Flags are those of the `inc edi` (CF unchanged).
// FUNCTION: GP4 0x0052ecdc
void __cdecl FUN_0052ecdc(gp4::Regs* r) {
    const uint8_t value = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    uint8_t* cursor = reinterpret_cast<uint8_t*>(r->edi);
    *cursor = value;
    const uint32_t flags = gp4::flags::inc32(r->edi, r->eflags);
    r->edi = r->edi + 1u;
    r->eax = (r->eax & 0xffffff00u) | value;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0052ecdc, FUN_0052ecdc, "regs(eax:u32, edi:ptr[0x40]:bytes, cf:u32[0..1]) -> (eax, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
