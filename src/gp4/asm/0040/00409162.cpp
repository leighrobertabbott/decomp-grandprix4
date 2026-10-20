#include <gp4/gp4.h>

struct Unknown_00409162 {
    uint32_t Words[0x107];
};

// Clears the 0x41c-byte record at esi, stores its index in the 0x41c-byte array based at
// 0x0066fef0 at +0x202, and fills in the DAT_00891504 pointer (+0x1c, +0x1b8), the 0x0066fef0
// pointer (+0x110) and the 0xff marker (+0x25e). eax/ecx/edx/edi are saved and restored.
// The "div" leaves the flags of the preceding "xor edx, edx" in the oracle (ZF | PF).
// FUNCTION: GP4 0x00409162
void __cdecl FUN_00409162(gp4::Regs* r) {
    Unknown_00409162* record = reinterpret_cast<Unknown_00409162*>(r->esi);
    uint8_t* bytes = reinterpret_cast<uint8_t*>(r->esi);

    for (int i = 0; i < 0x107; ++i) {
        record->Words[i] = 0;
    }
    const uint32_t offset = r->esi - 0x0066fef0u;
    bytes[0x202] = (uint8_t)(offset / 0x41cu);

    GP4_FIELD(uint32_t, bytes, 0x1c) = GP4_GLOBAL(uint32_t, 0x00891504);
    GP4_FIELD(uint32_t, bytes, 0x1b8) = GP4_GLOBAL(uint32_t, 0x00891504);
    GP4_FIELD(uint32_t, bytes, 0x110) = 0x0066fef0u;
    bytes[0x25e] = 0xff;

    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00409162, FUN_00409162, "regs(esi:ptr[0x41c]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
