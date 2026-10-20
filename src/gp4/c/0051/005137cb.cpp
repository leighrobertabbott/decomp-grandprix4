#include <gp4/gp4.h>
#include <gp4/flags.h>

// The table is indexed by the constant edx = 2; the compiler would fold the base into an absolute
// address and hide the global 0x00679b9c from the shape audit.
static __declspec(noinline) uint32_t tableEntry(const uint32_t* base, uint32_t index) {
    return base[index];
}

// Consumes the first `step` bytes of the event buffer: step = the dword at row [0x00679ba4] of the
// save-slot area (read by 0x00512d1a into [0x0088f604] and eax). The remaining [0x0066d830] - step
// bytes are moved from esi + step to the buffer [0x007a8e28] (forward byte copy), and when step is
// non-zero every slot row (pitch [0x006975c4], [0x007a8e1c] of them, starting at the row of
// [0x00679ba4]) whose first dword is 3 gets its position reduced by the step (0x00512d3f); then
// [0x0066d830] -= step. Registers: eax = [0x0066d830] - step (step on the early exit), ecx = 0,
// edx = high half of row * pitch (2 on the early exit), edi = end of the row walk (end of the copy
// on the early exit), esi = source + step + count. Flags: `sub eax, step` (`cmp step, 0` on the
// early exit).
// FUNCTION: GP4 0x005137cb
void __cdecl FUN_005137cb(gp4::Regs* r) {
    const uint32_t row = tableEntry(GP4_ARRAY(const uint32_t, 0x00679b9c), 2u);
    GP4_GLOBAL(uint32_t, 0x0088f624) = row;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00512d1a, c);

    const uint32_t step = GP4_GLOBAL(uint32_t, 0x0088f604);
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi + step));
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x007a8e28)));
    const uint32_t count = GP4_GLOBAL(uint32_t, 0x0066d830) - step;
    for (uint32_t i = 0u; i != count; ++i) {
        dst[i] = src[i];
    }

    r->ecx = 0u;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(src)) + count;

    if (step == 0u) {
        r->eax = c.eax;
        r->edx = 2u;
        r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(dst)) + count;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(step, 0u));
        return;
    }

    const uint32_t pitch = GP4_GLOBAL(uint32_t, 0x006975c4);
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(tableEntry(GP4_ARRAY(const uint32_t, 0x00679b9c), 2u)))
        * static_cast<int64_t>(static_cast<int32_t>(pitch));
    uint32_t edi = static_cast<uint32_t>(product) + GP4_GLOBAL(uint32_t, 0x007a8e18);
    uint32_t rows = GP4_GLOBAL(uint32_t, 0x007a8e1c);
    do {
        if (GP4_FIELD(uint32_t, edi, 0) == 3u) {
            gp4::Regs w = *r;
            w.st_in = 0;
            w.st_out = 0;
            w.eax = edi;
            gp4::call_regs(0x00512d3f, w);
        }
        edi += GP4_GLOBAL(uint32_t, 0x006975c4);
        --rows;
    } while (rows != 0u);

    const uint32_t position = GP4_GLOBAL(uint32_t, 0x0066d830);
    const uint32_t arith = gp4::flags::sub32(position, step);
    GP4_GLOBAL(uint32_t, 0x0066d830) = position - step;

    r->eax = position - step;
    r->edx = static_cast<uint32_t>(product >> 32);
    r->edi = edi;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005137cb, FUN_005137cb, "regs(esi:ptr[0x400]:bytes) -> (eax, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=rows_005137cb")
