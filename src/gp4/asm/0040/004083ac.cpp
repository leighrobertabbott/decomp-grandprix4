#include <gp4/gp4.h>

struct Unknown_0066fef0 {
    uint8_t Unknown00[0x68];
    uint16_t Unknown68;
    uint8_t Unknown6A[0x10];
    uint8_t Flags7A;
    uint8_t Unknown7B[2];
    uint8_t Index7D;
};

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x004083ac
void __cdecl FUN_004083ac(gp4::Regs* r) {
    const uint32_t self = r->esi;
    const Unknown_0066fef0* record = reinterpret_cast<const Unknown_0066fef0*>(static_cast<uintptr_t>(self));
    const int16_t* table = GP4_ARRAY(const int16_t, 0x0066fe3c);
    const int16_t* cursor = reinterpret_cast<const int16_t*>(reinterpret_cast<const uint8_t*>(table) + record->Index7D);

    uint32_t eax;
    uint32_t flags;
    for (;;) {
        --cursor;
        const int16_t offset = *cursor;
        if (offset < 0) {
            // or ax,ax leaves SF, PF of the low byte; stc sets CF
            eax = self;
            flags = 0x81u | parityFlag8(static_cast<uint16_t>(offset));
            break;
        }
        const uint32_t other = 0x0066fef0u + static_cast<uint32_t>(static_cast<int32_t>(offset));
        if (other == self) {
            // cmp ebx,esi equal: ZF, PF; stc sets CF
            eax = self;
            flags = 0x45u;
            break;
        }
        const Unknown_0066fef0* entry = reinterpret_cast<const Unknown_0066fef0*>(static_cast<uintptr_t>(other));
        if ((entry->Flags7A & 0x80u) != 0u) continue;
        // sub dx,[esi+0x68] leaves its flags; clc clears CF
        const uint32_t a = entry->Unknown68;
        const uint32_t b = record->Unknown68;
        const uint32_t res = (a - b) & 0xffffu;
        eax = other;
        flags = parityFlag8(res)
            | (((a ^ b ^ res) & 0x10u) != 0u ? 0x10u : 0u)
            | (res == 0u ? 0x40u : 0u)
            | ((res & 0x8000u) != 0u ? 0x80u : 0u)
            | ((((a ^ b) & (a ^ res)) & 0x8000u) != 0u ? 0x800u : 0u);
        break;
    }
    r->eax = eax;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004083ac, FUN_004083ac, "regs(esi:rec[0x66fef0,0x41c,0..21]) -> (eax, cf, pf, af, zf, sf, of); globals=layout=list_0066fe3c; cases=200")
