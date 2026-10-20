#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stream command handler (mode byte 0x0086857c): in any mode but 2 only skips a word (esi += 2, flags of
// the `add`). In mode 2 it reads the word at esi (ax = word, esi += 2) and stores its low byte into
// the record ebx: +0x17 when cl == 2, otherwise +0x16. The flags are those of `cmp cl, 2`.
// FUNCTION: GP4 0x00556e5c
void __cdecl FUN_00556e5c(gp4::Regs* r) {
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 2u) {
        const uint32_t before = r->esi;
        r->esi = before + 2u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(before, 2u));
        return;
    }
    const uint16_t word = *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t sel = static_cast<uint8_t>(r->ecx);
    r->eax = (r->eax & 0xffff0000u) | word;
    r->esi += 2u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(sel, 2u));
    if (sel == 2u) {
        GP4_FIELD(uint8_t, r->ebx, 0x17) = static_cast<uint8_t>(word);
    } else {
        GP4_FIELD(uint8_t, r->ebx, 0x16) = static_cast<uint8_t>(word);
    }
}
GP4_IMPL(0x00556e5c, FUN_00556e5c, "regs(eax:u32, ecx:u32{2,2,0,0x11,0x102,0xffffff02}, ebx:ptr[0x20]:bytes, esi:ptr[8]:bytes) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_0086857c")
