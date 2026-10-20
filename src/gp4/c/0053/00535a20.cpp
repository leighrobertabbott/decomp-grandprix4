#include <gp4/gp4.h>
#include <gp4/flags.h>

static inline uint16_t rd16(uint32_t addr) { return *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(addr)); }
static inline uint8_t rd8(uint32_t addr) { return *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(addr)); }

// `repe cmpsb` with ecx already loaded: compares [esi] with [edi] byte by byte, stopping at the
// first difference; esi, edi advance and ecx counts down for every compared byte, `arith` holds the
// flags of the last compare. Returns true when every byte was equal.
static bool RepeCmpsb(gp4::Regs& c, uint32_t& arith) {
    while (c.ecx != 0u) {
        const uint8_t a = rd8(c.esi);
        const uint8_t b = rd8(c.edi);
        arith = gp4::flags::sub8(a, b);
        c.esi += 1u;
        c.edi += 1u;
        c.ecx -= 1u;
        if (a != b) {
            return false;
        }
    }
    return true;
}

// Compares the 15-byte stamp at 0x822300 (word, word, byte, 5 bytes, 4 bytes) with the copies in
// the saved-state buffer [0x007a93e0] located by the helper routines 0x535a93/0x535aa4/0x535ab7/
// 0x535ae3/0x535ac8. Returns true ("clc") when every step matches - or when the second word is
// negative ([0x007a93b7] = 0xff then) or [0x007aafe8] is 3 or 4 - and false ("stc") at the first
// difference. `arith` receives the flags of the last compare.
static bool Walk(gp4::Regs& c, uint32_t& arith) {
    c.esi = 0x00822300u;
    gp4::call_regs(0x00535a93, c);
    c.edi = c.eax;
    {
        const uint16_t a = rd16(c.esi);
        const uint16_t b = rd16(c.edi);
        arith = gp4::flags::sub16(a, b);
        c.esi += 2u;
        c.edi += 2u;
        if (a != b) {
            return false;
        }
    }

    gp4::call_regs(0x00535aa4, c);
    c.edi = c.eax;
    const uint16_t word = rd16(c.esi);
    c.eax = (c.eax & 0xffff0000u) | word;
    {
        const uint16_t b = rd16(c.edi);
        arith = gp4::flags::sub16(word, b);
        c.esi += 2u;
        c.edi += 2u;
        if (word != b) {
            return false;
        }
    }
    arith = gp4::flags::logic16(word);
    if (word & 0x8000u) {
        GP4_GLOBAL(uint8_t, 0x007a93b7) = 0xffu;
        return true;
    }

    gp4::call_regs(0x00535ab7, c);
    c.edi = c.eax;
    const uint8_t first = rd8(c.esi);
    c.esi += 1u;
    const uint8_t diff = static_cast<uint8_t>(first ^ rd8(c.edi));
    c.eax = (c.eax & 0xffffff00u) | diff;
    arith = gp4::flags::sub8(diff, 1u);
    if (diff != 1u) {
        return false;
    }

    arith = gp4::flags::sub16(GP4_GLOBAL(uint16_t, 0x007aafe8), 3u);
    if (GP4_GLOBAL(uint16_t, 0x007aafe8) == 3u) {
        return true;
    }
    arith = gp4::flags::sub16(GP4_GLOBAL(uint16_t, 0x007aafe8), 4u);
    if (GP4_GLOBAL(uint16_t, 0x007aafe8) == 4u) {
        return true;
    }

    gp4::call_regs(0x00535ae3, c);
    c.edi = c.eax;
    c.ecx = 5u;
    if (!RepeCmpsb(c, arith)) {
        return false;
    }

    gp4::call_regs(0x00535ac8, c);
    c.edi = c.eax;
    c.ecx = 4u;
    return RepeCmpsb(c, arith);
}

// FUNCTION: GP4 0x00535a20
// Validates the stamp at 0x822300 against the saved-state copy; CF clear = valid, CF set = differs.
// The other flags are those of the last compare executed; eax, ecx, esi and edi keep the values
// the helper routines and string compares leave in them.
void __cdecl FUN_00535a20(gp4::Regs* r) {
    gp4::Regs c = *r;
    uint32_t arith = 0u;
    const bool match = Walk(c, arith);
    if (match) {
        arith &= ~gp4::CF;
    } else {
        arith |= gp4::CF;
    }
    *r = c;
    r->eflags = gp4::flags::merge(c.eflags, arith);
}
GP4_IMPL(0x00535a20, FUN_00535a20, "regs() -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=save_005359c8+stamp_00535a20")
