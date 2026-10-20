#include <gp4/gp4.h>
#include <gp4/flags.h>

// Remaps the 0x2a00 index bytes at 0x00922704 through five 256-byte lookup tables that start at
// 0x009346d8 + 0x100: block k (k = 1..5) is stored at 0x00922704 + k * 0x2a00 as
// table[k * 0x100 + src[i]]. pushal/popal wrapper: every GPR is restored; the flags are those
// of the final `cmp edi, 0xfc00` (equal).
static __declspec(noinline) uint8_t* advance(uint8_t* p, uint32_t bytes) { return p + bytes; }

// FUNCTION: GP4 0x0051b7cd
void __cdecl FUN_0051b7cd(gp4::Regs* r) {
    const uint8_t* src = GP4_ARRAY(const uint8_t, 0x00922704);
    const uint8_t* table = advance(GP4_ARRAY(uint8_t, 0x009346d8), 0x100u);
    uint8_t* dst = advance(GP4_ARRAY(uint8_t, 0x00922704), 0x2a00u);
    for (uint32_t block = 0u; block != 5u; ++block) {
        for (uint32_t i = 0u; i != 0x2a00u; ++i) {
            dst[i] = table[src[i]];
        }
        table = advance(const_cast<uint8_t*>(table), 0x100u);
        dst = advance(dst, 0x2a00u);
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0xfc00u, 0xfc00u));
}
GP4_IMPL(0x0051b7cd, FUN_0051b7cd, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=map_0051b7cd; wall=1800")
