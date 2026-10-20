#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal; XOR-decrypts the buffer at edx in place with a key stream, popal; clc; ret.
// The key starts at ([0x96a364] | 1) and is multiplied by 5 after every dword (`lea eax,[eax+eax*4]`);
// ecx>>2 dwords are xored (ecx>>2 must be nonzero: the `loop` would otherwise run 2^32 times), then the
// ecx&3 trailing bytes are xored with the low byte of the key, shifting it right by 8 each time.
// popal restores every register; the flags left are those of the last flag writer (`and ecx,3` when there
// is no tail, otherwise the final `inc edx`), with CF forced clear by the `clc` epilogue.
// FUNCTION: GP4 0x0051cf39
void __cdecl FUN_0051cf39(gp4::Regs* r) {
    const uint32_t length = r->ecx;
    uint8_t* data = reinterpret_cast<uint8_t*>(r->edx);

    uint32_t key = GP4_GLOBAL(uint32_t, 0x0096a364) | 1u;
    uint32_t dwords = length >> 2;
    uint32_t* wp = reinterpret_cast<uint32_t*>(data);
    do {
        *wp ^= key;
        key = key + key * 4u;
        ++wp;
        --dwords;
    } while (dwords != 0u);

    uint8_t* bp = reinterpret_cast<uint8_t*>(wp);
    uint32_t tail = length & 3u;
    uint32_t arith = gp4::flags::logic32(tail);
    while (tail != 0u) {
        *bp ^= static_cast<uint8_t>(key);
        key >>= 8;
        arith = gp4::flags::inc32(reinterpret_cast<uint32_t>(bp), 0u);
        ++bp;
        --tail;
    }
    r->eflags = gp4::flags::merge(r->eflags, arith & ~gp4::flags::CF);
}
GP4_IMPL(0x0051cf39, FUN_0051cf39, "regs(ecx:u32[4..0x3f], edx:ptr[0x40]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
