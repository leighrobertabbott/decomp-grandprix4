#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of "shrd reg32, reg32, 14" under the pinned oracle model:
// CF = last bit shifted out, AF = 0, OF = bit31(concat >> 13 xor concat >> 14).
static inline uint32_t shrd14Flags(uint32_t low, uint32_t high) {
    const uint64_t concat = (static_cast<uint64_t>(high) << 32) | low;
    const uint32_t previous = static_cast<uint32_t>(concat >> 13);
    const uint32_t result = static_cast<uint32_t>(concat >> 14);
    return ((low >> 13) & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((previous ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// eax * edx >> 14 through the 64-bit product (imul edx ; shrd eax, edx, 14). `flags` receives the
// flags of the shrd.
static inline uint32_t scale14(uint32_t value, uint32_t factor, uint32_t* flags) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(value)) * static_cast<int64_t>(static_cast<int32_t>(factor));
    const uint32_t low = static_cast<uint32_t>(static_cast<uint64_t>(product));
    const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    *flags = shrd14Flags(low, high);
    return (low >> 14) | (high << 18);
}

static inline gp4::Regs selectorRegs(const gp4::Regs& base, uint32_t value) {
    gp4::Regs c = base;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = value;
    return c;
}

// The four selector routines each return their factor in edx. Separate helpers keep the four
// call sites distinct.
static __declspec(noinline) uint32_t factorFrom40ff6c(const gp4::Regs& base, uint32_t value) {
    gp4::Regs c = selectorRegs(base, value);
    gp4::call_regs(0x0040ff6c, c);
    return c.edx;
}
static __declspec(noinline) uint32_t factorFrom40ff8b(const gp4::Regs& base, uint32_t value) {
    gp4::Regs c = selectorRegs(base, value);
    gp4::call_regs(0x0040ff8b, c);
    return c.edx;
}
static __declspec(noinline) uint32_t factorFrom40ffaa(const gp4::Regs& base, uint32_t value) {
    gp4::Regs c = selectorRegs(base, value);
    gp4::call_regs(0x0040ffaa, c);
    return c.edx;
}
static __declspec(noinline) uint32_t factorFrom40fff2(const gp4::Regs& base, uint32_t value) {
    gp4::Regs c = selectorRegs(base, value);
    gp4::call_regs(0x0040fff2, c);
    return c.edx;
}

// When bit 0x20 of byte +0x81 is set, rescales the dword at +0x164 (bit 0x40 of byte +0x80) and the
// word at +0x16a (bit 0x80 of byte +0x80) by factors chosen by the 0x40ff6c / 0x40ff8b / 0x40ffaa /
// 0x40fff2 selectors (factor in edx, Q14). The block is wrapped in pushal/popal, so only the flags
// are visible: those of the last shrd, or of the last `test` (zero result: ZF = PF = 1) when no
// scaling ran after it.
// FUNCTION: GP4 0x00410047
void __cdecl FUN_00410047(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(r->esi);
    uint32_t flags;

    if ((body[0x81] & 0x20u) == 0u) {
        flags = 0x44u;
    } else if ((body[0x80] & 0x40u) != 0u) {
        const uint32_t dword = GP4_FIELD(uint32_t, body, 0x164);
        GP4_FIELD(uint32_t, body, 0x164) = scale14(dword, factorFrom40ff6c(*r, dword), &flags);

        const uint32_t old = GP4_FIELD(uint16_t, body, 0x16a);
        uint32_t word;
        if ((body[0x80] & 0x80u) != 0u) {
            word = scale14(old, factorFrom40fff2(*r, old), &flags);
        } else {
            word = scale14(old, factorFrom40ff8b(*r, old), &flags);
        }
        GP4_FIELD(uint16_t, body, 0x16a) = static_cast<uint16_t>(word);
    } else if ((body[0x80] & 0x80u) != 0u) {
        const uint32_t old = GP4_FIELD(uint16_t, body, 0x16a);
        GP4_FIELD(uint16_t, body, 0x16a) = static_cast<uint16_t>(scale14(old, factorFrom40ffaa(*r, old), &flags));
    } else {
        flags = 0x44u;
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00410047, FUN_00410047, "regs(esi:ptr[0x172]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
