#include <gp4/gp4.h>
#include <gp4/flags.h>

// 13 entries of 0xe0 bytes at 0x866248: the dword at +0xdc is copied to +0x6c. Leaves eax = the last
// value copied, ebx = the end of the table, ecx = 0 and the flags of the final `add ebx, 0xe0`
// (`loop` does not touch them).
// FUNCTION: GP4 0x005539d1
void __cdecl FUN_005539d1(gp4::Regs* r) {
    uint8_t* entry = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(0x00866248));
    uint32_t value = 0;
    uint32_t flags = 0;
    for (int left = 13; left != 0; --left) {
        value = GP4_FIELD(uint32_t, entry, 0xdc);
        GP4_FIELD(uint32_t, entry, 0x6c) = value;
        flags = gp4::flags::add32(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(entry)), 0xe0u);
        entry += 0xe0;
    }
    r->eax = value;
    r->ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(entry));
    r->ecx = 0;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005539d1, FUN_005539d1, "regs() -> (eax, ebx, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=recs_005539d1")
