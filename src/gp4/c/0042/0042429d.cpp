#include <gp4/gp4.h>

// FUNCTION: GP4 0x0042429d
// Mirror of 0x0042430b: copies the record whose pointer is stored at 0x00609744 into the staged
// state at 0x00607f28..0x00607f54 (and 0x00607fbc), rearranging the field order.
void __cdecl FUN_0042429d(gp4::Regs* r) {
    uint8_t* source = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00609744)));
    GP4_GLOBAL(uint32_t, 0x00607f28) = GP4_FIELD(uint32_t, source, 0x00);
    GP4_GLOBAL(uint32_t, 0x00607f2c) = GP4_FIELD(uint32_t, source, 0x04);
    GP4_GLOBAL(uint32_t, 0x00607f30) = GP4_FIELD(uint32_t, source, 0x08);
    GP4_GLOBAL(uint32_t, 0x00607f34) = GP4_FIELD(uint32_t, source, 0x18);
    GP4_GLOBAL(uint32_t, 0x00607f38) = GP4_FIELD(uint32_t, source, 0x1c);
    GP4_GLOBAL(uint32_t, 0x00607f3c) = GP4_FIELD(uint32_t, source, 0x20);
    GP4_GLOBAL(uint32_t, 0x00607f4c) = GP4_FIELD(uint32_t, source, 0x0c);
    GP4_GLOBAL(uint32_t, 0x00607f50) = GP4_FIELD(uint32_t, source, 0x10);
    GP4_GLOBAL(uint32_t, 0x00607f54) = GP4_FIELD(uint32_t, source, 0x14);
    GP4_GLOBAL(uint32_t, 0x00607f40) = GP4_FIELD(uint32_t, source, 0x24);
    GP4_GLOBAL(uint32_t, 0x00607f44) = GP4_FIELD(uint32_t, source, 0x28);
    GP4_GLOBAL(uint32_t, 0x00607f48) = GP4_FIELD(uint32_t, source, 0x2c);
    const uint32_t last = GP4_FIELD(uint32_t, source, 0x58);
    GP4_GLOBAL(uint32_t, 0x00607fbc) = last;
    r->esi = reinterpret_cast<uintptr_t>(source);
    r->eax = last;
}
GP4_IMPL(0x0042429d, FUN_0042429d, "regs() -> (eax, esi); globals=layout=state_0042430b")
