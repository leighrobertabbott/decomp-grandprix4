#include <gp4/gp4.h>

// FUNCTION: GP4 0x00432799
void __cdecl FUN_00432799(gp4::Regs* r) {
    // push ebx / push edi ... pop edi / pop ebx: both are restored; eax, flags and every other
    // register come back from the 0x434044 call.
    const uint8_t* body = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    int32_t row = static_cast<int32_t>(body[0x7c] & 0x3fu) - 1;
    if (row > 0x15) row = 0x15;

    // 28-byte records at 0x62ffac; dword +8 receives the value of 0x602b5c. A zero byte gives
    // row -1 (the record in front of the table).
    uint8_t* rec = &GP4_ARRAY(uint8_t, 0x0062ffac)[0] + row * 0x1c;
    GP4_FIELD(uint32_t, rec, 8) = GP4_GLOBAL(uint32_t, 0x00602b5c);

    gp4::Regs call = *r;
    call.eax = 5;
    gp4::call_regs(0x00434044, call);
    call.ebx = r->ebx;
    call.edi = r->edi;
    *r = call;
}
GP4_IMPL(0x00432799, FUN_00432799, "regs(esi:ptr[0x84]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
