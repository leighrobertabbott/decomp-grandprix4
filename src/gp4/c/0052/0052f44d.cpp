#include <gp4/gp4.h>
#include <gp4/flags.h>

// Drive-type style byte for the translated character `key` (table at 0x81e9a7).
static __declspec(noinline) uint32_t KindOf(uint32_t key) {
    return GP4_ARRAY(uint8_t, 0x0081e9a7)[key];
}

// FUNCTION: GP4 0x0052f44d
// Stores the current path buffer (0x104 bytes at 0x81e7f2) into the four 0x105-byte path records
// at 0x82474b, for the drive character in al. The character is translated through 0x54d755; only
// when its entry in the byte table 0x81e9a7 is 3, 4 or 5 does it act. It copies into the first
// record whose translated first byte equals the translated character, else into the first record
// that starts with a NUL, else it does nothing. ebx, ecx, esi, edi are saved and restored; eax is
// left as ah = translated character, al = the last byte the scan loaded or translated; the flags
// are those of the last flag writer (`cmp al, 4` when the table entry is not 3/4/5, `cmp ah, al`
// on a first-scan match, `cmp byte [edi], 0` on an empty-record match, else `dec ecx`).
void __cdecl FUN_0052f44d(gp4::Regs* r) {
    gp4::Regs first = *r;
    gp4::call_regs(0x0054d755, first);
    const uint32_t key = first.eax & 0xffu;
    const uint32_t kind = KindOf(key);
    uint32_t eax = (first.eax & 0xffff0000u) | (key << 8) | kind;

    uint32_t flags = gp4::flags::sub8(kind, 3u);
    bool scan = kind == 3u;
    if (!scan) {
        flags = gp4::flags::sub8(kind, 5u);
        scan = kind == 5u;
    }
    if (!scan) {
        flags = gp4::flags::sub8(kind, 4u);
        scan = kind == 4u;
    }

    if (scan) {
        uint8_t* record = GP4_ARRAY(uint8_t, 0x0082474b);
        uint32_t count = 4u;
        bool copy = false;
        do {
            gp4::Regs next = *r;
            next.eax = (eax & 0xffffff00u) | record[0];
            gp4::call_regs(0x0054d755, next);
            eax = next.eax;
            const uint32_t translated = eax & 0xffu;
            flags = gp4::flags::sub8(key, translated);
            if (key == translated) {
                copy = true;
                break;
            }
            record += 0x105;
            flags = gp4::flags::add32((uint32_t)(uintptr_t)record - 0x105u, 0x105u);
            flags = gp4::flags::dec32(count, flags);
            --count;
        } while (count != 0u);

        if (!copy) {
            record = GP4_ARRAY(uint8_t, 0x0082474b);
            count = 4u;
            do {
                flags = gp4::flags::sub8(record[0], 0u);
                if (record[0] == 0u) {
                    copy = true;
                    break;
                }
                record += 0x105;
                flags = gp4::flags::add32((uint32_t)(uintptr_t)record - 0x105u, 0x105u);
                flags = gp4::flags::dec32(count, flags);
                --count;
            } while (count != 0u);
        }

        if (copy) {
            const uint8_t* source = GP4_ARRAY(uint8_t, 0x0081e7f2);
            for (uint32_t i = 0u; i < 0x104u; ++i) {
                record[i] = source[i];
            }
        }
    }

    r->eax = eax;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0052f44d, FUN_0052f44d, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=drives_0082474b+xlat_00833e18+kind_0052f44d")
