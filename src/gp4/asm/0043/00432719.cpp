#include <gp4/gp4.h>

// pushal ... popal wrapper around a car record (esi). Entry `index` = (byte +0x7c & 0x3f) - 1,
// capped at 0x15, of the 0x1c-byte table at 0x0062ffac gets the word +0x64 with bit 2 (byte +0x80
// bit 7) and bit 3 (byte +0x80 bit 6) added, and in its second dword the car's 0x43c75b figure plus
// the clock [0x0066d814]. Then 0x00434044 (eax = 9) sets the same car's bit in row 9 of the car-bit
// masks. Registers are restored by popal; the flags are those 0x00434044 leaves (it rewrites all
// six arithmetic flags itself, so nothing earlier reaches the caller).
// FUNCTION: GP4 0x00432719
void __cdecl FUN_00432719(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    int32_t index = static_cast<int32_t>(GP4_FIELD(uint8_t, car, 0x7c) & 0x3fu) - 1;
    if (index > 0x15) {
        index = 0x15;
    }

    uint32_t bits = GP4_FIELD(uint16_t, car, 0x64);
    if (GP4_FIELD(uint8_t, car, 0x80) & 0x80u) {
        bits |= 4u;
    }
    if (GP4_FIELD(uint8_t, car, 0x80) & 0x40u) {
        bits |= 8u;
    }

    uint8_t* entry = GP4_ARRAY(uint8_t, 0x0062ffac) + index * 0x1c;
    GP4_FIELD(uint32_t, entry, 0) = bits;

    gp4::Regs callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    callee.eax = r->esi;
    gp4::call_regs(0x0043c75b, callee);
    GP4_FIELD(uint32_t, entry, 4) = callee.eax + GP4_GLOBAL(uint32_t, 0x0066d814);

    callee.eax = 9;
    gp4::call_regs(0x00434044, callee);

    r->eflags = (r->eflags & ~0x8d5u) | (callee.eflags & 0x8d5u);
}
GP4_IMPL(0x00432719, FUN_00432719, "regs(esi:ptr[0x238]:layout=car_0043c677) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00432330+state_0043c677+segw_0043c608+rows_00434044")
