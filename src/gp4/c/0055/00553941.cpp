#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00553941
// Resets the object tables unless the byte [0x00933990] is set: clears the dword [0x00866db8], the
// dword +0x6c of the 13 entries of 0xe0 bytes at 0x00866248, bit 0 of the dword +0x4c of the 0x140
// entries of 0xe0 bytes at 0x00854414 and the 0x40 bytes at 0x00866e47, then sets the byte
// [0x006f3b7b] to 0xff. The body is a pushal/popal pair: every register is kept. Flags: those of
// `test byte [0x933990], 0xff` when the byte is set (early return), else those of the last
// `add esi, 0xe0` of the second loop (popal does not restore them).
void __cdecl FUN_00553941(gp4::Regs* r) {
    const uint32_t busy = GP4_GLOBAL(uint8_t, 0x00933990);
    if (busy != 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(busy));
        return;
    }
    GP4_GLOBAL(uint32_t, 0x00866db8) = 0u;

    uint8_t* entry = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(0x00866248));
    for (int left = 13; left != 0; --left) {
        GP4_FIELD(uint32_t, entry, 0x6c) = 0u;
        entry += 0xe0;
    }

    uint32_t flags = 0u;
    uint8_t* object = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(0x00854414));
    for (int left = 0x140; left != 0; --left) {
        GP4_FIELD(uint32_t, object, 0x4c) &= 0xfffffffeu;
        flags = gp4::flags::add32(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(object)), 0xe0u);
        object += 0xe0;
    }

    uint8_t* bytes = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(0x00866e47));
    for (int left = 0x40; left != 0; --left) {
        *bytes++ = 0u;
    }
    GP4_GLOBAL(uint8_t, 0x006f3b7b) = 0xffu;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00553941, FUN_00553941, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
