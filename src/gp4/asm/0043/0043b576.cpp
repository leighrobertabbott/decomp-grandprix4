#include <gp4/gp4.h>

struct Unknown_006043f0 {
    uint8_t Unknown00[0x7d];
    uint8_t Position7D;   // byte offset 2*i of the record in the car order list
};

// Rotates the 22-word car order list at 0x66fe3c: the list is first copied to the buffer at
// [0x8914f8]; then, starting at the copy's byte offset Position7D of the record at [0x6043f0]
// (wrapping at the buffer end), each word (a record offset k*0x41c, or a negative terminator) is
// written back to the list in order, and its record's +0x7d byte (0x66ff6d + word) is set to the
// new byte offset 2*i. eax = end of the copy, ebx = end of the list, ecx = 0x2c, dx = 0xffff
// (upper edx kept), esi = last word (sign extended), edi = read cursor; the flags are those of the
// final "sub dx, 1" (CF AF PF SF set).
// FUNCTION: GP4 0x0043b576
void __cdecl FUN_0043b576(gp4::Regs* r) {
    uint16_t* const list = GP4_ARRAY(uint16_t, 0x0066fe3c);
    uint8_t* const recordPositions = GP4_ARRAY(uint8_t, 0x0066ff6d);
    const uint32_t buffer = GP4_GLOBAL(uint32_t, 0x008914f8);

    // rep movsw: 22 words list -> buffer
    for (uint32_t i = 0; i < 0x16u; ++i) {
        reinterpret_cast<uint16_t*>(buffer)[i] = list[i];
    }
    const uint32_t bufferEnd = buffer + 0x2cu;

    const Unknown_006043f0* current = reinterpret_cast<const Unknown_006043f0*>(GP4_GLOBAL(uint32_t, 0x006043f0));
    uint32_t cursor = buffer + current->Position7D;
    uint32_t position = 0;
    int32_t entry = 0;
    uint16_t* out = list;
    for (int32_t remaining = 0x15; remaining >= 0; --remaining) {
        entry = *reinterpret_cast<const int16_t*>(cursor);
        cursor += 2;
        recordPositions[entry] = static_cast<uint8_t>(position);
        *out++ = static_cast<uint16_t>(entry);
        position += 2;
        if (cursor == bufferEnd) {
            cursor = buffer;
        }
    }

    r->eax = bufferEnd;
    r->ebx = 0x0066fe3cu + 0x2cu;
    r->ecx = 0x2cu;
    r->edx = (r->edx & 0xffff0000u) | 0xffffu;
    r->esi = static_cast<uint32_t>(entry);
    r->edi = cursor;
    // sub dx, 1 from 0: CF PF AF SF set, ZF OF clear
    r->eflags = (r->eflags & ~0x8d5u) | 0x95u;
}
GP4_IMPL(0x0043b576, FUN_0043b576, "regs(edx:u32) -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=cars_0066fef0")
