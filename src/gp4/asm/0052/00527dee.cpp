#include <gp4/gp4.h>
#include <gp4/flags.h>

static inline uint32_t& Dword(uint32_t address) {
    return *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(address));
}

// FUNCTION: GP4 0x00527dee
// Loads the frame number eax into its cache cell: [0x81cd58] = eax; fails (stc) when the frame
// file is not open (byte 0x81cd90 == 0) or eax >= the frame count [0x8174f8 - 4]. The frame record
// is esi = 0x8174f8 + 0x18 * eax and the handle cell edi = 0x81c0b8 + 4 * eax. A nonzero cell is
// validated by 0x0053888e (a stale handle, answer 0, clears the cell). An empty cell is refilled:
// 0x005387d6 allocates the record's size (dword +0x10); no block (0) leaves with CF clear, else the
// handle is stored in the cell, the file is positioned at the record's offset (+0x14) with
// SetFilePointer(file [0x81ea28], offset, NULL, FILE_BEGIN) (thunk 0x5a4936), the block address
// comes from 0x005388ba and ReadFile (thunk 0x5a48f4) fills it with +0x10 bytes (count in
// 0x81ea38). CF is set unless ReadFile returned 1 with the full count; a filled cell leaves CF
// clear. eax/ebx/ecx/edx are restored (push/pop); esi and edi leave pointing at the record and
// the cell, the flags are those of the last compare with CF replaced by the clc/stc epilogue.
void __cdecl FUN_00527dee(gp4::Regs* r) {
    const uint32_t frame = r->eax;
    GP4_GLOBAL(uint32_t, 0x0081cd58) = frame;

    uint32_t flags = gp4::flags::sub8(GP4_GLOBAL(uint8_t, 0x0081cd90), 0u);
    uint32_t carry = 1u;
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    if (GP4_GLOBAL(uint8_t, 0x0081cd90) != 0u) {
        const uint32_t count = GP4_GLOBAL(uint32_t, 0x008174f4);
        flags = gp4::flags::sub32(frame, count);
        if (frame < count) {
            c.esi = frame * 0x18u + 0x008174f8u;
            c.edi = GP4_GLOBAL(uint32_t, 0x0081cd58) * 4u + 0x0081c0b8u;

            const uint32_t cell = Dword(c.edi);
            flags = gp4::flags::logic32(cell);
            if (cell != 0u) {
                c.eax = cell;
                c.eflags = gp4::flags::merge(c.eflags, flags);
                gp4::call_regs(0x0053888e, c);
                flags = gp4::flags::logic32(c.eax);
                if (c.eax == 0u) {
                    Dword(c.edi) = 0u;
                }
            }
            flags = gp4::flags::sub32(Dword(c.edi), 0u);
            if (Dword(c.edi) != 0u) {
                carry = 0u;
            } else {
                c.eax = Dword(c.esi + 0x10u);
                c.eflags = gp4::flags::merge(c.eflags, flags);
                gp4::call_regs(0x005387d6, c);
                flags = gp4::flags::logic32(c.eax);
                if (c.eax == 0u) {
                    carry = 0u;
                } else {
                    Dword(c.edi) = c.eax;
                    const uint32_t seek[4] = {GP4_GLOBAL(uint32_t, 0x0081ea28), Dword(c.esi + 0x14u), 0u, 0u};
                    gp4::call_regs_stdcall(0x005a4936, c, seek);
                    c.eax = Dword(c.edi);
                    gp4::call_regs(0x005388ba, c);

                    const uint32_t countAddr =
                        static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&GP4_GLOBAL(uint8_t, 0x0081ea38)));
                    const uint32_t read[5] = {GP4_GLOBAL(uint32_t, 0x0081ea28), c.eax, Dword(c.esi + 0x10u),
                                              countAddr, 0u};
                    gp4::call_regs_stdcall(0x005a48f4, c, read);
                    const uint32_t wanted = Dword(c.esi + 0x10u);
                    flags = gp4::flags::sub32(c.eax, 1u);
                    carry = 1u;
                    if (c.eax == 1u) {
                        flags = gp4::flags::sub32(wanted, GP4_GLOBAL(uint32_t, 0x0081ea38));
                        if (wanted == GP4_GLOBAL(uint32_t, 0x0081ea38)) {
                            carry = 0u;
                        }
                    }
                }
            }
        }
    }

    r->esi = c.esi;
    r->edi = c.edi;
    r->eflags = gp4::flags::merge(r->eflags, (flags & ~1u) | carry);
}
GP4_IMPL(0x00527dee, FUN_00527dee, "regs(eax:u32[0..16]) -> (esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=frames_008174f8+slots_005388ed+slot8_005388ba+cells_00527eae+pool_005387dc+serial_005387dc; emu=skip:win32 SetFilePointer")
