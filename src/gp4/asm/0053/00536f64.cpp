#include <gp4/gp4.h>
#include <gp4/flags.h>

// Follows a chain of 16-byte records ("free list" style links) until a record whose dword +8 is -2.
// A record is found from an index: indices below the signed base [0x00829710] map to the static
// sentinel record at 0x00829700, the others to ((index - base) & 0x1fff) * 16 + [0x00829278].
// Start: eax -> record; eax = its dword +4; then repeat: record = lookup(eax); stop when its
// dword +8 == -2, else eax = that dword. Exit: eax = last index, esi = the record found, flags of
// the final `cmp dword [esi+8], -2` (equal).
static __declspec(noinline) uint32_t record_address(uint32_t index) {
    if ((int32_t)index < (int32_t)GP4_GLOBAL(uint32_t, 0x00829710)) {
        return (uint32_t)reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x00829700));
    }
    return (((index - GP4_GLOBAL(uint32_t, 0x00829710)) & 0x1fffu) << 4) + GP4_GLOBAL(uint32_t, 0x00829278);
}

// FUNCTION: GP4 0x00536f64
void __cdecl FUN_00536f64(gp4::Regs* r) {
    uint32_t record = record_address(r->eax);
    uint32_t index = *(const uint32_t*)(uintptr_t)(record + 4u);
    uint32_t link;
    for (;;) {
        record = record_address(index);
        link = *(const uint32_t*)(uintptr_t)(record + 8u);
        if (link == 0xfffffffeu) {
            break;
        }
        index = link;
    }
    r->eax = index;
    r->esi = record;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(link, 0xfffffffeu));
}
GP4_IMPL(0x00536f64, FUN_00536f64, "regs(eax:u32) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00829278")
