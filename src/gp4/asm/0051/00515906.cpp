#include <gp4/gp4.h>
#include <gp4/flags.h>

// Hand-written routine on the record in esi.  Selects a slot index from byte [esi+0x74]
// (forced to 2 when dword [esi+0x1c] >= [0x891500]; otherwise advanced by one, wrapping at 3, unless
// bit 1 of byte [esi+0x82] is set), forces 2 again when the dword [esi+0x2c+idx*4] is 0x20000000,
// stores (idx != 2) in 0x00866054, stores [0x00893cec] (or that dword when it is zero) in 0x0086607c,
// clears bit 4 of byte [esi+0x8a] and sets byte 0x00954169 to 0xff.  The flags are those of the final `and`.
// The two table loads from 0x00891bd0 (entries 0xb / 0xc) are overwritten at once by the original.
static __declspec(noinline) uint32_t TableEntry(const uint32_t* table, uint32_t index) {
    return table[index];
}

// FUNCTION: GP4 0x00515906
void __cdecl FUN_00515906(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    uint32_t idx = rec[0x74];
    const uint32_t edx = *reinterpret_cast<const uint32_t*>(rec + 0x1c);
    if (edx < GP4_GLOBAL(uint32_t, 0x00891500)) {
        if ((rec[0x82] & 2u) == 0) {
            idx = idx + 1;
            if (static_cast<int32_t>(idx) >= 3) idx = 0;
        }
    } else {
        idx = 2;
    }
    if (*reinterpret_cast<const uint32_t*>(rec + 0x2c + idx * 4) == 0x20000000u) idx = 2;

    uint32_t eax;
    if (idx == 2) {
        GP4_GLOBAL(uint32_t, 0x00866054) = 0;
        eax = TableEntry(GP4_ARRAY(uint32_t, 0x00891bd0), 0xb);
        eax = 3;
    } else {
        GP4_GLOBAL(uint32_t, 0x00866054) = 1;
        eax = TableEntry(GP4_ARRAY(uint32_t, 0x00891bd0), 0xc);
        eax = 4;
    }
    (void)eax;

    eax = GP4_GLOBAL(uint32_t, 0x00893cec);
    if (eax == 0) eax = *reinterpret_cast<const uint32_t*>(rec + 0x2c + idx * 4);
    GP4_GLOBAL(uint32_t, 0x0086607c) = eax;
    const uint8_t cleared = static_cast<uint8_t>(rec[0x8a] & 0xefu);
    rec[0x8a] = cleared;
    GP4_GLOBAL(uint8_t, 0x00954169) = 0xff;

    r->eax = eax;
    r->ebx = idx;
    r->edx = edx;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(cleared));
}
GP4_IMPL(0x00515906, FUN_00515906, "regs(esi:ptr[0x430]:u32{0,1,2,0x20000000,0x20000,0xffffffff}) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
