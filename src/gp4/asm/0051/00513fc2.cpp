#include <gp4/gp4.h>
#include <gp4/flags.h>

// Dword at +0x2c + 4*index of the record (index = byte at +0x74).
static __declspec(noinline) uint32_t TargetAt(uint32_t record, uint32_t index) {
    return *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(record + index * 4u + 0x2cu));
}
// Dword at +0x20 + 4*index of the record.
static __declspec(noinline) uint32_t SourceAt(uint32_t record, uint32_t index) {
    return *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(record + index * 4u + 0x20u));
}

// FUNCTION: GP4 0x00513fc2
// Raises event 3 through the dispatcher 0x514152, then (unless the record's dword +0x2c[i] is the
// 0x20000000 "none" marker, i = byte +0x74) publishes it in 0x893cec and raises event 0x12 (with
// 0x866078 = +0x20[i] - +0x2c[i], unless +0x20[i] is the 0x10000000 marker) and event 0x13.
// 0x893cec is cleared on exit. eax and ebx are left as the original leaves them.
void __cdecl FUN_00513fc2(gp4::Regs* r) {
    const uint32_t record = r->esi;
    const uint32_t index = GP4_FIELD(uint8_t, record, 0x74);
    r->ebx = index;
    GP4_GLOBAL(uint32_t, 0x0088ff30) = 3u;
    gp4::Regs c = *r;
    gp4::call_regs(0x00514152, c);
    r->ebx = c.ebx;
    r->eax = c.eax;
    uint32_t flags = gp4::flags::sub32(TargetAt(record, index), 0x20000000u);
    if (TargetAt(record, index) != 0x20000000u) {
        uint32_t target = TargetAt(record, index);
        GP4_GLOBAL(uint32_t, 0x00893cec) = target;
        uint32_t source = SourceAt(record, index);
        flags = gp4::flags::sub32(source, 0x10000000u);
        r->eax = source;
        if (source != 0x10000000u) {
            const uint32_t diff = source - TargetAt(record, index);
            flags = gp4::flags::sub32(source, TargetAt(record, index));
            r->eax = diff;
            GP4_GLOBAL(uint32_t, 0x00866078) = diff;
            GP4_GLOBAL(uint32_t, 0x0088ff30) = 0x12u;
            c = *r;
            c.eflags = gp4::flags::merge(r->eflags, flags);
            gp4::call_regs(0x00514152, c);
            r->eax = c.eax;
            r->ebx = c.ebx;
            flags = c.eflags;
        }
        GP4_GLOBAL(uint32_t, 0x0088ff30) = 0x13u;
        c = *r;
        c.eflags = gp4::flags::merge(r->eflags, flags);
        gp4::call_regs(0x00514152, c);
        r->eax = c.eax;
        r->ebx = c.ebx;
        flags = c.eflags;
    }
    GP4_GLOBAL(uint32_t, 0x00893cec) = 0u;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00513fc2, FUN_00513fc2, "regs(eax:u32, ebx:u32, ecx:u32, edi:u32, esi:ptr[0x7c]:u32{=0x20000000,0x10000000,1,2,3,5,7,0x1000,0xfffff000}) -> (eax, ebx, cf, pf, af, zf, sf, of); globals=layout=player_0040f8d9+disp_005141b8+events_00514152")
