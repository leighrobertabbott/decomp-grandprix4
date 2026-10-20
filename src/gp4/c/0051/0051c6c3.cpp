#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0051c6c3
// Builds a 6-bin histogram of a texture's row bytes. The word at 0x9e1b20 + 0x34d * 2 (= 0x9e21ba)
// is 0xffff (none) or a byte offset of a 0x20-byte entry of the texture table at 0x6975ec; the
// entry holds the row pointer (+0), the row width (+4) and the row count (+6), rows 0x100 bytes
// apart. Every byte b increments bin (uint8_t)(b - 0x12) of the dword table at 0x937de8, then each
// of the six bins becomes (bin << 14) / total, with total = width * rows (an unsigned 32-bit
// edx:eax = 0:eax idiv). pushal/popal keep every register; the flags are those of the last flag
// writer: `cmp bx,-1` when skipped, otherwise the final `cmp ecx,6`.
static __declspec(noinline) uint32_t SlotWord(uint32_t index) {
    return GP4_ARRAY(uint16_t, 0x009e1b20)[index];
}

void __cdecl FUN_0051c6c3(gp4::Regs* r) {
    uint32_t* bins = GP4_ARRAY(uint32_t, 0x00937de8);
    for (uint32_t i = 0; i < 6u; ++i) {
        bins[i] = 0u;
    }

    const uint32_t slot = SlotWord(0x34du);
    uint32_t f = gp4::flags::sub16(slot, 0xffffu);
    if (slot != 0xffffu) {
        uint8_t* entry = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(0x006975ecu + slot));
        const uint8_t* row = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(GP4_FIELD(uint32_t, entry, 0)));
        const uint32_t width = GP4_FIELD(uint16_t, entry, 4);
        uint32_t rows = GP4_FIELD(uint16_t, entry, 6);
        uint32_t total = 0u;
        do {
            const uint8_t* p = row;
            uint32_t n = width;
            do {
                const uint32_t bin = static_cast<uint8_t>(*p - 0x12u);
                bins[bin] = bins[bin] + 1u;
                ++total;
                ++p;
                --n;
            } while (n != 0u);
            row += 0x100;
            --rows;
        } while (rows != 0u);

        for (uint32_t i = 0; i < 6u; ++i) {
            int32_t rem;
            bins[i] = static_cast<uint32_t>(gp4::idiv64(static_cast<int64_t>(bins[i] << 14), static_cast<int32_t>(total), &rem));
        }
        f = gp4::flags::sub32(6u, 6u);
    }
    r->eflags = gp4::flags::merge(r->eflags, f);
}
GP4_IMPL(0x0051c6c3, FUN_0051c6c3, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=tex_0051c6c3")
