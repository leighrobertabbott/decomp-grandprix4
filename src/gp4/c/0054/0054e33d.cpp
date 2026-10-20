#include <gp4/gp4.h>
#include <gp4/flags.h>

typedef int(__stdcall* SetCurrentDirectoryA_t)(const char*);

// Lists the language files: selects the root directory with 0x005367b9 (eax = 1), changes into
// "ldata" (SetCurrentDirectoryA thunk 0x005a48a6) and enumerates "*.str" (FindFirstFileA/FindNextFileA
// thunks 0x005a48dc/0x005a490c, find data at 0x00838a4f, cFileName at +0x2c). For each entry that is
// not a directory (attribute 0x10) nor read-only/hidden/system (0x106) and whose name is shorter
// than 0x18 (strlen 0x0054d8d0), the name up to its first '.' is copied NUL-terminated into the
// packed name buffer 0x00823300 and the pointer stored in the table 0x00838b99[count]; at most 11
// names ([0x00838b91]). The find handle [0x00838a4b] is closed (thunk 0x005a4948) and reset to -1. The
// registers and flags the two unbracketed/last Win32 calls leave are taken from those calls.
// FUNCTION: GP4 0x0054e33d
void __cdecl FUN_0054e33d(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.eax = 1u;
    gp4::call_regs(0x005367b9, c);

    // SetCurrentDirectoryA is not bracketed by push/pop in the original: eax, ecx and edx are what
    // it leaves (called with the live registers).
    gp4::Regs sd = c;
    const uint32_t dir_args[1] = {static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(const char, 0x00838a3f)))};
    gp4::call_regs_stdcall(0x005a48a6, sd, dir_args);
    uint32_t eax = sd.eax;
    uint32_t ebx = c.ebx;
    uint32_t ecx = sd.ecx;
    uint32_t edx = sd.edx;
    uint32_t esi = c.esi;
    uint8_t* names = GP4_ARRAY(uint8_t, 0x00823300);

    uint32_t& count = GP4_GLOBAL(uint32_t, 0x00838b91);
    uint32_t& handle = GP4_GLOBAL(uint32_t, 0x00838a4b);
    count = 0u;
    handle = 0xffffffffu;
    eax = gp4::stdcall_<0x005a48dc, uint32_t>(GP4_ARRAY(const char, 0x00838a45), GP4_ARRAY(uint8_t, 0x00838a4f));
    if (eax != 0xffffffffu) {
        handle = eax;
        for (;;) {
            uint8_t* data = GP4_ARRAY(uint8_t, 0x00838a4f);
            esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(data));
            const uint32_t attributes = *reinterpret_cast<uint32_t*>(data);
            if ((attributes & 0x10u) == 0u && (attributes & 0x106u) == 0u) {
                gp4::Regs s = c;
                s.eax = eax;
                s.ebx = ebx;
                s.ecx = ecx;
                s.edx = edx;
                s.esi = esi + 0x2cu;
                s.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(names));
                gp4::call_regs(0x0054d8d0, s);
                eax = s.eax;
                esi = s.esi;
                if (eax < 0x18u) {
                    uint32_t* slots = GP4_ARRAY(uint32_t, 0x00838b99);
                    slots[count] = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(names));
                    ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&slots[count]));
                    ecx = 0x15u;
                    edx = (edx & 0xffffff00u) | 0xffu;
                    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(esi));
                    for (;;) {
                        const uint8_t ch = *src++;
                        if (ch == '.' || ch == 0u) {
                            break;
                        }
                        *names++ = ch;
                    }
                    *names++ = 0u;
                    eax &= 0xffffff00u;
                    esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(src));
                    ++count;
                }
            }
            if (count >= 0xbu) {
                break;
            }
            eax = static_cast<uint32_t>(gp4::stdcall_<0x005a490c, int>(handle, GP4_ARRAY(uint8_t, 0x00838a4f)));
            if (eax != 1u) {
                break;
            }
        }
    }

    // The flags at the return are those of FindClose (called with the live registers) when a find
    // handle was open, else those of 'cmp [0x838a4b], -1'; ecx/edx/ebx are pushed around the call.
    uint32_t arith = gp4::flags::sub32(handle, 0xffffffffu);
    if (handle != 0xffffffffu) {
        gp4::Regs fc = c;
        fc.eax = eax;
        fc.ebx = ebx;
        fc.ecx = ecx;
        fc.edx = edx;
        fc.esi = esi;
        fc.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(names));
        fc.eflags = gp4::flags::merge(fc.eflags, arith);
        const uint32_t close_args[1] = {handle};
        gp4::call_regs_stdcall(0x005a4948, fc, close_args);
        eax = fc.eax;
        arith = fc.eflags;
        handle = 0xffffffffu;
    }

    r->eax = eax;
    r->ebx = ebx;
    r->ecx = ecx;
    r->edx = edx;
    r->esi = esi;
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(names));
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0054e33d, FUN_0054e33d, "regs() -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
