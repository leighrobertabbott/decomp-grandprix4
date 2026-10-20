#include <gp4/gp4.h>

// FUNCTION: GP4 0x00417db1
// Clears the status byte 0x00601f1d and the dword at +0x1d4 of each of the four 0x300-byte records of
// the table at 0x006380d8 (ebx = the table base), then runs 0x00417c6c and 0x00417d32 on esi.
void __cdecl FUN_00417db1(gp4::Regs* r) {
    uint8_t* table = GP4_ARRAY(uint8_t, 0x006380d8);
    r->ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(table));
    GP4_GLOBAL(uint8_t, 0x00601f1d) = 0u;
    GP4_FIELD(uint32_t, table, 0x1d4) = 0u;
    GP4_FIELD(uint32_t, table, 0x4d4) = 0u;
    GP4_FIELD(uint32_t, table, 0x7d4) = 0u;
    GP4_FIELD(uint32_t, table, 0xad4) = 0u;
    gp4::call_regs(0x00417c6c, *r);
    gp4::call_regs(0x00417d32, *r);
}
GP4_IMPL(0x00417db1, FUN_00417db1, "regs(eax:u32, ecx:u32, edx:u32, esi:ptr[0xa2]:bytes) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_004178e0")
