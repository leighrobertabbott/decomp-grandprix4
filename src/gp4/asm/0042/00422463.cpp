#include <gp4/gp4.h>

// For each of the DAT_0062a05c entries of the dword table behind DAT_0062a048: an entry that does
// not intersect the mask DAT_0062a054, or that has any of the 0x14280 bits set, gets the "none"
// value 0x80000000 in DAT_0060832c[i]; any other entry is measured by 0x00422327 (eax = i,
// edx = the caller's body) and its result is stored and folded into the running maximum
// DAT_0062a060 (starts at 0xc0000000, signed compare). Leaves the registers the loop used:
// esi = table, edi = mask, ebx = count, eax/ebp from the last measurement (unchanged if none),
// flags from the final "cmp ebx, count" (equal).
// FUNCTION: GP4 0x00422463
void __cdecl FUN_00422463(gp4::Regs* r) {
    const uint32_t* entries = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0062a048)));
    const uint32_t mask = GP4_GLOBAL(uint32_t, 0x0062a054);
    int32_t* results = GP4_ARRAY(int32_t, 0x0060832c);
    const uint32_t body = r->edx;
    uint32_t eax = r->eax;
    uint32_t ebp = r->ebp;

    GP4_GLOBAL(int32_t, 0x0062a060) = static_cast<int32_t>(0xc0000000u);

    uint32_t index = 0u;
    do {
        const uint32_t entry = entries[index];
        if ((entry & mask) == 0u || (entry & 0x14280u) != 0u) {
            results[index] = static_cast<int32_t>(0x80000000u);
        } else {
            gp4::Regs call = *r;
            call.eax = index;
            call.edx = body;
            gp4::call_regs(0x00422327, call);
            eax = call.eax;
            ebp = call.ebp;
            results[index] = static_cast<int32_t>(eax);
            if (static_cast<int32_t>(eax) >= GP4_GLOBAL(int32_t, 0x0062a060)) {
                GP4_GLOBAL(int32_t, 0x0062a060) = static_cast<int32_t>(eax);
            }
        }
        ++index;
    } while (index != GP4_GLOBAL(uint32_t, 0x0062a05c));

    r->eax = eax;
    r->ebp = ebp;
    r->ebx = index;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(entries));
    r->edi = mask;
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00422463, FUN_00422463, "regs(eax:u32, ebp:u32, edx:ptr[0xd0]:f64) -> (eax, ebp, ebx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=state_0042257b")
