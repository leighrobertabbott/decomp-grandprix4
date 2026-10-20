#include <gp4/gp4.h>

// FUNCTION: GP4 0x0041c5f7
// For each of the four records (stride 0x300) from 0x006380d8 up to 0x00638cd8: clears dword +0x40
// and copies dword +0x1c to the table at [esi + 0x39c + 4*i]. pushal/popal preserve every GPR; the
// flags are those of the last "cmp ebx, 0x638cd8" (equal).
void __cdecl FUN_0041c5f7(gp4::Regs* r) {
    uint8_t* table = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    uint32_t walker = 0x006380d8u;
    uint32_t slot = 0x39cu;
    do {
        uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(walker));
        GP4_FIELD(uint32_t, record, 0x40) = 0u;
        GP4_FIELD(uint32_t, table, slot) = GP4_FIELD(uint32_t, record, 0x1c);
        slot += 4u;
        walker += 0x300u;
    } while (walker != 0x00638cd8u);

    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0041c5f7, FUN_0041c5f7, "regs(esi:ptr[0x3ac]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=dw_0041c027")
