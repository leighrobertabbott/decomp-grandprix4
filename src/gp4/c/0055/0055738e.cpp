#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the mode byte 0x0086857c is 2, files the record at ebx in the 4-slot pointer table at
// 0x006f3dbc: slot = (cl - 0x21) & 3, and sets bit 5 (0x20) of record byte +0x92, or bit 4 (0x10)
// when the slot's bit (2 for slots 0..1, 8 for slots 2..3) of the byte [0x0086855c] is clear.
// Leaves eax = slot * 4, edx = the record pointer, ebx = 0x006f3dbc and the flags of the final
// `or byte [ebx + 0x92], dl`; otherwise only the flags of the dead `add esi, 0`.
// FUNCTION: GP4 0x0055738e
void __cdecl FUN_0055738e(gp4::Regs* r) {
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 2u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(r->esi, 0u));
        return;
    }
    const uint32_t record = r->ebx;
    const uint32_t slot = (r->ecx - 0x21u) & 3u;
    const uint8_t modeBits = GP4_GLOBAL(uint8_t, 0x0086855c);
    const uint8_t tested = slot < 2u ? (modeBits & 2u) : (modeBits & 8u);
    const uint8_t bit = tested != 0u ? 0x20u : 0x10u;
    const uint8_t flagsByte = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x92) | bit);
    GP4_FIELD(uint8_t, record, 0x92) = flagsByte;
    GP4_ARRAY(uint32_t, 0x006f3dbc)[slot] = record;
    r->eax = slot << 2;
    r->edx = record;
    r->ebx = 0x006f3dbcu;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(flagsByte));
}
GP4_IMPL(0x0055738e, FUN_0055738e, "regs(eax:u32, ecx:u32{0x21,0x22,0x23,0x24,0x00,0xffffff23}, edx:u32, ebx:ptr[0xa0]:bytes, esi:u32) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
