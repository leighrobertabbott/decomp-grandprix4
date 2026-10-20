#include <gp4/gp4.h>

struct Unknown_0041a6f5_Record {
    uint32_t Unknown00;
    uint16_t Unknown04;
    uint8_t Unknown06;
    uint8_t Unknown07;
    uint8_t Unknown08;
    uint8_t Unknown09;
    uint16_t Unknown0A;
    uint32_t Unknown0C;
    uint32_t Unknown10;
};

// FUNCTION: GP4 0x0041a6f5
void __cdecl FUN_0041a6f5(gp4::Regs* r) {
    Unknown_0041a6f5_Record* rec = reinterpret_cast<Unknown_0041a6f5_Record*>(static_cast<uintptr_t>(r->ebx));

    rec->Unknown00 = 0u;

    // mov ax, [0x868558] ; shl ax, 4 : the only flag writer (16-bit operand, count 4)
    const uint32_t word = GP4_GLOBAL(uint16_t, 0x00868558);
    const uint32_t shifted = (word << 4) & 0xffffu;
    rec->Unknown04 = static_cast<uint16_t>(shifted);

    rec->Unknown06 = static_cast<uint8_t>(GP4_GLOBAL(uint32_t, 0x007ad894));
    rec->Unknown07 = GP4_GLOBAL(uint8_t, 0x00623ad8);
    rec->Unknown08 = static_cast<uint8_t>(GP4_GLOBAL(uint16_t, 0x007c2e96));
    rec->Unknown09 = 0u;
    rec->Unknown0A = GP4_GLOBAL(uint16_t, 0x0066d868);
    const uint32_t last = GP4_GLOBAL(uint32_t, 0x0066d864);
    rec->Unknown0C = last;
    rec->Unknown10 = 0u;

    uint32_t parity = shifted & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    const uint32_t cf = (word >> 12) & 1u;
    const uint32_t of = (((word << 3) ^ shifted) & 0x8000u) != 0u ? 0x800u : 0u;
    const uint32_t flags = cf
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (shifted == 0u ? 0x40u : 0u)
        | ((shifted & 0x8000u) != 0u ? 0x80u : 0u)
        | of;

    r->eax = last;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041a6f5, FUN_0041a6f5, "regs(ebx:ptr[0x20]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
