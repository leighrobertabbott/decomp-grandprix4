#include <gp4/gp4.h>
#include <gp4/flags.h>

// cmp bp, 4 ; jne ret ; test word [esi+0x14], 0xffff ; ret. Only eflags changes: those of the
// 16-bit compare, replaced by those of the word test when bp == 4.
// FUNCTION: GP4 0x005517ee
void __cdecl FUN_005517ee(gp4::Regs* r) {
    const uint32_t bp = r->ebp & 0xffffu;
    uint32_t f = gp4::flags::sub16(bp, 4u);                      // cmp bp, 4
    if (bp == 4u) {
        const uint16_t word = *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(r->esi + 0x14u));
        f = gp4::flags::logic16(word);                           // test word [esi+0x14], 0xffff
    }
    r->eflags = gp4::flags::merge(r->eflags, f);
}
GP4_IMPL(0x005517ee, FUN_005517ee, "regs(ebp:u32[0..8], esi:ptr[0x40]:bytes) -> (cf, pf, af, zf, sf, of)")
