#include <gp4/gp4.h>
#include <gp4/flags.h>

// The base addresses are returned from helpers so the compiler keeps 0x0083e49c and 0x007ab760
// as the globals the original addresses (instead of folding 0x0083e49c + 0x38).
static __declspec(noinline) uint8_t* recordsFile() { return GP4_ARRAY(uint8_t, 0x0083e49c); }
static __declspec(noinline) uint8_t* driverTable() { return GP4_ARRAY(uint8_t, 0x007ab760); }

// Merges 17 records of 0xe8 bytes into the table at 0x007ab760: the pointer at +0x38 of the block
// at 0x0083e49c addresses the source records. For each pair, when the dword at +0x28 of the source
// is below the table's, the first 0x2e bytes are copied (rep movsb); when the dword at +0x54 is
// below the table's, the 0xba bytes from +0x2e are copied. pushal/popal around each copy keeps the
// registers, so the result registers are eax = the last source dword +0x54, ecx = 0 (loop), esi/edi
// advanced past the 17 records; the flags are those of the last `add edi, 0xe8` (loop keeps them).
// FUNCTION: GP4 0x0054fa80
void __cdecl FUN_0054fa80(gp4::Regs* r) {
    const uint8_t* source = *reinterpret_cast<const uint8_t* const*>(recordsFile() + 0x38);
    uint8_t* table = driverTable();
    uint32_t last = 0u;
    uint32_t arith = 0u;
    for (uint32_t remaining = 0x11u; remaining != 0u; --remaining) {
        const uint32_t first = *reinterpret_cast<const uint32_t*>(source + 0x28);
        if (first < *reinterpret_cast<const uint32_t*>(table + 0x28)) {
            for (uint32_t i = 0u; i != 0x2eu; ++i) {
                table[i] = source[i];
            }
        }
        last = *reinterpret_cast<const uint32_t*>(source + 0x54);
        if (last < *reinterpret_cast<const uint32_t*>(table + 0x54)) {
            for (uint32_t i = 0u; i != 0xbau; ++i) {
                table[0x2e + i] = source[0x2e + i];
            }
        }
        arith = gp4::flags::add32(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(table)), 0xe8u);
        source += 0xe8;
        table += 0xe8;
    }
    r->eax = last;
    r->ecx = 0u;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(source));
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(table));
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0054fa80, FUN_0054fa80, "regs() -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=recs_0054fa80")
