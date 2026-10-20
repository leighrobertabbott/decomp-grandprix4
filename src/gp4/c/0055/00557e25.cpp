#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stream command handler (mode byte 0x0086857c): in mode 3 it loads the slot mask eax = 0x3ff and runs
// the entry copy 0x00557e76 on the 4-byte entry at esi; every mode then skips the entry (esi += 4),
// so the flags are those of that `add`.
// FUNCTION: GP4 0x00557e25
void __cdecl FUN_00557e25(gp4::Regs* r) {
    const uint32_t entry = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) == 3u) {
        r->eax = 0x3ffu;
        gp4::call_regs(0x00557e76, *r);
    }
    r->esi = entry + 4u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(entry, 4u));
}
GP4_IMPL(0x00557e25, FUN_00557e25, "regs(eax:u32, esi:ptr[0x10]:i16[-3..12]) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_0086857c+steps_00875918")
