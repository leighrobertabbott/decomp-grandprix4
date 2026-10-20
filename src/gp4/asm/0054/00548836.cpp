#include <gp4/gp4.h>
#include <gp4/flags.h>

// Counts the bytes equal to bl in row edx of the 22-row table at 0x007c2f2d (17-byte rows): the
// first [0x007c2e84] bytes of the row are compared. For edx >= 0x16 nothing is scanned and eax = 0.
// ecx, edx and esi are saved and restored. The flags are those of `cmp edx, 0x16` for an invalid
// row, else those of the last `inc esi` (pointer of the last byte examined), whose CF is kept from
// the last `cmp bl, [esi]` (bl < that byte; `inc eax` does not touch it).
// FUNCTION: GP4 0x00548836
void __cdecl FUN_00548836(gp4::Regs* r) {
    const uint32_t row = r->edx;
    const uint8_t key = static_cast<uint8_t>(r->ebx);
    uint32_t count = 0u;
    uint32_t flags = gp4::flags::sub32(row, 0x16u);
    if (row < 0x16u) {
        const uint8_t* cursor = GP4_ARRAY(const uint8_t, 0x007c2f2d) + row * 0x11u;
        uint32_t left = GP4_GLOBAL(uint32_t, 0x007c2e84);
        uint32_t carry = 0u;
        do {
            carry = key < *cursor ? 1u : 0u;
            if (key == *cursor) {
                ++count;
            }
            flags = gp4::flags::inc32(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(cursor)), carry);
            ++cursor;
            --left;
        } while (left != 0u);
    }
    r->eax = count;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00548836, FUN_00548836, "regs(ebx:u32{1,2,3,0x16}, edx:u32[0..0x1f]) -> (eax, cf, pf, af, zf, sf, of); globals=layout=cnt_00548836")
