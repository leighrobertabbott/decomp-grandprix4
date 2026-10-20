#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stores the nibble value (byte 0x00874b4e) into byte [ebx+0x24]: the low nibble when dl is 0x20
// (the high nibble kept), otherwise the high nibble (value << 4, the low nibble kept). al leaves
// holding the value (shifted in the second case); the flags are those of the final `or`.
// FUNCTION: GP4 0x0055926c
void __cdecl FUN_0055926c(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint8_t al = GP4_GLOBAL(uint8_t, 0x00874b4e);
    uint8_t kept;
    if ((r->edx & 0xffu) == 0x20u) {
        kept = static_cast<uint8_t>(record[0x24] & 0xf0u);
    } else {
        kept = static_cast<uint8_t>(record[0x24] & 0x0fu);
        al = static_cast<uint8_t>(al << 4);
    }
    const uint8_t result = static_cast<uint8_t>(kept | al);
    record[0x24] = result;
    r->eax = (r->eax & 0xffffff00u) | al;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(result));
}
GP4_IMPL(0x0055926c, FUN_0055926c, "regs(eax:u32, edx:u32{0x20,0,0x41,0xff,0x120}, ebx:ptr[0x30]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
