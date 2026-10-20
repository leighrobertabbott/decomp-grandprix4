#include <gp4/gp4.h>

// Runs 0x00410967 (eax = max(3, (al * percent + 0x32) / 100)) on one byte. It preserves every
// other register; its eax and flags are copied back into the caller's register file.
static inline uint8_t scaleByte(gp4::Regs* r, uint8_t value) {
    gp4::Regs callee = *r;
    callee.eax = (callee.eax & 0xffffff00u) | value;
    gp4::call_regs(0x00410967, callee);
    r->eax = callee.eax;
    r->eflags = (r->eflags & ~0x8d5u) | (callee.eflags & 0x8d5u);
    return static_cast<uint8_t>(callee.eax);
}

// FUNCTION: GP4 0x004109aa
// Copies four bytes from the record at ebx into the record at esi (+0x198..+0x19b): the first
// verbatim, the other three scaled by 0x00410967. The source bytes are +0x33..+0x36 when
// the byte at 0x00601faa is 0xff, otherwise +0x0a and +0x0c..+0x0e.
// Leaves eax and the flags of the last 0x00410967 call.
void __cdecl FUN_004109aa(gp4::Regs* r) {
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    if (GP4_GLOBAL(uint8_t, 0x00601faa) == 0xffu) {
        dst[0x198] = src[0x33];
        dst[0x199] = scaleByte(r, src[0x34]);
        dst[0x19a] = scaleByte(r, src[0x35]);
        dst[0x19b] = scaleByte(r, src[0x36]);
    } else {
        dst[0x198] = src[0x0a];
        dst[0x199] = scaleByte(r, src[0x0c]);
        dst[0x19a] = scaleByte(r, src[0x0d]);
        dst[0x19b] = scaleByte(r, src[0x0e]);
    }
}
GP4_IMPL(0x004109aa, FUN_004109aa, "regs(ebx:ptr[0x40]:bytes, esi:ptr[0x1a8]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
