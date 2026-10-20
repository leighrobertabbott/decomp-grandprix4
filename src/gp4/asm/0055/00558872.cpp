#include <gp4/gp4.h>
#include <gp4/flags.h>

// One flag dword: `test f, a; je; or f, b` then `test f, b; je; or f, a` (a set implies b set, and
// the reverse); *arith gets the flags of the last test/or.
static inline void PairBits(uint8_t* rec, int off, uint32_t a, uint32_t b, uint32_t* arith) {
    uint32_t f = GP4_FIELD(uint32_t, rec, off);
    *arith = gp4::flags::logic32(f & a);
    if ((f & a) != 0u) {
        f |= b;
        GP4_FIELD(uint32_t, rec, off) = f;
        *arith = gp4::flags::logic32(f);
    }
    *arith = gp4::flags::logic32(f & b);
    if ((f & b) != 0u) {
        f |= a;
        GP4_FIELD(uint32_t, rec, off) = f;
        *arith = gp4::flags::logic32(f);
    }
}

// When both bits of a pair are set in ebp | [edi+0x2c], each of the dwords +0x3c, +0x30, +0x34,
// +0x38 and +0x40 gets the other bit of the pair whenever it has one of them; done for the pairs
// (0x100000, 0x20) and then (0x80000, 0x10). eax = ebp | [edi+0x2c]; the flags are those of the
// last test/or.
// FUNCTION: GP4 0x00558872
void __cdecl FUN_00558872(gp4::Regs* r) {
    uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    static const int kFields[5] = {0x3c, 0x30, 0x34, 0x38, 0x40};
    static const uint32_t kPairs[2][2] = {{0x100000u, 0x20u}, {0x80000u, 0x10u}};
    uint32_t arith = 0;
    uint32_t mask = 0;
    for (int p = 0; p < 2; ++p) {
        const uint32_t hi = kPairs[p][0];
        const uint32_t lo = kPairs[p][1];
        mask = r->ebp | GP4_FIELD(uint32_t, rec, 0x2c);
        arith = gp4::flags::logic32(mask & hi);
        if ((mask & hi) == 0u) {
            continue;
        }
        arith = gp4::flags::logic32(mask & lo);
        if ((mask & lo) == 0u) {
            continue;
        }
        for (int k = 0; k < 5; ++k) {
            PairBits(rec, kFields[k], lo, hi, &arith);
        }
    }
    r->eax = mask;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00558872, FUN_00558872, "regs(ebp:u32, edi:ptr[0x48]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
