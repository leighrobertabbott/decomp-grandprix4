#include <gp4/gp4.h>
#include <gp4/flags.h>

// The original indexes four dword tables with the constant edx = 2 ([edx*4 + base]); the compiler
// would fold that into absolute addresses and hide the table bases.
static __declspec(noinline) int32_t tableEntry(const int32_t* base, int32_t index) {
    return base[index];
}

// Walks a ring position back `eax` steps. The ring runs from [0x679ba4] up to the wrap limit
// [0x679b2c]: stepping below the lower bound [0x679ba4] re-enters at the top (position - lower +
// [0x679bb4]), and a re-entry above [0x679b2c] aborts. Starting at [0x679b3c], returns the position
// after eax steps, or -1 when eax is zero / above [0x007a8e1c], when the walk aborts, or when a
// step lands on the marker position [0x0088f624] (which also sets the flag byte [0x0088f5fc] to 0xff).
// ecx and edx are saved/restored; the flags are those of the last compare executed.
// FUNCTION: GP4 0x00512cc1
void __cdecl FUN_00512cc1(gp4::Regs* r) {
    const int32_t steps = static_cast<int32_t>(r->eax);
    uint32_t arith = gp4::flags::sub32(static_cast<uint32_t>(steps), GP4_GLOBAL(uint32_t, 0x007a8e1c));
    uint32_t result = 0xffffffffu;

    if (steps <= GP4_GLOBAL(int32_t, 0x007a8e1c)) {
        arith = gp4::flags::logic32(static_cast<uint32_t>(steps));
        if (steps != 0) {
            int32_t remaining = steps;
            int32_t pos = tableEntry(GP4_ARRAY(int32_t, 0x00679b34), 2);
            for (;;) {
                arith = gp4::flags::dec32(static_cast<uint32_t>(pos), arith);
                pos -= 1;
                const int32_t lower = tableEntry(GP4_ARRAY(int32_t, 0x00679b9c), 2);
                arith = gp4::flags::sub32(static_cast<uint32_t>(pos), static_cast<uint32_t>(lower));
                bool aborted = false;
                if (pos < lower) {
                    pos = static_cast<int32_t>(static_cast<uint32_t>(pos) - static_cast<uint32_t>(lower) +
                                               static_cast<uint32_t>(tableEntry(GP4_ARRAY(int32_t, 0x00679bac), 2)));
                    const int32_t upper = tableEntry(GP4_ARRAY(int32_t, 0x00679b24), 2);
                    arith = gp4::flags::sub32(static_cast<uint32_t>(pos), static_cast<uint32_t>(upper));
                    aborted = pos > upper;
                }
                if (aborted) {
                    break;
                }
                const int32_t marker = GP4_GLOBAL(int32_t, 0x0088f624);
                arith = gp4::flags::sub32(static_cast<uint32_t>(pos), static_cast<uint32_t>(marker));
                if (pos == marker) {
                    GP4_GLOBAL(uint8_t, 0x0088f5fc) = 0xffu;
                    break;
                }
                remaining -= 1;
                if (remaining == 0) {
                    result = static_cast<uint32_t>(pos);
                    break;
                }
            }
        }
    }

    r->eax = result;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00512cc1, FUN_00512cc1, "regs(eax:u32[0..12]) -> (eax, cf, pf, af, zf, sf, of); globals=layout=walk_00512cc1")
