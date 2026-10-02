#include <gp4/gp4.h>

struct Unknown_0041011a_Source {
    uint8_t Unknown00[0x10];
    uint8_t Unknown10[1];
};

static inline uint32_t compareFlags8(uint8_t left, uint8_t right) {
    const uint8_t result = static_cast<uint8_t>(left - right);
    uint32_t parity = result;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (left < right ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result) & 0x80u) != 0u) ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0041011a
void __cdecl FUN_0041011a(gp4::Regs* r) {
    const uint32_t index = r->ecx;
    const uint8_t* source = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint8_t* dest = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    const int32_t sample = static_cast<int32_t>(source[index + 0x10u]);
    const int32_t scale = GP4_GLOBAL(int32_t, 0x00601d5c);
    const int64_t product = static_cast<int64_t>(sample) * static_cast<int64_t>(scale);

    // shrd eax, edx, 14: the 64-bit product shifted right by 14, low dword.
    int32_t value = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14));
    const int32_t limit = GP4_ARRAY(int32_t, 0x00601e40)[index];
    if (value > limit) {
        value = limit;
    }

    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x00601f1f);
    if (mode != 0xffu) {
        value = 0;
    }
    *reinterpret_cast<int32_t*>(dest + index * 4u + 0x35cu) = value;

    r->eax = static_cast<uint32_t>(value);
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | compareFlags8(mode, 0xffu);
}
GP4_IMPL(0x0041011a, FUN_0041011a, "regs(ebx:ptr[0x40]:bytes, ecx:u32[0..15], esi:ptr[0x3b0]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
