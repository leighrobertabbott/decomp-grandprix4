#include <gp4/gp4.h>

// Five helper calls on one register file (0x00401fb4, 0x004029e4, 0x0041d539, 0x0041d338,
// 0x0041d492), then the dword at 0x0063803c is stored to [esi+0xac]. The registers and flags are
// those the helpers left.
// FUNCTION: GP4 0x0041d770
void __cdecl FUN_0041d770(gp4::Regs* r) {
    gp4::call_regs(0x00401fb4, *r);
    gp4::call_regs(0x004029e4, *r);
    gp4::call_regs(0x0041d539, *r);
    gp4::call_regs(0x0041d338, *r);
    gp4::call_regs(0x0041d492, *r);

    r->eax = GP4_GLOBAL(uint32_t, 0x0063803c);
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    GP4_FIELD(uint32_t, record, 0xac) = r->eax;
}
GP4_IMPL(0x0041d770, FUN_0041d770, "regs(esi:ptr[0xb8]:bytes) -> (eax, ebx, ecx, edx, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
