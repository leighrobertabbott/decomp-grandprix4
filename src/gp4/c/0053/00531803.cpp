#include <gp4/gp4.h>
#include <gp4/flags.h>

// `bt dword [0x7ad848], index` with a signed bit index: the dword at 0x7ad848 + 4 * (index >> 5),
// bit (index & 31).
static __declspec(noinline) uint32_t BitmapBit(uint32_t index) {
    const int32_t signedIndex = static_cast<int32_t>(index);
    const uint32_t* words = GP4_ARRAY(uint32_t, 0x007ad848);
    return (words[signedIndex >> 5] >> (index & 31u)) & 1u;
}

// Address of the 0x38-byte record `index` of the array at 0x7ad670.
static __declspec(noinline) uint32_t RecordAddress(uint32_t index) {
    return 0x007ad670u + index * 0x38u;
}

// FUNCTION: GP4 0x00531803
// Selects the current record: the index [0x7ad87c] (read through 0x530774) is stored at 0xa4a2d8.
// If its bit in the bitmap at 0x7ad848 is set, ebx = 0x7ad638 and [0xa4a2d4] = 0; otherwise
// ebx = 0x7ad670 + index * 0x38 and [0xa4a2d4] = 1. ebx is stored at 0x642bf0 and returned. eax is
// saved and restored. The flags are those of `bt` (CF = bit, the rest from `sub eax, 0`) when the
// bit is set, else of `add eax, 0x7ad670` after the `imul`.
void __cdecl FUN_00531803(gp4::Regs* r) {
    const uint32_t index = gp4::cdecl_<0x00530774, uint32_t>();
    GP4_GLOBAL(uint32_t, 0x00a4a2d8) = index;

    uint32_t flags = gp4::flags::sub32(index, 0u);
    uint32_t record;
    if (BitmapBit(index)) {
        flags = (flags & ~gp4::flags::CF) | gp4::flags::CF;
        GP4_GLOBAL(uint32_t, 0x00a4a2d4) = 0u;
        record = 0x007ad638u;
    } else {
        flags = flags & ~gp4::flags::CF;
        GP4_GLOBAL(uint32_t, 0x00a4a2d4) = 1u;
        record = RecordAddress(index);
        flags = gp4::flags::add32(index * 0x38u, 0x007ad670u);
    }
    GP4_GLOBAL(uint32_t, 0x00642bf0) = record;

    r->ebx = record;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00531803, FUN_00531803, "regs() -> (ebx, cf, pf, af, zf, sf, of); globals=layout=sel_00531859")
