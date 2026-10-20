#include <gp4/gp4.h>
#include <gp4/flags.h>

// Byte-code skipper: esi points at an opcode byte. If the opcode is below 0x14, esi advances by the
// signed length in the dword table 0x00833f98[opcode] (flags of that `add`; eax is saved and restored).
// Otherwise esi is incremented until it reaches an opcode below 0x14 (flags of the last
// `cmp byte [esi], 0x14`).
// FUNCTION: GP4 0x0054d0c7
void __cdecl FUN_0054d0c7(gp4::Regs* r) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t esi = r->esi;
    uint8_t op = *p;

    if (op < 0x14u) {
        const uint32_t delta = GP4_ARRAY(uint32_t, 0x00833f98)[op];
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi, delta));
        r->esi = esi + delta;
        return;
    }

    do {
        esi += 1u;
        op = GP4_GLOBAL(uint8_t, esi);
    } while (op >= 0x14u);

    r->esi = esi;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(op, 0x14u));
}
GP4_IMPL(0x0054d0c7, FUN_0054d0c7, "regs(esi:ptr[0x40]:u8{0,1,2,3,4,5,6,7,8,9,0x0a,0x13,0x14,0x20,0xff}) -> (esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
