#include <gp4/gp4.h>
#include <gp4/flags.h>

// Compares the live menu [0x0082fd34] against the saved snapshot buffer [0x0082fd38] and reports
// whether anything differs (CF set when at least one byte differs).
//  1. The extra-field table at header+0x28 (6-byte entries: word length, dword pointer; a zero
//     length ends it) is compared byte by byte (cmpsb, edi advancing through the snapshot).
//  2. Every item of the list that starts at [header] (link +0, null ends) whose byte +0x1e has
//     bit 0 set has its value at the pointer +0xe compared with the snapshot: kind (byte +0xd & 3)
//     0 = byte, 1 = word, 2 = dword, 3 = NUL-terminated string (the terminator is compared too).
//     edx counts the differences. (The original's table also has a fifth, unreachable slot that
//     calls the bare routine 0x54e232: the kind is masked to 0..3.)
// Registers: ebx = 0 and edi = end of the snapshot data; esi/ecx/eax keep the values their last
// cmps / loop / dispatch left. Flags: `cmp edx, 1; cmc`.

// FUNCTION: GP4 0x0053de6f
void __cdecl FUN_0053de6f(gp4::Regs* r) {
    uint32_t differences = 0u;
    uint32_t snapshot = GP4_GLOBAL(uint32_t, 0x0082fd38);
    uint32_t esi = r->esi;
    uint32_t ecx = r->ecx;
    uint32_t eax = r->eax;

    uint32_t entry = GP4_GLOBAL(uint32_t, 0x0082fd34) + 0x28u;
    while (*reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(entry)) != 0u) {
        ecx = *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(entry));
        esi = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(entry + 2u));
        do {
            if (*reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(esi)) !=
                *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(snapshot))) {
                ++differences;
            }
            ++esi;
            ++snapshot;
            --ecx;
        } while (ecx != 0u);
        entry += 6u;
    }

    uint32_t item = GP4_GLOBAL(uint32_t, 0x0082fd34);
    for (;;) {
        item = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(item));
        if (item == 0u) {
            break;
        }
        if ((*reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(item + 0x1eu)) & 1u) == 0u) {
            continue;
        }
        esi = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(item + 0xeu));
        uint32_t kind = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(item + 0xdu)) & 3u;
        if (kind >= 5u) {
            kind = 4u;
        }
        eax = (kind << 2) + 0x0053ded0u;
        switch (kind) {
        case 0:
            if (*reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(esi)) !=
                *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(snapshot))) {
                ++differences;
            }
            esi += 1u;
            snapshot += 1u;
            break;
        case 1:
            if (*reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(esi)) !=
                *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(snapshot))) {
                ++differences;
            }
            esi += 2u;
            snapshot += 2u;
            break;
        case 2:
            if (*reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(esi)) !=
                *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(snapshot))) {
                ++differences;
            }
            esi += 4u;
            snapshot += 4u;
            break;
        case 3: {
            uint32_t al;
            do {
                al = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(esi));
                if (al != *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(snapshot))) {
                    ++differences;
                }
                ++esi;
                ++snapshot;
            } while (al != 0u);
            eax &= 0xffffff00u;
            break;
        }
        default: {
            gp4::Regs e = *r;
            e.esi = 0x0053df05u;
            gp4::call_regs(0x0054e232, e);
            break;
        }
        }
    }

    r->eax = eax;
    r->ebx = 0u;
    r->ecx = ecx;
    r->edx = differences;
    r->esi = esi;
    r->edi = snapshot;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(differences, 1u) ^ gp4::flags::CF);
}
GP4_IMPL(0x0053de6f, FUN_0053de6f, "regs(eax:u32, ecx:u32, esi:u32) -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=menu_0084300a+mitems_0053ddbb+snap_0053de6f")
