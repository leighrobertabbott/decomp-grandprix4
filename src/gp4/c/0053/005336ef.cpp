#include <gp4/gp4.h>
#include <gp4/flags.h>

// Looks up the key in eax among the 16 eight-byte entries of the table at 0x00828020 (key = dword
// at +4). Found: edi = the entry, CF clear. Not found (the count runs out): edi = 0, CF set. ecx is
// saved and restored. The flags other than CF are those of the final `or ecx, ecx`.
// FUNCTION: GP4 0x005336ef
void __cdecl FUN_005336ef(gp4::Regs* r) {
    const uint32_t key = r->eax;
    uint32_t count = 0x10u;
    const uint32_t* entry = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(0x00828020u));
    while (count != 0u && entry[1] != key) {
        entry += 2;
        --count;
    }
    uint32_t flags = gp4::flags::logic32(count);
    if (count != 0u) {
        r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(entry));
        flags &= ~1u;
    } else {
        r->edi = 0u;
        flags |= 1u;
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005336ef, FUN_005336ef, "regs(eax:u32{1,2,3,4,5,6,7,8,9,10,11,12}, ecx:u32) -> (edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=keys_005336ef")
