#include <gp4/gp4.h>
#include <gp4/flags.h>

// Claims a free driver slot key: al is the key. The 22 key bytes at 0x007c2a1c are scanned and the
// matches counted; when more than one slot already holds the key, FUN_00540698 (release) clears
// them all and the count restarts at zero. With no match left, the first free slot (key byte 0xff)
// receives the key (nothing is stored when all 22 are in use); one match leaves the table as is.
// ebx and ecx are saved and restored. The flags are those of the last flag writer: `or ecx, ecx`
// when a single match ends the routine, else the final `cmp ebx, 0x16` of the free-slot scan.
// FUNCTION: GP4 0x005406bf
void __cdecl FUN_005406bf(gp4::Regs* r) {
    const uint32_t key = r->eax & 0xffu;
    uint8_t* const keys = GP4_ARRAY(uint8_t, 0x007c2a1c);
    uint32_t matches = 0u;
    for (uint32_t i = 0u; i != 0x16u; ++i) {
        if (keys[i] == key) {
            ++matches;
        }
    }
    if (matches > 1u) {
        gp4::Regs c = *r;
        gp4::call_regs(0x00540698, c);
        matches = 0u;
    }
    uint32_t arith = gp4::flags::logic32(matches);
    if (matches == 0u) {
        uint32_t slot = 0u;
        while (keys[slot] != 0xffu && slot < 0x16u) {
            ++slot;
        }
        arith = gp4::flags::sub32(slot, 0x16u);
        if (slot < 0x16u) {
            keys[slot] = static_cast<uint8_t>(key);
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005406bf, FUN_005406bf, "regs(eax:u32{1,2,3,0xff,0x20,0x80}) -> (cf, pf, af, zf, sf, of); globals=layout=blocks_0054045b+drv22_0052d402+keys_007c2a1c")
