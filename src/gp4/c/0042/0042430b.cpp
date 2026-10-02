#include <gp4/gp4.h>

// FUNCTION: GP4 0x0042430b
// Copies twelve dwords of staged state (0x00607f28..0x00607f54) into the record whose pointer
// is stored at 0x00609744, rearranging the field order.
void __cdecl FUN_0042430b(gp4::Regs* r) {
    uint8_t* dest = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00609744)));
    GP4_FIELD(uint32_t, dest, 0x00) = GP4_GLOBAL(uint32_t, 0x00607f28);
    GP4_FIELD(uint32_t, dest, 0x04) = GP4_GLOBAL(uint32_t, 0x00607f2c);
    GP4_FIELD(uint32_t, dest, 0x08) = GP4_GLOBAL(uint32_t, 0x00607f30);
    GP4_FIELD(uint32_t, dest, 0x18) = GP4_GLOBAL(uint32_t, 0x00607f34);
    GP4_FIELD(uint32_t, dest, 0x1c) = GP4_GLOBAL(uint32_t, 0x00607f38);
    GP4_FIELD(uint32_t, dest, 0x20) = GP4_GLOBAL(uint32_t, 0x00607f3c);
    GP4_FIELD(uint32_t, dest, 0x0c) = GP4_GLOBAL(uint32_t, 0x00607f4c);
    GP4_FIELD(uint32_t, dest, 0x10) = GP4_GLOBAL(uint32_t, 0x00607f50);
    GP4_FIELD(uint32_t, dest, 0x14) = GP4_GLOBAL(uint32_t, 0x00607f54);
    GP4_FIELD(uint32_t, dest, 0x24) = GP4_GLOBAL(uint32_t, 0x00607f40);
    GP4_FIELD(uint32_t, dest, 0x28) = GP4_GLOBAL(uint32_t, 0x00607f44);
    GP4_FIELD(uint32_t, dest, 0x2c) = GP4_GLOBAL(uint32_t, 0x00607f48);
    r->esi = reinterpret_cast<uintptr_t>(dest);
    r->eax = GP4_GLOBAL(uint32_t, 0x00607f48);
}
GP4_IMPL(0x0042430b, FUN_0042430b, "regs() -> (eax, esi); globals=layout=state_0042430b")
