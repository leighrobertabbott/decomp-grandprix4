#include <gp4/gp4.h>
#include <gp4/flags.h>

// Keyword match helper used by the table scan 0x0052b2c0: compares the zero-terminated pattern at
// edi with the text at esi. The match succeeds (CF = 0, esi and edi advanced past the pattern) when
// the whole pattern is found and the next text character is not a lowercase letter; otherwise
// CF = 1 and esi is restored to its entry value (edi stays where the comparison stopped). eax and
// ebx are pushed and popped. The other flags are those of the last compare: pattern byte vs text byte
// on a mismatch, else text byte vs 0x61 or 0x7a; clc/stc replace CF.
// FUNCTION: GP4 0x0052b2db
void __cdecl FUN_0052b2db(gp4::Regs* r) {
    const uint32_t text_start = r->esi;
    const uint8_t* pattern = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->edi));
    const uint8_t* text = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t flags;
    bool failed;
    for (;;) {
        const uint8_t expected = *pattern;
        if (expected == 0u) {
            const uint8_t next = *text;
            flags = gp4::flags::sub8(next, 0x61u);
            if (next < 0x61u) {
                failed = false;
                break;
            }
            flags = gp4::flags::sub8(next, 0x7au);
            failed = !(next > 0x7au);
            break;
        }
        flags = gp4::flags::sub8(expected, *text);
        if (expected != *text) {
            failed = true;
            break;
        }
        ++pattern;
        ++text;
    }
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(pattern));
    r->esi = failed ? text_start : static_cast<uint32_t>(reinterpret_cast<uintptr_t>(text));
    r->eflags = gp4::flags::merge(r->eflags, (flags & ~1u) | (failed ? 1u : 0u));
}
GP4_IMPL(0x0052b2db, FUN_0052b2db, "regs(edi:ptr[0x40]:u8{0x61,0x62,0,0x61,0x62}, esi:ptr[0x40]:u8{0x61,0x62,0,0x20,0x7b,0x61,0x62}) -> (edi, esi, cf, pf, af, zf, sf, of)")
