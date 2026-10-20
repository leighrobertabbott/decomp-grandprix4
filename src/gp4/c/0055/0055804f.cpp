#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stream command handler (mode byte 0x0086857c): in any mode but 1 only skips a word (esi += 2, flags
// of the `add`). In mode 1 it reads the word at esi (esi += 2), sets its bits in the dword
// 0x00870634 and clears them in 0x00870630; eax ends as the complement of the zero-extended word and
// the flags are those of the last `and`.
// FUNCTION: GP4 0x0055804f
void __cdecl FUN_0055804f(gp4::Regs* r) {
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 1u) {
        const uint32_t before = r->esi;
        r->esi = before + 2u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(before, 2u));
        return;
    }
    const uint32_t word = *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(r->esi));
    r->esi += 2u;
    GP4_GLOBAL(uint32_t, 0x00870634) |= word;
    const uint32_t cleared = GP4_GLOBAL(uint32_t, 0x00870630) & ~word;
    GP4_GLOBAL(uint32_t, 0x00870630) = cleared;
    r->eax = ~word;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(cleared));
}
GP4_IMPL(0x0055804f, FUN_0055804f, "regs(eax:u32, esi:ptr[8]:bytes) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_0086857c")
