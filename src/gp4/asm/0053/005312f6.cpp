#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper that fills the empty entries of a device record's seven setting dwords (+0x28..+0x40).
// For each column 0..6 whose dword is zero it asks 0x00531445 (ebx = the record, ecx = the column) for the
// speed-dependent default in edx; when the record's type dword [ebx] is nonzero and the dword +0xc of the
// 0x34-byte type descriptor (type - 1) * 0x34 + 0x0082736c is not above 0x800000, the default is scaled by
// 150 / 100 (signed 64-bit product, unsigned division). The result is stored into the column. popal restores
// every register; the flags are those of the final `cmp ecx, 7`.
// FUNCTION: GP4 0x005312f6
void __cdecl FUN_005312f6(gp4::Regs* r) {
    uint32_t* record = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(r->ebx));
    gp4::Regs c = *r;

    for (uint32_t column = 0u; column != 7u; ++column) {
        uint32_t* setting = record + 10u + column;
        if (*setting == 0u) {
            c.ecx = column;
            gp4::call_regs(0x00531445, c);
            uint32_t value = c.edx;
            const uint32_t type = record[0];
            if (type != 0u) {
                const uint32_t* descriptor =
                    reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>((type - 1u) * 0x34u + 0x0082736cu));
                if (descriptor[3] <= 0x00800000u) {
                    uint32_t remainder;
                    const int64_t product = static_cast<int64_t>(0x96) * static_cast<int64_t>(static_cast<int32_t>(value));
                    value = gp4::div64(static_cast<uint64_t>(product), 0x64u, &remainder);
                    c.edx = value;
                }
            }
            *setting = value;
        }
    }

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(7u, 7u));
}
GP4_IMPL(0x005312f6, FUN_005312f6, "regs(ebx:u32{=0x827160,0x82719c,0x8271d8}) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=dev_0053106d+keys_00530f3f")
