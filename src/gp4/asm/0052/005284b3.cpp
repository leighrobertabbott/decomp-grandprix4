#include <gp4/gp4.h>
#include <gp4/flags.h>

// Finds the first live entry of the 8-entry table at 0x81cd91 (stride 0x1c) whose pointer at +4
// points at a dword equal to eax. An entry is live when its dword +0 is nonzero. Returns the entry
// address in eax, or 0 when none matches. ecx ends as the key, edx as the last pointer loaded
// (unchanged when no entry was live). Both exits follow an equal compare (`cmp [edx], ecx` on a
// match, `cmp eax, 0x81ce71` at the end of the table), so the flags are those of a compare of equal
// operands.
// FUNCTION: GP4 0x005284b3
void __cdecl FUN_005284b3(gp4::Regs* r) {
    const uint32_t key = r->eax;
    uint32_t pointer = r->edx;
    uint32_t found = 0u;
    uint8_t* entry = GP4_ARRAY(uint8_t, 0x0081cd91);
    uint8_t* const end = GP4_ARRAY(uint8_t, 0x0081ce71);
    do {
        if (GP4_FIELD(uint32_t, entry, 0) != 0u) {
            pointer = GP4_FIELD(uint32_t, entry, 4);
            if (*reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(pointer)) == key) {
                found = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(entry));
                break;
            }
        }
        entry += 0x1c;
    } while (entry != end);
    r->eax = found;
    r->ecx = key;
    r->edx = pointer;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(key, key));
}
GP4_IMPL(0x005284b3, FUN_005284b3, "regs(eax:u32{0,1,2,3}, edx:u32) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=keys_0052848b")
