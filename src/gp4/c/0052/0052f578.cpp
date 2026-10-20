#include <gp4/gp4.h>
#include <gp4/flags.h>

// Case-insensitive prefix match of the string at 0x00826e0e's NUL-separated name list
// ("com1", ...) against the string at 0x0082471a: CF = 1 when the string starts with a listed
// name followed by '.'. Registers are saved and restored (pushal/popal); only the flags remain:
// those of `cmp ah, 0` with CF set (match) or of the final `cmp esi, 0x00826e2e` with CF clear.
// FUNCTION: GP4 0x0052f578
static uint8_t foldChar(uint8_t c) {
    gp4::Regs fold = {};
    fold.eax = c;
    gp4::call_regs(0x0054d755, fold);
    return static_cast<uint8_t>(fold.eax);
}

void __cdecl FUN_0052f578(gp4::Regs* r) {
    const uint8_t* entry = GP4_ARRAY(const uint8_t, 0x00826e0e);
    const uint8_t* const listEnd = GP4_ARRAY(const uint8_t, 0x00826e2e);
    uint32_t arith;
    for (;;) {
        const uint8_t* name = entry;
        const uint8_t* text = GP4_ARRAY(const uint8_t, 0x0082471a);
        bool matched = false;
        for (;;) {
            const uint8_t folded = foldChar(*name);
            uint8_t ch = *text;
            if (ch == 0u) {
                break;
            }
            ch = foldChar(ch);
            name++;
            text++;
            if (ch == folded) {
                continue;
            }
            matched = ch == 0x2eu && folded == 0u;
            break;
        }
        if (matched) {
            arith = gp4::flags::sub8(0u, 0u) | gp4::flags::CF;
            break;
        }
        const uint8_t* scan = entry;
        uint8_t c;
        do {
            c = *scan++;
        } while (c != 0u);
        entry = scan;
        if (!(scan < listEnd)) {
            arith = gp4::flags::sub32(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(scan)),
                                      static_cast<uint32_t>(reinterpret_cast<uintptr_t>(listEnd)));
            break;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0052f578, FUN_0052f578, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=lower_00833e18+name_0082471a")
