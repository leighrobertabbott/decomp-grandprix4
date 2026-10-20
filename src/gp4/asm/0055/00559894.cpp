#include <gp4/gp4.h>
#include <gp4/flags.h>

// Squared distance between two 0x160-byte ring records in the (+4, +8) word pair: each coordinate
// difference is taken in 16 bits (`sub ax, [esi+k]`), squared with the signed 16-bit `imul ax`, and the
// two 32-bit products are summed. eax = the sum, flags those of the final add. edx keeps its upper half
// and holds the high word of the second product in dx; ebx is saved and restored.
// FUNCTION: GP4 0x00559894
void __cdecl FUN_00559894(gp4::Regs* r) {
    uint8_t* a = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    uint8_t* b = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    const int16_t dx1 = static_cast<int16_t>(static_cast<uint16_t>(GP4_FIELD(int16_t, a, 4) - GP4_FIELD(int16_t, b, 4)));
    const uint32_t first = static_cast<uint32_t>(static_cast<int32_t>(dx1) * static_cast<int32_t>(dx1));

    const int16_t dx2 = static_cast<int16_t>(static_cast<uint16_t>(GP4_FIELD(int16_t, a, 8) - GP4_FIELD(int16_t, b, 8)));
    const uint32_t second = static_cast<uint32_t>(static_cast<int32_t>(dx2) * static_cast<int32_t>(dx2));

    r->edx = (r->edx & 0xffff0000u) | (second >> 16);
    r->eax = second + first;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(second, first));
}
GP4_IMPL(0x00559894, FUN_00559894, "regs(edx:u32, edi:ptr[0x10]:bytes, esi:ptr[0x10]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of)")
