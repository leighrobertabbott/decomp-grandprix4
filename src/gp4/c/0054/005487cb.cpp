#include <gp4/gp4.h>
#include <gp4/flags.h>

// The compiler would fold the constant table bases into the loop; keep the lookups opaque.
static __declspec(noinline) uint32_t PairByte(uint32_t index) {
    return GP4_ARRAY(uint8_t, 0x007a9452)[index];
}

static __declspec(noinline) uint32_t StateWord(uint32_t index) {
    return GP4_ARRAY(uint16_t, 0x007a9426)[index];
}

static __declspec(noinline) void MarkWord(uint32_t index) {
    GP4_ARRAY(uint16_t, 0x007a9426)[index] |= 0x8000u;
}

static __declspec(noinline) uint32_t PairBase() {
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x007a9452)));
}

// Walks the ten neighbouring pairs (a, b) of the byte list at 0x007a9452 (indices into the 11 state
// words at 0x007a9426). When both words have the same nonzero value in their low 15 bits and the
// per-place counts 0x0054885d(a) and 0x0054885d(b) agree for every key 1..0x16, bit 15 of word b is
// set. The registers are those of the last pair: ebx = the list end 0x007a9452 + 10, ecx = 0 and
// esi/edi/eax/edx the last pair's indices (doubled byte offsets while only the words were compared,
// edi doubled again once the mark was set) and compared values. The flags are those of the final
// `inc ebx`, whose CF is that of the last pair's last flag writer.
// FUNCTION: GP4 0x005487cb
void __cdecl FUN_005487cb(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    uint32_t eax = 0u;
    uint32_t edx = 0u;
    uint32_t esi = 0u;
    uint32_t edi = 0u;
    uint32_t carry = 0u;
    for (uint32_t pair = 0u; pair < 10u; ++pair) {
        esi = PairByte(pair) << 1;
        edi = PairByte(pair + 1u) << 1;
        eax = StateWord(esi >> 1) & 0x7fffu;
        edx = StateWord(edi >> 1) & 0x7fffu;
        carry = eax < edx ? 1u : 0u;
        if (eax != edx) {
            continue;
        }
        carry = 0u;
        if (eax == 0u) {
            continue;
        }

        esi >>= 1;
        edi >>= 1;
        bool same = true;
        for (uint32_t key = 1u; key <= 0x16u; ++key) {
            c.ebx = key;
            c.edx = edi;
            gp4::call_regs(0x0054885d, c);
            const uint32_t first = c.eax;
            c.edx = esi;
            gp4::call_regs(0x0054885d, c);
            eax = c.eax;
            edx = first;
            carry = eax < edx ? 1u : 0u;
            if (eax != edx) {
                same = false;
                break;
            }
        }
        if (!same) {
            continue;
        }

        edi <<= 1;
        MarkWord(edi >> 1);
        carry = 0u;
    }

    const uint32_t last = PairBase() + 9u;
    r->eax = eax;
    r->ebx = last + 1u;
    r->ecx = 0u;
    r->edx = edx;
    r->esi = esi;
    r->edi = edi;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::inc32(last, carry));
}
GP4_IMPL(0x005487cb, FUN_005487cb, "regs() -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=words_0054888e+cnt_00548836+slots_005401a2+pairs_005487cb")
