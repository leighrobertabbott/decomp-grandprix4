#include <gp4/gp4.h>
#include <gp4/flags.h>

// Peeks the head of the key queue without consuming it: ax = word at the read cursor [0x008245c4],
// or ax = 0 when the read cursor equals the write cursor [0x008245c0] (empty). Upper half of eax is
// kept, edi is preserved, flags are those of `cmp edi, [0x008245c0]`.
// FUNCTION: GP4 0x0052d7c8
void __cdecl FUN_0052d7c8(gp4::Regs* r) {
    const uint32_t rd = GP4_GLOBAL(uint32_t, 0x008245c4);
    const uint32_t wr = GP4_GLOBAL(uint32_t, 0x008245c0);
    const uint32_t arith = gp4::flags::sub32(rd, wr);
    uint32_t ax = 0u;
    if (rd != wr) {
        ax = *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(rd));
    }
    r->eax = (r->eax & 0xffff0000u) | ax;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0052d7c8, FUN_0052d7c8, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=keyq_008245c0")
