#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00556fc4
// R3 stream handler: in mode 2 ([0x86857c]) remembers the record edi in [0x866f08] and stores the low 11 bits
// of its word +0x1a in the word [0x868562] (ax carries them out). Either way esi advances by 2; the flags are
// those of that final `add esi, 2`.
void __cdecl FUN_00556fc4(gp4::Regs* r) {
    const uint32_t esi = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) == 2u) {
        GP4_GLOBAL(uint32_t, 0x00866f08) = r->edi;
        uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
        const uint16_t value = static_cast<uint16_t>(GP4_FIELD(uint16_t, record, 0x1a) & 0x7ffu);
        GP4_GLOBAL(uint16_t, 0x00868562) = value;
        r->eax = (r->eax & 0xffff0000u) | value;
    }
    r->esi = esi + 2u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi, 2u));
}
GP4_IMPL(0x00556fc4, FUN_00556fc4, "regs(eax:u32, esi:u32, edi:ptr[0x40]:bytes) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_0086857c")
