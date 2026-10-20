#include <gp4/gp4.h>

// One of the three 0x20-byte entries at 0x00627a2c: seven sign-extended source words (the callee
// 0x0040cc58 rewrites +4..+0x18 when the first one is zero).
struct Unknown_0040cbf7_Entry {
    int32_t Unknown00;
    int32_t Unknown04;
    int32_t Unknown08;
    int32_t Unknown0C;
    int32_t Unknown10;
    int32_t Unknown14;
    int32_t Unknown18;
    uint8_t Unknown1C[4];
};

// FUNCTION: GP4 0x0040cbf7
// Copies a 64-byte block at esi into globals: three 16-byte groups of eight words, the first seven
// sign-extended into the 0x20-byte entries at 0x00627a2c (the eighth word is skipped); when an
// entry's first dword is zero, 0x0040cc58 is called with eax = 4 - (remaining groups) and edi at the
// entry. Then the last eight words go to 0x00627a8c. esi ends 0x40 past its start; eax is the last
// sign-extended word (or 3 when the last entry called the helper); ecx and edi are preserved; the
// flags are those of the last `add edi, 0x20` (0x627a6c + 0x20: all arithmetic flags clear).
void __cdecl FUN_0040cbf7(gp4::Regs* r) {
    const int16_t* src = reinterpret_cast<const int16_t*>(static_cast<uintptr_t>(r->esi));
    Unknown_0040cbf7_Entry* entry = reinterpret_cast<Unknown_0040cbf7_Entry*>(GP4_ARRAY(uint8_t, 0x00627a2c));

    uint32_t eax = 0;
    for (uint32_t remaining = 3; remaining != 0; --remaining) {
        entry->Unknown00 = src[0];
        entry->Unknown04 = src[1];
        entry->Unknown08 = src[2];
        entry->Unknown0C = src[3];
        entry->Unknown10 = src[4];
        entry->Unknown14 = src[5];
        entry->Unknown18 = src[6];
        eax = static_cast<uint32_t>(static_cast<int32_t>(src[6]));
        src += 8;
        if (entry->Unknown00 == 0) {
            gp4::Regs call = *r;
            call.eax = 4u - remaining;
            call.ecx = remaining;
            call.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(entry));
            gp4::call_regs(0x0040cc58, call);
            eax = 4u - remaining;
        }
        ++entry;
    }

    uint16_t* tail = reinterpret_cast<uint16_t*>(GP4_ARRAY(uint8_t, 0x00627a8c));
    for (int i = 0; i < 8; ++i) {
        tail[i] = static_cast<uint16_t>(src[i]);
    }

    r->eax = eax;
    r->esi = r->esi + 0x40u;
    r->eflags = r->eflags & ~0x8d5u;
}
GP4_IMPL(0x0040cbf7, FUN_0040cbf7, "regs(esi:ptr[0x40]:bytes) -> (eax, esi, cf, pf, af, zf, sf, of); globals=layout=scale_0062eade+trk_007ad894")
