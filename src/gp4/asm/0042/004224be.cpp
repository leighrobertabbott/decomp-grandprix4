#include <gp4/gp4.h>

// FUNCTION: GP4 0x004224be
// eax = index of a 16-byte record (x, y, z, ...) in the table behind DAT_0062a058. Computes the
// 64-bit squared distance from (DAT_0062a064, DAT_0062a068, DAT_0062a06c) to the record's first
// three dwords and returns its integer square root (0x0040113f of edx:eax) in eax; edx keeps the
// high half of the squared distance, the flags are the callee's.
void __cdecl FUN_004224be(gp4::Regs* r) {
    const int32_t* record = reinterpret_cast<const int32_t*>(
        static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0062a058) + (r->eax << 4)));

    const int32_t dx = record[0] - GP4_GLOBAL(int32_t, 0x0062a064);
    const int64_t sx = static_cast<int64_t>(dx) * static_cast<int64_t>(dx);
    const int32_t dy = record[1] - GP4_GLOBAL(int32_t, 0x0062a068);
    const int64_t sy = static_cast<int64_t>(dy) * static_cast<int64_t>(dy);
    const int32_t dz = record[2] - GP4_GLOBAL(int32_t, 0x0062a06c);
    const int64_t sz = static_cast<int64_t>(dz) * static_cast<int64_t>(dz);

    const uint64_t squared = static_cast<uint64_t>(sx) + static_cast<uint64_t>(sy) + static_cast<uint64_t>(sz);

    gp4::Regs call = *r;
    call.eax = static_cast<uint32_t>(squared);
    call.edx = static_cast<uint32_t>(squared >> 32);
    gp4::call_regs(0x0040113f, call);

    r->eax = call.eax;
    r->edx = static_cast<uint32_t>(squared >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x004224be, FUN_004224be, "regs(eax:u32[0..63]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=state_00422397")
