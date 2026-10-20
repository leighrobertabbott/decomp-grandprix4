#include <gp4/gp4.h>
#include <gp4/flags.h>

// Byte-code skipper: esi points at an opcode byte. Opcodes >= 0x14, 8 and 9 are one byte, 3 is three
// bytes and 5 is two bytes. If the first opcode is any other (< 0x14) opcode, esi advances by the
// signed length in the dword table 0x00833f98[opcode] and the routine returns (flags of that add).
// Otherwise the one-, two- and three-byte opcodes are skipped until an "other" opcode is reached,
// where esi stays and the flags are those of the last `cmp byte [esi], 5`. eax is saved and restored.
// FUNCTION: GP4 0x0054d0e4
void __cdecl FUN_0054d0e4(gp4::Regs* r) {
    const uint8_t* p = (const uint8_t*)(uintptr_t)r->esi;
    uint32_t esi = r->esi;
    uint8_t op = *p;

    if (op < 0x14u && op != 8u && op != 9u && op != 3u && op != 5u) {
        const uint32_t delta = GP4_ARRAY(uint32_t, 0x00833f98)[op];
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi, delta));
        r->esi = esi + delta;
        return;
    }

    for (;;) {
        if (op >= 0x14u || op == 8u || op == 9u) {
            esi += 1u;
        } else if (op == 3u) {
            esi += 3u;
        } else if (op == 5u) {
            esi += 2u;
        } else {
            break;
        }
        op = *(const uint8_t*)(uintptr_t)esi;
    }

    r->esi = esi;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(op, 5u));
}
GP4_IMPL(0x0054d0e4, FUN_0054d0e4, "regs(esi:ptr[0x40]:u8{0,1,2,3,4,5,6,7,8,9,0x0a,0x13,0x14,0x20,0xff}) -> (esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
