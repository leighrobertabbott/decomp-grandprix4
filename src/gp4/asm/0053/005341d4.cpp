#include <gp4/gp4.h>
#include <gp4/flags.h>

// Looks the key in al up in 12 four-byte entries at 0x0082829b (+0 key byte, +1 row byte,
// +2 column byte). On a hit returns the dword at 0x007ab02c + (row * 6 + column) * 0x10 in eax and
// the index row * 6 + column in ecx (flags of the final `add eax, 0x7ab02c`). Without a hit
// eax = 0, ecx = 0 and the flags are those of the last `dec ecx` (CF from the last `add esi, 4`).
// esi is preserved.

// FUNCTION: GP4 0x005341d4
void __cdecl FUN_005341d4(gp4::Regs* r) {
    const uint8_t key = static_cast<uint8_t>(r->eax);
    const uint8_t* entry = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(0x0082829bu));
    uint32_t left = 12;
    uint32_t flags = 0;
    uint32_t cf = 0;
    uint32_t index = 0;
    uint32_t result = 0;
    for (;;) {
        if (key == entry[0]) {
            index = static_cast<uint32_t>(entry[1]) * 6u + static_cast<uint32_t>(entry[2]);
            const uint32_t offset = index * 0x10u;
            flags = gp4::flags::add32(offset, 0x007ab02cu);
            result = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(offset + 0x007ab02cu));
            break;
        }
        const uint32_t pos = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(entry));
        cf = gp4::flags::add32(pos, 4u) & 1u;
        entry += 4;
        --left;
        if (left == 0) {
            flags = gp4::flags::dec32(1u, cf);
            index = 0;
            result = 0;
            break;
        }
    }
    r->eax = result;
    r->ecx = index;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005341d4, FUN_005341d4, "regs(eax:u32) -> (eax, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
