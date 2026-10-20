#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stream command handler (mode byte 0x0086857c): in any mode but 1 only skips a word (esi += 2, flags
// of the `add`). In mode 1 it reads the word at esi (eax = zero-extended word, esi += 2) and ORs it
// into the dwords 0x00870634 and 0x00870630; the flags are those of the last `or`.
// FUNCTION: GP4 0x0055802f
void __cdecl FUN_0055802f(gp4::Regs* r) {
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 1u) {
        const uint32_t before = r->esi;
        r->esi = before + 2u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(before, 2u));
        return;
    }
    const uint32_t word = *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(r->esi));
    r->eax = word;
    r->esi += 2u;
    GP4_GLOBAL(uint32_t, 0x00870634) |= word;
    const uint32_t merged = GP4_GLOBAL(uint32_t, 0x00870630) | word;
    GP4_GLOBAL(uint32_t, 0x00870630) = merged;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(merged));
}
GP4_IMPL(0x0055802f, FUN_0055802f, "regs(eax:u32, esi:ptr[8]:bytes) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_0086857c")
