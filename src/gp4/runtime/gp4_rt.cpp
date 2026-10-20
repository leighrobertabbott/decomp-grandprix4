// gp4_rt.cpp - registry bounds and gp4_call_regs. Linked into BOTH the emulator
// test DLL and the in-game hook DLL. (CRT shims for the freestanding test DLL
// live in gp4_freestanding.cpp.)
#include <gp4/gp4.h>

// ---------------------------------------------------------------- registry bounds
extern "C" __declspec(allocate(".gp4reg$a")) const gp4::ImplRecord gp4_impl_begin = {0, nullptr, nullptr, nullptr};
extern "C" __declspec(allocate(".gp4reg$z")) const gp4::ImplRecord gp4_impl_end = {0, nullptr, nullptr, nullptr};

// ---------------------------------------------------------------- calling original asm routines
// void gp4_call_regs(uint32_t addr, gp4::Regs* r)
// Loads r's GPRs and r->st_in x87 values (80-bit from st_xin where st_xin_mask
// says so), calls addr, stores GPRs, EFLAGS and r->st_out x87 values back (both
// as doubles in st[] and at full precision in st_xout[], st_xout_mask set). The callee must not pop stack arguments (true for
// register-convention routines).
extern "C" __declspec(naked) void __cdecl gp4_call_regs(uint32_t, gp4::Regs*) {
    __asm {
        push ebp
        push ebx
        push esi
        push edi
        mov eax, [esp+24]          // r
        push eax                   // [esp] = r
        mov dword ptr [eax+0x10C], 0   // st_xout_mask: filled below
        mov ecx, [eax+0x60]        // st_in
    in_loop:
        test ecx, ecx
        jz in_done
        dec ecx
        bt dword ptr [eax+0x108], ecx  // st_xin_mask bit k: 80-bit input
        jc in_ext
        fld qword ptr [eax+0x20+ecx*8]
        jmp in_loop
    in_ext:
        lea edx, [ecx+ecx*4]
        fld tbyte ptr [eax+0x68+edx*2] // st_xin[k]
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
        fld st(0)
        fstp qword ptr [eax+0x20+edx*8]
        push edx
        lea edx, [edx+edx*4]
        fstp tbyte ptr [eax+0xB8+edx*2] // st_xout[k]: the value at full precision
        pop edx
        bts dword ptr [eax+0x10C], edx
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

// void gp4_call_regs_stdcall(uint32_t addr, gp4::Regs* r, const uint32_t* args, uint32_t n)
// Like gp4_call_regs (GPRs and EFLAGS in, GPRs and EFLAGS out; no x87), but first pushes the n
// dwords args[0..n-1] as stack arguments (args[0] nearest the return address) for a callee that
// pops them itself (stdcall: 'ret 4*n'). Used where an original hand-written routine calls a
// stdcall routine with live registers (eax already set, say) and keeps the registers it returns.
// No static state: the target is staged in the dword just below the arguments, which
// 'call dword ptr [esp-4]' reads before it pushes the return address over it.
extern "C" __declspec(naked) void __cdecl gp4_call_regs_stdcall(uint32_t, gp4::Regs*, const uint32_t*, uint32_t) {
    __asm {
        push ebp
        push ebx
        push esi
        push edi
        mov eax, [esp+24]          // r
        mov ebx, [esp+20]          // addr
        push eax                   // [esp] = r: args now at +32, n at +36
        mov ecx, [esp+36]          // n
        mov edx, [esp+32]          // args
    push_loop:
        test ecx, ecx
        jz pushed
        dec ecx
        push dword ptr [edx+ecx*4] // last argument first
        jmp push_loop
    pushed:
        push dword ptr [eax+0x1C]
        popfd                      // input EFLAGS; nothing below writes flags before the call
        mov [esp-4], ebx           // target, just below the arguments (no push/pop from here on)
        mov ebx, [eax+4]
        mov ecx, [eax+8]
        mov edx, [eax+12]
        mov esi, [eax+16]
        mov edi, [eax+20]
        mov ebp, [eax+24]
        mov eax, [eax]
        call dword ptr [esp-4]
        pushfd                     // the callee popped its arguments: [esp] = flags, [esp+4] = r
        xchg eax, [esp+4]          // eax = r, slot = callee's eax
        pop dword ptr [eax+0x1C]
        mov [eax+4], ebx
        mov [eax+8], ecx
        mov [eax+12], edx
        mov [eax+16], esi
        mov [eax+20], edi
        mov [eax+24], ebp
        pop ecx                    // callee's eax
        mov [eax], ecx
        pop edi
        pop esi
        pop ebx
        pop ebp
        ret
    }
}
