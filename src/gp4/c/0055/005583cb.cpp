#include <gp4/gp4.h>
#include <gp4/flags.h>

extern "C" void __cdecl __debugbreak(void);

// Difference of two table addresses (the original computes 0x961b92 - 0x9619f8 at run time).
static __declspec(noinline) uint32_t TableSpan(uint32_t end, uint32_t base) {
    return end - base;
}

// Address of the 10-byte entry at byte offset `off` of the table at 0x00961b9c.
static __declspec(noinline) uint32_t EntryAddr(uint32_t off) {
    return 0x00961b9cu + off;
}

// Track loader record reader for the 7-word record at esi in mode 1 ([0x0086857c] == 1); any other
// mode just skips the record. Word 0 (low byte) selects a 10-byte entry (offset = byte * 10, which
// must stay below 0x19a else int3) of the table at 0x00961b9c, which receives: byte +4 = word 1,
// word +0 = word 2, byte +5 = word 3, byte +6 = word 4 clamped to 1..255 (signed), byte +7 = word 5,
// byte +2 = (word 6 & 0x7f) | 0x80, byte +3 = (word 6 >> 8) | 3. In mode 1 eax ends as the entry
// address (upper half) with ax = the last two bytes written, edx as the table span 0x19a. Both ways
// end with esi += 0xe and the flags of that add.
// FUNCTION: GP4 0x005583cb
void __cdecl FUN_005583cb(gp4::Regs* r) {
    const uint32_t source = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) == 1u) {
        const uint16_t* words = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(source));
        const uint32_t span = TableSpan(0x00961b92u, 0x009619f8u);
        const uint32_t offset = (words[0] & 0xffu) * 10u;
        if (offset >= span) {
            __debugbreak();
        }
        const uint32_t entry = EntryAddr(offset);
        GP4_FIELD(uint8_t, entry, 4) = static_cast<uint8_t>(words[1]);
        GP4_FIELD(uint16_t, entry, 0) = words[2];
        GP4_FIELD(uint8_t, entry, 5) = static_cast<uint8_t>(words[3]);
        int16_t level = static_cast<int16_t>(words[4]);
        if (level > 0xff) {
            level = 0xff;
        }
        if (level < 1) {
            level = 1;
        }
        GP4_FIELD(uint8_t, entry, 6) = static_cast<uint8_t>(level);
        GP4_FIELD(uint8_t, entry, 7) = static_cast<uint8_t>(words[5]);
        const uint16_t flags_word = words[6];
        const uint8_t low = static_cast<uint8_t>((flags_word & 0x7fu) | 0x80u);
        const uint8_t high = static_cast<uint8_t>((flags_word >> 8) | 3u);
        GP4_FIELD(uint8_t, entry, 2) = low;
        GP4_FIELD(uint8_t, entry, 3) = high;
        r->eax = (entry & 0xffff0000u) | (static_cast<uint32_t>(high) << 8) | low;
        r->edx = span;
    }
    r->esi = source + 0xeu;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(source, 0xeu));
}
GP4_IMPL(0x005583cb, FUN_005583cb, "regs(eax:u32, edx:u32, esi:ptr[0x10]:u16{=0,1,5,0x28,0x100,0x101,0x7f00,0x8000,0xff00}) -> (eax, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_0086857c")
