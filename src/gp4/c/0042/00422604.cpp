#include <gp4/gp4.h>

// Clears the dword table entry [0x62a048 + i*4] for every 16-byte record i of the table at
// 0x62a058 whose dword +0xc equals 5, for i = 0 .. [0x62a05c]-1 (do-while: the count is >= 1).
// Leaves esi = the table, edi = one past the last record, ebx = the count and the flags of the
// final `cmp ebx, [0x62a05c]` (equal).
// FUNCTION: GP4 0x00422604
void __cdecl FUN_00422604(gp4::Regs* r) {
    uint32_t* table = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0062a048)));
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0062a058)));
    uint32_t index = 0u;
    do {
        if (GP4_FIELD(uint32_t, record, 0xc) == 5u) {
            table[index] = 0u;
        }
        record += 0x10;
        ++index;
    } while (index != GP4_GLOBAL(uint32_t, 0x0062a05c));

    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(table));
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(record));
    r->ebx = index;
    // cmp ebx, [0x62a05c] with equality: ZF | PF
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00422604, FUN_00422604, "regs() -> (ebx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=list_0062a048")
