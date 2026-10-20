#include <gp4/gp4.h>
#include <gp4/flags.h>

// Builds 16 brightened rows of a 256-entry colour remap table. For every index 255..0 the int16
// {x, y, z} point of entry `index` of the table [0x0096a3d4] (6-byte entries) is copied to the words
// 0x933a14/0x933a16/0x933a18; then for rows 0..15 each word is raised by 0x3f0 and clamped (signed)
// to 0..0x3f00, and the nearest table entry to the result (0x0051b645) is stored at
// 0x923104 + 0x1000 + row * 0x100 + index. Index 0 stores 0 in every row without a search. Leaves
// esi = the table, ecx = -1, edx = 0, edi = 0x1000, ebp = 0, al = 0; flags of the final `dec ecx`
// (0 -> -1) with the CF of the last `add edi, 0x100` (clear).

// `add ax, 0x3f0` then the signed clamps `cmp ax, 0; jge` / `cmp ax, 0x3f00; jle`.
static inline uint16_t Brighten(uint16_t word) {
    uint16_t value = static_cast<uint16_t>(word + 0x3f0u);
    if (static_cast<int16_t>(value) < 0) {
        value = 0u;
    }
    if (static_cast<int16_t>(value) > 0x3f00) {
        value = 0x3f00u;
    }
    return value;
}

// `mov byte ptr [ecx + edi + 0x923104], al`: the table base stays the global 0x923104, the offset
// (edi + ecx) is computed by the caller.
static __declspec(noinline) void StoreShade(uint32_t offset, uint8_t value) {
    GP4_ARRAY(uint8_t, 0x00923104)[offset] = value;
}

// FUNCTION: GP4 0x0051be22
void __cdecl FUN_0051be22(gp4::Regs* r) {
    const uint32_t table = GP4_GLOBAL(uint32_t, 0x0096a3d4);
    gp4::Regs c = *r;
    c.esi = table;
    uint32_t eax = r->eax;

    for (int32_t index = 0xff; index >= 0; --index) {
        const uint32_t offset = static_cast<uint32_t>(index) * 6u;
        const uint16_t* const entry = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(table + offset));
        GP4_GLOBAL(uint16_t, 0x00933a14) = entry[0];
        GP4_GLOBAL(uint16_t, 0x00933a16) = entry[1];
        eax = (eax & 0xffff0000u) | entry[2];
        GP4_GLOBAL(uint16_t, 0x00933a18) = entry[2];

        uint32_t rowOffset = 0x1000u;
        for (uint32_t rows = 16u; rows != 0u; --rows) {
            eax &= 0xffffff00u;
            if (index != 0) {
                GP4_GLOBAL(uint16_t, 0x00933a14) = Brighten(GP4_GLOBAL(uint16_t, 0x00933a14));
                GP4_GLOBAL(uint16_t, 0x00933a16) = Brighten(GP4_GLOBAL(uint16_t, 0x00933a16));
                const uint16_t z = Brighten(GP4_GLOBAL(uint16_t, 0x00933a18));
                GP4_GLOBAL(uint16_t, 0x00933a18) = z;
                c.eax = (eax & 0xffff0000u) | z;
                c.ecx = static_cast<uint32_t>(index);
                c.edx = offset;
                c.edi = rowOffset;
                c.ebp = rows;
                gp4::call_regs(0x0051b645, c);
                eax = c.eax;
            }
            StoreShade(rowOffset + static_cast<uint32_t>(index), static_cast<uint8_t>(eax));
            rowOffset += 0x100u;
        }
    }

    r->eax = eax;
    r->ecx = 0xffffffffu;
    r->edx = 0u;
    r->esi = table;
    r->edi = 0x1000u;
    r->ebp = 0u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(0u, gp4::flags::add32(0x1f00u, 0x100u)));
}
GP4_IMPL(0x0051be22, FUN_0051be22, "regs(eax:u32) -> (eax, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=pts_0051b645; maxinsn=40000000; wall=3600")
