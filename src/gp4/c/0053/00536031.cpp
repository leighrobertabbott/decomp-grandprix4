#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00536031
// `pushal ... popal`: matches the extension of the current file record [0x826d11] (name from +0, a dword
// of type bits at +0x4c) against the 4-byte extension table at 0x826dbe. If bit 18 of the type dword is set
// the record is accepted at once (CF clear). Otherwise the name is scanned to its '.' (no '.' before the NUL:
// CF set); then for every set bit of the type dword, shifted out from bit 0, the next table entry (4 bytes
// each, advancing for every bit including the clear ones) is compared with the three characters after the
// dot, both mapped through the character table 0x0054d747; a '?' in the entry accepts a character that
// 0x0052f208 classifies as a letter or digit. Three matching characters leave CF clear, running out of bits
// leaves CF set. Registers are restored by the popal; the flags are those of the last compare / test / or
// with CF replaced by clc / stc.
void __cdecl FUN_00536031(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00826d11)));
    uint32_t bits = GP4_FIELD(uint32_t, record, 0x4c);
    uint32_t arith = gp4::flags::logic32(bits & 0x40000u);
    if ((bits & 0x40000u) != 0u) {
        r->eflags = gp4::flags::merge(r->eflags, arith & ~gp4::flags::CF);
        return;
    }

    const uint8_t* cursor = record;
    bool dot = false;
    for (;;) {
        const uint32_t ch = *cursor++;
        arith = gp4::flags::sub8(ch, 0x2eu);
        if (ch == 0x2eu) {
            dot = true;
            break;
        }
        arith = gp4::flags::logic8(ch);
        if (ch == 0u) {
            break;
        }
    }
    if (!dot) {
        r->eflags = gp4::flags::merge(r->eflags, arith | gp4::flags::CF);
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(cursor));
    uint32_t entry = 0x00826dbeu;
    for (;;) {
        const uint32_t selected = bits & 1u;
        bits >>= 1;
        if (selected != 0u) {
            uint32_t index = 0u;
            bool matched = true;
            for (;;) {
                const uint8_t* wanted = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(entry + index));
                c.eax = (c.eax & 0xffffff00u) | *wanted;
                c.ebx = index;
                c.edx = bits;
                c.edi = entry;
                gp4::call_regs(0x0054d747, c);
                c.eax = (c.eax & 0xffff00ffu) | ((c.eax & 0xffu) << 8);
                c.eax = (c.eax & 0xffffff00u) | cursor[index];
                gp4::call_regs(0x0054d747, c);
                const uint32_t ah = (c.eax >> 8) & 0xffu;
                const uint32_t al = c.eax & 0xffu;
                arith = gp4::flags::sub8(ah, 0x3fu);
                bool miss;
                if (ah != 0x3fu) {
                    arith = gp4::flags::sub8(al, ah);
                    miss = (al != ah);
                } else {
                    gp4::call_regs(0x0052f208, c);
                    arith = c.eflags;
                    miss = (c.eflags & gp4::flags::CF) != 0u;
                }
                if (miss) {
                    matched = false;
                    break;
                }
                arith = gp4::flags::inc32(index, arith);
                ++index;
                arith = gp4::flags::sub32(index, 3u);
                if (index >= 3u) {
                    break;
                }
            }
            if (matched) {
                r->eflags = gp4::flags::merge(r->eflags, arith & ~gp4::flags::CF);
                return;
            }
        }
        entry += 4u;
        arith = gp4::flags::logic32(bits);
        if (bits == 0u) {
            r->eflags = gp4::flags::merge(r->eflags, arith | gp4::flags::CF);
            return;
        }
    }
}
GP4_IMPL(0x00536031, FUN_00536031, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=upper_00833d18+lower_00833e18+ext_00536031")
