#include <gp4/gp4.h>
#include <gp4/flags.h>

extern "C" void __cdecl __debugbreak(void);

// Text-edit key handler for the record at edi (+4 end pointer, +8 cursor, +0xc capacity end). When bit 0 of
// the byte [0x0082fd6a] is set it is cleared and the pending key [0x007ab00e] & 0x6ff (low byte translated
// through the table 0x00833d18 by 0x0054d747) is looked up in the 10-entry {key word, handler} table at
// 0x0054ddf6 (6 bytes each, read through cs:); a match calls the handler with the live registers. Otherwise
// a printable character (ah == 2, al == 0x0d or 0x20..0x7b/0x7d..) is inserted at the cursor when the text
// has more than 2 bytes of room: 0x0054e12a opens a one-byte gap, the byte [0x007ab00e] is stored there, the
// cursor is advanced and 0x0054ded0 re-wraps the text; with 2 or less bytes of room the original beeps with
// `int 0x10` (al = 7, ah = 0x0e), which cannot run in Windows.
// FUNCTION: GP4 0x0054dd7a
void __cdecl FUN_0054dd7a(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    const uint8_t pending = GP4_GLOBAL(uint8_t, 0x0082fd6a);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(pending & 1u));
    if ((pending & 1u) != 0u) {
        const uint8_t cleared = static_cast<uint8_t>(pending & 0xfeu);
        GP4_GLOBAL(uint8_t, 0x0082fd6a) = cleared;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(cleared));

        const uint32_t key = GP4_GLOBAL(uint16_t, 0x007ab00e) & 0x6ffu;
        c.eax = (c.eax & 0xffff0000u) | key;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic16(key));
        gp4::call_regs(0x0054d747, c);

        c.ebx = 0x0054ddf6u;
        c.ecx = 0xau;
        bool matched = false;
        for (;;) {
            const uint32_t entryKey = GP4_GLOBAL(uint16_t, c.ebx);
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub16(c.eax & 0xffffu, entryKey));
            if ((c.eax & 0xffffu) == entryKey) {
                matched = true;
                break;
            }
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(c.ebx, 6u));
            c.ebx += 6u;
            c.ecx -= 1u;
            if (c.ecx == 0u) {
                break;
            }
        }
        if (matched) {
            gp4::call_regs(GP4_GLOBAL(uint32_t, c.ebx + 2u), c);
        } else {
            const uint32_t high = (c.eax >> 8) & 0xffu;
            const uint32_t low = c.eax & 0xffu;
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(high, 2u));
            if (high == 2u) {
                bool insert = true;
                c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(low, 0xdu));
                if (low != 0xdu) {
                    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(low, 0x20u));
                    if (low < 0x20u) {
                        insert = false;
                    } else {
                        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(low, 0x7cu));
                        if (low == 0x7cu) {
                            insert = false;
                        }
                    }
                }
                if (insert) {
                    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.edi));
                    const uint32_t room = GP4_FIELD(uint32_t, record, 0xc) - GP4_FIELD(uint32_t, record, 4);
                    c.edx = room;
                    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(GP4_FIELD(uint32_t, record, 0xc), GP4_FIELD(uint32_t, record, 4)));
                    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(room, 2u));
                    if (room > 2u) {
                        c.edx = GP4_FIELD(uint32_t, record, 8);
                        c.eax = 1u;
                        gp4::call_regs(0x0054e12a, c);
                        c.eax = (c.eax & 0xffff0000u) | GP4_GLOBAL(uint16_t, 0x007ab00e);
                        GP4_FIELD(uint8_t, c.edx, 0) = static_cast<uint8_t>(c.eax);
                        const uint32_t cursor = GP4_FIELD(uint32_t, record, 8);
                        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::inc32(cursor, c.eflags));
                        GP4_FIELD(uint32_t, record, 8) = cursor + 1u;
                        gp4::call_regs(0x0054ded0, c);
                    } else {
                        c.eax = (c.eax & 0xffff0000u) | 0x0e07u;
                        __debugbreak();
                    }
                }
            }
        }
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054dd7a, FUN_0054dd7a, "regs(eax:u32, edi:ptr[0x10]:layout=rec_0054defe) -> (eax, ebx, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=text_0054defe+kbd_0054dd7a+nul_00833d18")
