#include <gp4/gp4.h>
#include <gp4/flags.h>

// Fills the 14-entry point rows (x at 0x871cb0, height at 0x872ab0, y at 0x8738b0; byte offset esi =
// 0 or 0x38 selects the row) for the segment record edi. Entries 0..9 come from 0x00551227 (esi = 2 *
// entry, eax = -1 for entries 0..4, else 0): x/y from 0x934df0/0x934df4, height from its ecx.
// Entries 10 and 11 are the record's origin words +4/+8 << 6 plus the offset words +0xb8/+0xc0 and
// +0xba/+0xc2 << 3, entries 12 and 13 the vectors of 0x00553aef / 0x00553b09 (0x934df0/0x934df4,
// each sar 5 in place); entries 10..13 take the height word +6 << 3. Registers leave as the last
// entry left them (ebx = 13, ecx = height, eax/edx = the scaled vector); flags of the final shl.

static inline int32_t Word(const uint8_t* record, uint32_t offset) {
    return *reinterpret_cast<const int16_t*>(record + offset);
}

// Flags of `shl reg, 3` (oracle model: CF = last bit out, AF = 0, OF = bit31(v<<2 ^ v<<3)).
static inline uint32_t Shl3Flags(uint32_t value) {
    const uint32_t result = value << 3;
    const uint32_t before = value << 2;
    return ((value >> 29) & 1u) | gp4::flags::logic32(result) |
           ((((before ^ result) >> 31) & 1u) != 0u ? gp4::flags::OF : 0u);
}

static inline void Store(uint32_t base, uint32_t entry, uint32_t x, uint32_t height, uint32_t y) {
    GP4_GLOBAL(uint32_t, 0x00871cb0 + base + entry * 4u) = x;
    GP4_GLOBAL(uint32_t, 0x00872ab0 + base + entry * 4u) = height;
    GP4_GLOBAL(uint32_t, 0x008738b0 + base + entry * 4u) = y;
}

// After 0x00553aef / 0x00553b09: sar 5 the vector in place, store it with the height word +6 << 3.
// Returns the flags of that `shl ecx, 3`.
static inline uint32_t ScaledVector(gp4::Regs& c, uint32_t base, const uint8_t* record, uint32_t entry) {
    const uint32_t x = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00934df0) >> 5);
    GP4_GLOBAL(uint32_t, 0x00934df0) = x;
    const uint32_t y = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00934df4) >> 5);
    GP4_GLOBAL(uint32_t, 0x00934df4) = y;
    const uint32_t word = static_cast<uint32_t>(Word(record, 6u));
    const uint32_t height = word << 3;
    c.eax = x;
    c.edx = y;
    c.ecx = height;
    Store(base, entry, x, height, y);
    return Shl3Flags(word);
}

// FUNCTION: GP4 0x00554ab1
void __cdecl FUN_00554ab1(gp4::Regs* r) {
    const uint32_t base = r->esi;
    const uint8_t* const record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->edi));
    gp4::Regs c = *r;

    for (uint32_t entry = 0u; entry != 10u; ++entry) {
        c.eax = static_cast<int32_t>(entry) <= 4 ? 0xffffffffu : 0u;
        c.ebx = entry;
        c.esi = entry * 2u;
        gp4::call_regs(0x00551227, c);
        c.esi = base;
        c.ebx = entry;
        c.eax = GP4_GLOBAL(uint32_t, 0x00934df4);
        Store(base, entry, GP4_GLOBAL(uint32_t, 0x00934df0), c.ecx, c.eax);
    }

    for (uint32_t entry = 10u; entry != 12u; ++entry) {
        const uint32_t k = (entry - 10u) * 2u;
        const uint32_t x = (static_cast<uint32_t>(Word(record, 0xb8u + k)) << 3) +
                           (static_cast<uint32_t>(Word(record, 4u)) << 6);
        const uint32_t y = (static_cast<uint32_t>(Word(record, 0xc0u + k)) << 3) +
                           (static_cast<uint32_t>(Word(record, 8u)) << 6);
        const uint32_t height = static_cast<uint32_t>(Word(record, 6u)) << 3;
        c.ebx = entry;
        c.eax = x;
        c.edx = y;
        c.ecx = height;
        Store(base, entry, x, height, y);
    }

    c.ebx = 12u;
    gp4::call_regs(0x00553aef, c);
    ScaledVector(c, base, record, 12u);
    c.ebx = 13u;
    gp4::call_regs(0x00553b09, c);
    const uint32_t flags = ScaledVector(c, base, record, 13u);

    c.eflags = gp4::flags::merge(c.eflags, flags);
    *r = c;
}
GP4_IMPL(0x00554ab1, FUN_00554ab1, "regs(esi:u32{=0,0x38}, edi:rec[0x8943a0,0x160,0..7]) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=geom_008943a0+span_00551227+w6_008943a0+words_008943a0")
