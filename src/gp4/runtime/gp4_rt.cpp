// gp4_rt.cpp - registry bounds and gp4_call_regs. Linked into BOTH the emulator
// test DLL and the in-game hook DLL. (CRT shims for the freestanding test DLL
// live in gp4_freestanding.cpp.)
#include <gp4/gp4.h>

// ---------------------------------------------------------------- registry bounds
extern "C" __declspec(allocate(".gp4reg$a")) const gp4::ImplRecord gp4_impl_begin = {0, nullptr, nullptr, nullptr};
extern "C" __declspec(allocate(".gp4reg$z")) const gp4::ImplRecord gp4_impl_end = {0, nullptr, nullptr, nullptr};

// ---------------------------------------------------------------- calling original asm routines
// void gp4_call_regs(uint32_t addr, gp4::Regs* r)
// Loads r's GPRs and r->st_in x87 values, calls addr, stores GPRs, EFLAGS and
// r->st_out x87 values back. The callee must not pop stack arguments (true for
// register-convention routines).
extern "C" __declspec(naked) void __cdecl gp4_call_regs(uint32_t, gp4::Regs*) {
    __asm {
        push ebp
        push ebx
        push esi
        push edi
        mov eax, [esp+24]          // r
        push eax                   // [esp] = r
        mov ecx, [eax+0x60]        // st_in
    in_loop:
        test ecx, ecx
        jz in_done
        dec ecx
        fld qword ptr [eax+0x20+ecx*8]
        jmp in_loop
    in_done:
        push dword ptr [esp+24]    // [esp] = target, [esp+4] = r
        push dword ptr [eax+0x1C]  // input EFLAGS (routines that consume CF/ZF from the caller)
        popfd                      // nothing below touches flags before the call
        mov ebx, [eax+4]
        mov ecx, [eax+8]
        mov edx, [eax+12]
        mov esi, [eax+16]
        mov edi, [eax+20]
        mov ebp, [eax+24]
        mov eax, [eax]
        call dword ptr [esp]
        pushfd                     // [esp] = flags, [esp+4] = target, [esp+8] = r
        xchg eax, [esp+8]          // eax = r, slot = callee's eax
        pop dword ptr [eax+0x1C]   // eflags
        mov [eax+4], ebx
        mov [eax+8], ecx
        mov [eax+12], edx
        mov [eax+16], esi
        mov [eax+20], edi
        mov [eax+24], ebp
        mov ecx, [esp+4]
        mov [eax], ecx
        mov ecx, [eax+0x64]        // st_out
        xor edx, edx
    out_loop:
        cmp edx, ecx
        jge out_done
        fstp qword ptr [eax+0x20+edx*8]
        inc edx
        jmp out_loop
    out_done:
        add esp, 8
        pop edi
        pop esi
        pop ebx
        pop ebp
        ret
    }
}
