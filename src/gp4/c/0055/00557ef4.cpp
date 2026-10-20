#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stream command handler (mode byte 0x0086857c): in mode 3 it loads ax with the word at esi + 4 (the
// slot mask) and runs the entry copy 0x00557e76 on the entry at esi; every mode then skips the 6-byte
// record (esi += 6), so the flags are those of that `add`.
// FUNCTION: GP4 0x00557ef4
void __cdecl FUN_00557ef4(gp4::Regs* r) {
    const uint32_t entry = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) == 3u) {
        const uint16_t mask = *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(entry + 4u));
        r->eax = (r->eax & 0xffff0000u) | mask;
        gp4::call_regs(0x00557e76, *r);
    }
    r->esi = entry + 6u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(entry, 6u));
}
GP4_IMPL(0x00557ef4, FUN_00557ef4, "regs(eax:u32, esi:ptr[0x10]:i16[-3..12]) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_0086857c+steps_00875918")
