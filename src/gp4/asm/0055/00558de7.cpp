#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00558de7
// Linear search of the table of 0x60-byte entries at 0x9559f8 (entry count in the dword at
// 0x9559f0) for one equal to the 0x60-byte record at esi. Returns eax = index of the first equal
// entry with CF clear, or eax = entry count with CF set when none matches (the loop always tests
// entry 0 first, so a zero count never terminates). Also leaves edx = esi and ecx = dwords left
// uncompared by the final `repe cmpsd`. esi, edi and ebp are saved and restored. Flags are those
// of the last cmpsd (found) or of `cmp eax, count` (not found), with CF forced by clc / stc.
void __cdecl FUN_00558de7(gp4::Regs* r) {
    const uint32_t* probe = (const uint32_t*)r->esi;
    const uint32_t entryCount = GP4_GLOBAL(uint32_t, 0x009559f0);

    uint32_t index = 0u;
    uint32_t entry = 0x009559f8u;
    uint32_t left;
    uint32_t flags;

    for (;;) {
        const uint32_t* a = probe;
        const uint32_t* b = (const uint32_t*)entry;
        bool equal;
        left = 0x18u;
        do {
            flags = gp4::flags::sub32(*a, *b);
            equal = (*a == *b);
            ++a;
            ++b;
            --left;
        } while (equal && left != 0u);

        if (equal) {
            flags &= ~gp4::flags::CF;   // clc
            break;
        }
        ++index;
        entry += 0x60u;
        flags = gp4::flags::sub32(index, entryCount);
        if (index == entryCount) {
            flags |= gp4::flags::CF;    // stc
            break;
        }
    }

    r->eax = index;
    r->ecx = left;
    r->edx = r->esi;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00558de7, FUN_00558de7, "regs(esi:rec[0x9559f8,0x60,0..5]) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
