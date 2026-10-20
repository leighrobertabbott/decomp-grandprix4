#include <gp4/gp4.h>
#include <gp4/flags.h>

// Clears every live entry of the 8-entry table at 0x81cd91 (stride 0x1c) whose pointer at +4 points
// at a dword equal to eax: an entry is live when its dword +0 is nonzero. ecx ends as the key (eax),
// eax as the table end 0x81ce71, edx as the last pointer loaded (unchanged when no entry was live);
// the flags are those of the final `cmp eax, 0x81ce71` (equal operands).
// FUNCTION: GP4 0x005284d8
void __cdecl FUN_005284d8(gp4::Regs* r) {
    const uint32_t key = r->eax;
    uint32_t pointer = r->edx;
    uint8_t* entry = GP4_ARRAY(uint8_t, 0x0081cd91);
    uint8_t* const end = GP4_ARRAY(uint8_t, 0x0081ce71);
    do {
        if (GP4_FIELD(uint32_t, entry, 0) != 0u) {
            pointer = GP4_FIELD(uint32_t, entry, 4);
            if (*reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(pointer)) == key) {
                GP4_FIELD(uint32_t, entry, 0) = 0u;
            }
        }
        entry += 0x1c;
    } while (entry != end);
    const uint32_t table_end = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end));
    r->ecx = key;
    r->eax = table_end;
    r->edx = pointer;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(table_end, table_end));
}
GP4_IMPL(0x005284d8, FUN_005284d8, "regs(eax:u32{0,1,2,3}, edx:u32) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=keys_0052848b")
