#include <gp4/gp4.h>

// The compiler would fold `address - address` into a constant and hide both globals from the shape audit.
static __declspec(noinline) uint32_t addrOf(const uint8_t* p) {
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p));
}

// The original indexes two dword tables with the constant edx = 2 ([edx*4 + base]); the compiler
// would fold that into absolute addresses and hide the table bases.
static __declspec(noinline) int32_t tableEntry(const int32_t* base, int32_t index) {
    return base[index];
}

// Register routine (ecx saved/restored). Picks the start row [0x0088f624] (0x00512ca2), refreshes
// [0x0088f604] (0x00512d1a), finds two earlier ring rows with 0x00512cc1 (3 and -1 when the byte
// [0x007c2e82] is set, which also moves the start row there; else 6 and 0x14) into [0x0088f62c] /
// [0x0088f628], then walks [0x007a8e1c] rows of the slot area ([0x007a8e18] + row * [0x006975c4])
// from the start row (row + 1, back to [0x00679ba4] past [0x00679bb4]) and appends to edi by the
// row's tag dword: 1 -> 4 bytes, 2 -> 8 bytes, 3 -> the whole row when it is one of the three
// chosen rows (then 0x00512d3f on the copy) else 8 bytes retagged 2; other tags copy nothing.
// Finally 0x00512d3f runs on [0x0088f5f8] + (0x0066d810 - 0x0066d7a4).
// Outputs: eax, edx = 2, esi/edi advanced, flags of the last 0x00512d3f.
// FUNCTION: GP4 0x00512d68
void __cdecl FUN_00512d68(gp4::Regs* r) {
    gp4::Regs c = *r;

    gp4::call_regs(0x00512ca2, c);
    GP4_GLOBAL(uint32_t, 0x0088f624) = c.eax;
    gp4::call_regs(0x00512d1a, c);

    c.eax = GP4_GLOBAL(uint8_t, 0x007c2e82) != 0 ? 3u : 6u;
    gp4::call_regs(0x00512cc1, c);
    GP4_GLOBAL(uint32_t, 0x0088f62c) = c.eax;
    if (GP4_GLOBAL(uint8_t, 0x007c2e82) != 0) {
        GP4_GLOBAL(uint32_t, 0x0088f624) = c.eax;
        GP4_GLOBAL(uint32_t, 0x0088f628) = 0xffffffffu;
    } else {
        c.eax = 0x14;
        gp4::call_regs(0x00512cc1, c);
        GP4_GLOBAL(uint32_t, 0x0088f628) = c.eax;
    }

    GP4_GLOBAL(uint32_t, 0x0088f608) = GP4_GLOBAL(uint32_t, 0x0088f624);
    uint32_t count = GP4_GLOBAL(uint32_t, 0x007a8e1c);
    uint8_t* dst = reinterpret_cast<uint8_t*>(c.edi);
    const uint8_t* src = reinterpret_cast<const uint8_t*>(c.esi);
    do {
        const uint32_t row = GP4_GLOBAL(uint32_t, 0x0088f608);
        src = reinterpret_cast<const uint8_t*>(row * GP4_GLOBAL(uint32_t, 0x006975c4) +
                                               GP4_GLOBAL(uint32_t, 0x007a8e18));
        const uint32_t tag = *reinterpret_cast<const uint32_t*>(src);
        if (tag == 1) {
            *reinterpret_cast<uint32_t*>(dst) = *reinterpret_cast<const uint32_t*>(src);
            src += 4;
            dst += 4;
        } else if (tag == 2) {
            reinterpret_cast<uint32_t*>(dst)[0] = reinterpret_cast<const uint32_t*>(src)[0];
            reinterpret_cast<uint32_t*>(dst)[1] = reinterpret_cast<const uint32_t*>(src)[1];
            src += 8;
            dst += 8;
        } else if (tag == 3) {
            uint8_t* const out = dst;
            if (row == GP4_GLOBAL(uint32_t, 0x0088f624) || row == GP4_GLOBAL(uint32_t, 0x0088f628) ||
                row == GP4_GLOBAL(uint32_t, 0x0088f62c)) {
                for (uint32_t n = GP4_GLOBAL(uint32_t, 0x006975c4); n != 0; --n) {
                    *dst++ = *src++;
                }
                c.eax = reinterpret_cast<uint32_t>(out);
                c.ecx = count;
                c.esi = reinterpret_cast<uint32_t>(src);
                c.edi = reinterpret_cast<uint32_t>(dst);
                gp4::call_regs(0x00512d3f, c);
            } else {
                reinterpret_cast<uint32_t*>(dst)[0] = reinterpret_cast<const uint32_t*>(src)[0];
                reinterpret_cast<uint32_t*>(dst)[1] = reinterpret_cast<const uint32_t*>(src)[1];
                src += 8;
                dst += 8;
                *reinterpret_cast<uint32_t*>(out) = 2;
            }
        }
        int32_t next = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0088f608)) + 1;
        if (next > tableEntry(GP4_ARRAY(int32_t, 0x00679bac), 2)) {
            next = tableEntry(GP4_ARRAY(int32_t, 0x00679b9c), 2);
        }
        GP4_GLOBAL(int32_t, 0x0088f608) = next;
    } while (--count != 0);

    c.eax = GP4_GLOBAL(uint32_t, 0x0088f5f8) + addrOf(GP4_ARRAY(uint8_t, 0x0066d810)) -
            addrOf(GP4_ARRAY(uint8_t, 0x0066d7a4));
    c.ecx = r->ecx;
    c.edx = 2;
    c.esi = reinterpret_cast<uint32_t>(src);
    c.edi = reinterpret_cast<uint32_t>(dst);
    gp4::call_regs(0x00512d3f, c);

    r->eax = c.eax;
    r->edx = 2;
    r->esi = reinterpret_cast<uint32_t>(src);
    r->edi = reinterpret_cast<uint32_t>(dst);
    r->eflags = c.eflags;
}
GP4_IMPL(0x00512d68, FUN_00512d68, "regs(edi:ptr[0x4e100]:bytes) -> (eax, edx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=walk_00512cc1+idx_00520b65+ring_00512d68")
