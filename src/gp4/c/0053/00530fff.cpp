#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies the replacement dwords 0x007ad880.. over the current ones 0x007ad870.. ([0x007a97a4] + 1
// dwords, `rep movsd`) unless both gates say no: byte 0x007a97a3 nonzero and byte 0x0082741e zero.
// With the copy done esi/edi sit past the last dword and ecx = 0; the flags are those of `inc ecx`
// (CF kept from the gate compare), or of the last gate compare when nothing is copied.
// FUNCTION: GP4 0x00530fff
void __cdecl FUN_00530fff(gp4::Regs* r) {
    const uint32_t gate = GP4_GLOBAL(uint8_t, 0x007a97a3);
    uint32_t flags = gp4::flags::sub8(gate, 0u);
    if (gate != 0u) {
        const uint32_t second = GP4_GLOBAL(uint8_t, 0x0082741e);
        flags = gp4::flags::sub8(second, 0u);
        if (second == 0u) {
            r->eflags = gp4::flags::merge(r->eflags, flags);
            return;
        }
    }
    uint32_t count = GP4_GLOBAL(uint32_t, 0x007a97a4);
    flags = gp4::flags::inc32(count, flags);
    count += 1u;
    const uint32_t* src = GP4_ARRAY(uint32_t, 0x007ad880);
    uint32_t* dst = GP4_ARRAY(uint32_t, 0x007ad870);
    for (uint32_t i = 0u; i != count; ++i) {
        dst[i] = src[i];
    }
    r->esi = 0x007ad880u + count * 4u;
    r->edi = 0x007ad870u + count * 4u;
    r->ecx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00530fff, FUN_00530fff, "regs() -> (ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=count_007a97a4+dev_0053106d")
