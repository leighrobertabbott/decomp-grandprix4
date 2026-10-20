#include <gp4/gp4.h>
#include <gp4/flags.h>

// Keep the table lookups opaque (the compiler would fold the constant bases into the loops).
static __declspec(noinline) uint32_t PairByte(uint32_t index) {
    return GP4_ARRAY(uint8_t, 0x007a9452)[index];
}

static __declspec(noinline) void SetPairByte(uint32_t index, uint32_t value) {
    GP4_ARRAY(uint8_t, 0x007a9452)[index] = static_cast<uint8_t>(value);
}

static __declspec(noinline) uint32_t StateWord(uint32_t index) {
    return GP4_ARRAY(uint16_t, 0x007a9426)[index];
}

static __declspec(noinline) uint32_t PairBase() {
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x007a9452)));
}

// Bubble-sorts the list of the 11 entries 0..10 kept as bytes at 0x007a9452 by the state words at
// 0x007a9426 (low 15 bits), highest first. Neighbours a, b with equal nonzero words are ordered by the
// per-place counts 0x0054885d returns for the keys 1..0x16 (the first key where they differ decides:
// the entry with the larger count goes first); neighbours that tie on every key stay in place. Passes
// repeat until one makes no swap.
// Registers of the final pass's last pair: ebx = the list end 0x007a9452 + 10, ecx = 0, edx keeps its upper
// 24 bits (dl = 0), esi/edi the last pair's entries (doubled while only the words were compared, plain
// once the counts were compared) and eax the last compared value. The flags are those of `or dl, dl`.
// FUNCTION: GP4 0x0054873e
void __cdecl FUN_0054873e(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    for (uint32_t i = 0u; i < 0x0bu; ++i) {
        SetPairByte(i, i);
    }

    uint32_t eax = 0u;
    uint32_t esi = 0u;
    uint32_t edi = 0u;
    bool swapped;
    do {
        swapped = false;
        for (uint32_t pair = 0u; pair < 10u; ++pair) {
            esi = PairByte(pair) << 1;
            edi = PairByte(pair + 1u) << 1;
            eax = StateWord(esi >> 1) & 0x7fffu;
            const uint32_t other = StateWord(edi >> 1) & 0x7fffu;
            bool swap = false;
            if (eax <= other) {
                if (eax != other) {
                    swap = true;
                } else if (eax != 0u) {
                    esi >>= 1;
                    edi >>= 1;
                    for (uint32_t key = 1u; key <= 0x16u; ++key) {
                        c.ebx = key;
                        c.edx = edi;
                        gp4::call_regs(0x0054885d, c);
                        const uint32_t first = c.eax;
                        c.edx = esi;
                        gp4::call_regs(0x0054885d, c);
                        eax = c.eax;
                        if (eax > first) {
                            break;
                        }
                        if (eax != first) {
                            swap = true;
                            break;
                        }
                    }
                }
            }
            if (swap) {
                const uint32_t lo = PairByte(pair);
                const uint32_t hi = PairByte(pair + 1u);
                SetPairByte(pair, hi);
                SetPairByte(pair + 1u, lo);
                swapped = true;
            }
        }
    } while (swapped);

    r->eax = eax;
    r->ebx = PairBase() + 10u;
    r->ecx = 0u;
    r->edx = r->edx & 0xffffff00u;
    r->esi = esi;
    r->edi = edi;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(0u));
}
GP4_IMPL(0x0054873e, FUN_0054873e, "regs(edx:u32) -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=words_0054888e+cnt_00548836+slots_005401a2")
