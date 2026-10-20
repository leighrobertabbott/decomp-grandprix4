#include <gp4/gp4.h>
#include <gp4/flags.h>

// Track loader record reader for the 7-word record at esi in mode 2 ([0x0086857c] == 2); any other
// mode just skips the record. The sign of word 0 picks the slot (negative: 0, else 1). Words 1:2 form
// a dword stored at 0x852cac[slot]; words 3:4 form a dword for 0x852cb4[slot], replaced when the
// first dword is below 0x555 by [edi + 0xec] -/+ 0x40000000 (slot 0 / slot 1). Words 5 and 6,
// sign-extended and shifted left by 6, go to 0x852cbc[slot] and 0x852cc4[slot]. In mode 2 eax ends
// as the last value, ecx as the slot and edx as the first dword. Both ways end with esi += 0xe and
// the flags of that add.
// FUNCTION: GP4 0x00557b74
void __cdecl FUN_00557b74(gp4::Regs* r) {
    const uint32_t source = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) == 2u) {
        const int16_t* words = reinterpret_cast<const int16_t*>(static_cast<uintptr_t>(source));
        const uint32_t slot = (words[0] >= 0) ? 1u : 0u;
        const uint32_t first = (static_cast<uint32_t>(static_cast<uint16_t>(words[1])) << 16) |
                               static_cast<uint16_t>(words[2]);
        GP4_ARRAY(uint32_t, 0x00852cac)[slot] = first;
        uint32_t second = (static_cast<uint32_t>(static_cast<uint16_t>(words[3])) << 16) |
                          static_cast<uint16_t>(words[4]);
        if (first < 0x555u) {
            const uint32_t base = GP4_FIELD(uint32_t, r->edi, 0xec);
            if (slot == 0u) {
                second = base - 0x40000000u;
            } else {
                second = base + 0x40000000u;
            }
        }
        GP4_ARRAY(uint32_t, 0x00852cb4)[slot] = second;
        GP4_ARRAY(uint32_t, 0x00852cbc)[slot] =
            static_cast<uint32_t>(static_cast<int32_t>(words[5])) << 6;
        const uint32_t last = static_cast<uint32_t>(static_cast<int32_t>(words[6])) << 6;
        GP4_ARRAY(uint32_t, 0x00852cc4)[slot] = last;
        r->eax = last;
        r->ecx = slot;
        r->edx = first;
    }
    r->esi = source + 0xeu;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(source, 0xeu));
}
GP4_IMPL(0x00557b74, FUN_00557b74, "regs(eax:u32, ecx:u32, edx:u32, esi:ptr[0x10]:u16{0,0,1,2,0x100,0x554,0x555,0x8000,0xffff}, edi:ptr[0xf8]:bytes) -> (eax, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_0086857c")
