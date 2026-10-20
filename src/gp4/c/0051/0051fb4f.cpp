#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0051fb4f
// Maps the code byte [0x007c2ea9] to a small index in eax: 1 -> 1, 3 -> 2, 0x44 -> 3, 2 -> 4,
// 0x80 -> 5, anything else (including 0) -> 0. The compares are tried in that order; the flags are
// those of the last `cmp byte [0x7c2ea9], k` that ran.
void __cdecl FUN_0051fb4f(gp4::Regs* r) {
    static const uint8_t keys[] = {0x00u, 0x01u, 0x03u, 0x44u, 0x02u, 0x80u};
    static const uint32_t results[] = {0u, 1u, 2u, 3u, 4u, 5u};
    const uint32_t code = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    uint32_t value = 0u;
    uint32_t flags = 0u;
    for (int i = 0; i < 6; ++i) {
        flags = gp4::flags::sub8(code, keys[i]);
        if (code == keys[i]) {
            value = results[i];
            break;
        }
    }
    r->eax = value;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0051fb4f, FUN_0051fb4f, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
