#include <gp4/gp4.h>

// FUNCTION: GP4 0x00402f8a
void __cdecl FUN_00402f8a(gp4::Regs* r) {
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const int16_t first = *reinterpret_cast<const int16_t*>(record + 0x12);
    const uint32_t second = *reinterpret_cast<const uint32_t*>(record + 0x08);
    const int16_t third = *reinterpret_cast<const int16_t*>(record + 0x16);
    r->eax = static_cast<uint32_t>(static_cast<int32_t>(first));
    r->edx = second;
    r->ecx = static_cast<uint32_t>(static_cast<int32_t>(third));
}
GP4_IMPL(0x00402f8a, FUN_00402f8a, "regs(esi:ptr[0x40]:bytes) -> (eax, edx, ecx)")
