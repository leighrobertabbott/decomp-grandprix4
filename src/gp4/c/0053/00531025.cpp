#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00531025
// Walks the first [0x007a97a4] + 1 device records (0x3c bytes each, from 0x00827160) in step with
// two dword arrays: 0x007ad870 (current value) and 0x007ad880 (replacement). For every record
// whose byte +5 is not 2 it searches the record's eight dwords +0x14..+0x30 for the current value;
// when none matches (index 7 reached) the current value is replaced by the array 0x007ad880
// entry. Register-interface routine: eax/edx keep their values from the last record that was
// searched (or the caller's when none was), ebx/esi/edi end one past the last record/entries,
// ecx is 0 and the flags are those of the final `dec ecx` after `add edi, 4`.
void __cdecl FUN_00531025(gp4::Regs* r) {
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t esi = 0x007ad880u;
    uint32_t edi = 0x007ad870u;
    uint32_t ebx = 0x00827160u;
    uint32_t ecx = GP4_GLOBAL(uint32_t, 0x007a97a4) + 1u;
    uint32_t flags = 0u;
    uint32_t before;
    do {
        uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(ebx));
        if (GP4_FIELD(uint8_t, rec, 5) != 2u) {
            uint32_t* current = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(edi));
            edx = *current;
            eax = 0u;
            for (;;) {
                if (edx == GP4_FIELD(uint32_t, rec, 0x14 + eax * 4u)) {
                    break;
                }
                if (!(eax < 7u)) {
                    break;
                }
                eax += 1u;
            }
            if (eax == 7u) {
                eax = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(esi));
                *current = eax;
            }
        }
        ebx += 0x3cu;
        esi += 4u;
        flags = gp4::flags::add32(edi, 4u);
        edi += 4u;
        before = ecx;
        ecx -= 1u;
    } while (ecx != 0u);

    r->eax = eax;
    r->edx = edx;
    r->ebx = ebx;
    r->ecx = ecx;
    r->esi = esi;
    r->edi = edi;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(before, flags));
}
GP4_IMPL(0x00531025, FUN_00531025, "regs(eax:u32, edx:u32) -> (eax, edx, ebx, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=count_007a97a4")
