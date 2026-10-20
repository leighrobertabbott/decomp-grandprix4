#include <gp4/gp4.h>
#include <gp4/flags.h>

// Block write used by the save-file code: remembers the source (esi), destination/file offset (edi) and
// length (ecx) in 0x829bb4/0x829bb8/0x829bbc. When the mode byte [0x829ba3] is 2 the data is written to
// the open file [0x81ea28]: SetFilePointer(file, offset, NULL, FILE_BEGIN) (thunk 0x5a4936), then
// WriteFile(file, source, length, &[0x81ea3c], NULL) (thunk 0x5a491e); CF = 1 when either call fails or fewer
// bytes than requested were written. Otherwise the block is copied in memory (`rep movsb`) and CF = 0.
// pushal/popal keep every register; the flags are those of the last compare, with CF replaced by the
// clc/stc epilogue.
// FUNCTION: GP4 0x005381e6
void __cdecl FUN_005381e6(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x00829bb4) = r->esi;
    GP4_GLOBAL(uint32_t, 0x00829bb8) = r->edi;
    GP4_GLOBAL(uint32_t, 0x00829bbc) = r->ecx;

    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x00829ba3);
    uint32_t flags = gp4::flags::sub8(mode, 2u);
    uint32_t carry;
    if (mode == 2u) {
        const uint32_t position = gp4::stdcall_<0x005a4936, uint32_t>(
            GP4_GLOBAL(uint32_t, 0x0081ea28), GP4_GLOBAL(uint32_t, 0x00829bb8), 0u, 0u);
        flags = gp4::flags::sub32(position, 0xffffffffu);
        carry = 1u;
        if (position != 0xffffffffu) {
            const uint32_t ok = gp4::stdcall_<0x005a491e, uint32_t>(
                GP4_GLOBAL(uint32_t, 0x0081ea28), GP4_GLOBAL(uint32_t, 0x00829bb4), GP4_GLOBAL(uint32_t, 0x00829bbc),
                0x0081ea3cu, 0u);
            flags = gp4::flags::sub32(ok, 1u);
            if (ok == 1u) {
                const uint32_t wanted = GP4_GLOBAL(uint32_t, 0x00829bbc);
                const uint32_t written = GP4_GLOBAL(uint32_t, 0x0081ea3c);
                flags = gp4::flags::sub32(wanted, written);
                carry = wanted != written ? 1u : 0u;
            }
        }
    } else {
        const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
        uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
        for (uint32_t left = r->ecx; left != 0u; --left) {
            *dst++ = *src++;
        }
        carry = 0u;
    }
    r->eflags = gp4::flags::merge(r->eflags, (flags & ~1u) | carry);
}
GP4_IMPL(0x005381e6, FUN_005381e6, "regs(ecx:u32[0..0x40], edi:ptr[0x40]:bytes, esi:ptr[0x40]:bytes) -> (cf, pf, af, zf, sf, of); globals=layout=fh_0081ea28")
