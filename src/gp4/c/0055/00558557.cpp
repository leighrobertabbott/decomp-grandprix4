#include <gp4/gp4.h>
#include <gp4/flags.h>

extern "C" void __cdecl __debugbreak(void);

// Address of dword `index` of the id table at 0x0087059c.
static __declspec(noinline) uint32_t IdSlot(int32_t index) {
    return 0x0087059cu + static_cast<uint32_t>(index) * 4u;
}

// Address of the dword at `base` in row `row` of a 16-byte-row threshold table.
static __declspec(noinline) uint32_t RowField(uint32_t base, int32_t row) {
    return base + static_cast<uint32_t>(row) * 0x10u;
}

// Track loader record reader for the 8-word record at esi, mode 1 only ([0x0086857c] == 1): takes
// the next row number from the counter [0x00870598] (int3 once it reaches 0x3ff), stores it in the
// id table at [0x0087059c + 4 * word0] and stores the seven following words, sign-extended, in the
// row fields 0x868598/0x86859c/0x8685a0/0x8685a4 and 0x86c598/0x86c59c/0x86c5a0. Any other mode only
// skips the record. Both ways end with esi += 0x10 (popal restores the pointer first), with the
// flags of that add.
// FUNCTION: GP4 0x00558557
void __cdecl FUN_00558557(gp4::Regs* r) {
    const uint32_t source = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) == 1u) {
        const int32_t row = GP4_GLOBAL(int32_t, 0x00870598);
        GP4_GLOBAL(int32_t, 0x00870598) = row + 1;
        if (row >= 0x3ff) {
            __debugbreak();
        }
        const int16_t* words = reinterpret_cast<const int16_t*>(static_cast<uintptr_t>(source));
        GP4_GLOBAL(int32_t, IdSlot(words[0])) = row;
        GP4_GLOBAL(int32_t, RowField(0x00868598u, row)) = words[1];
        GP4_GLOBAL(int32_t, RowField(0x0086859cu, row)) = words[2];
        GP4_GLOBAL(int32_t, RowField(0x008685a0u, row)) = words[3];
        GP4_GLOBAL(int32_t, RowField(0x008685a4u, row)) = words[4];
        GP4_GLOBAL(int32_t, RowField(0x0086c598u, row)) = words[5];
        GP4_GLOBAL(int32_t, RowField(0x0086c59cu, row)) = words[6];
        GP4_GLOBAL(int32_t, RowField(0x0086c5a0u, row)) = words[7];
    }
    r->esi = source + 0x10u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(source, 0x10u));
}
GP4_IMPL(0x00558557, FUN_00558557, "regs(esi:ptr[0x18]:i16[-32767..32767]) -> (esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_0086857c+cnt_00870598")
