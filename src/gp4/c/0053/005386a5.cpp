#include <gp4/gp4.h>
#include <gp4/flags.h>

// Bulk read, twin of 0x538635 with source and destination swapped. When the file-mode byte
// [0x829c68] is 2 the request is recorded (file offset esi -> [0x829c69], destination edi ->
// [0x829c6d], count ecx -> [0x829c71]) and read from the open file handle [0x81ea28]:
// SetFilePointer(handle, offset, 0, FILE_BEGIN) (thunk 0x5a4936), then, unless it returned -1,
// ReadFile(handle, destination, count, &[0x81ea3c], 0) (thunk 0x5a48f4). CF is clear only when
// ReadFile returned 1. Otherwise (memory mode) `rep movsb` copies ecx bytes from esi to edi and CF
// is cleared. pushal/popal keeps every register; the flags are those of the last compare
// (`cmp byte [0x829c68], 2`, `cmp eax, -1` or `cmp eax, 1`) with CF forced by clc/stc.
// FUNCTION: GP4 0x005386a5
void __cdecl FUN_005386a5(gp4::Regs* r) {
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x00829c68);
    uint32_t arith = gp4::flags::sub8(mode, 2u);
    if (mode == 2u) {
        GP4_GLOBAL(uint32_t, 0x00829c69) = r->esi;
        GP4_GLOBAL(uint32_t, 0x00829c6d) = r->edi;
        GP4_GLOBAL(uint32_t, 0x00829c71) = r->ecx;
        const uint32_t pos = gp4::stdcall_<0x005a4936, uint32_t>(
            GP4_GLOBAL(uint32_t, 0x0081ea28), GP4_GLOBAL(uint32_t, 0x00829c69), 0u, 0u);
        if (pos == 0xffffffffu) {
            arith = gp4::flags::sub32(pos, 0xffffffffu) | gp4::CF;
        } else {
            const uint32_t ok = gp4::stdcall_<0x005a48f4, uint32_t>(
                GP4_GLOBAL(uint32_t, 0x0081ea28), GP4_GLOBAL(uint32_t, 0x00829c6d),
                GP4_GLOBAL(uint32_t, 0x00829c71),
                static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&GP4_GLOBAL(uint32_t, 0x0081ea3c))), 0u);
            arith = gp4::flags::sub32(ok, 1u);
            if (ok == 1u) {
                arith &= ~gp4::CF;
            } else {
                arith |= gp4::CF;
            }
        }
    } else {
        const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
        uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
        for (uint32_t n = r->ecx; n != 0u; --n) {
            *dst++ = *src++;
        }
        arith &= ~gp4::CF;
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005386a5, FUN_005386a5, "regs(esi:ptr[0x100]:bytes, edi:ptr[0x100]:bytes, ecx:u32[0..0x80]) -> (cf, pf, af, zf, sf, of); globals=layout=fh_0081ea28")
