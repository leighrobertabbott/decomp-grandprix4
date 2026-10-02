// gp4_freestanding.cpp - CRT shims for the freestanding emulator test DLL only.
//
// The test DLL is built /NODEFAULTLIB /NOENTRY so it can sit next to GP4.exe in
// the emulator with no CRT and no imports. Everything the compiler may emit
// implicitly lives here. If the linker reports an unresolved helper, add it here
// (never pull in the CRT). The hook DLL links the real CRT and skips this file.
#include <gp4/gp4.h>

extern "C" int _fltused = 0;

// ---------------------------------------------------------------- memory intrinsics
extern "C" void* __cdecl memset(void*, int, size_t);
extern "C" void* __cdecl memcpy(void*, const void*, size_t);
extern "C" int __cdecl memcmp(const void*, const void*, size_t);
#pragma intrinsic(memset, memcpy, memcmp)
#pragma function(memset, memcpy, memcmp)
extern "C" void* __cdecl memset(void* d, int c, size_t n) {
    unsigned char* p = static_cast<unsigned char*>(d);
    while (n--) *p++ = static_cast<unsigned char>(c);
    return d;
}
extern "C" void* __cdecl memcpy(void* d, const void* s, size_t n) {
    unsigned char* p = static_cast<unsigned char*>(d);
    const unsigned char* q = static_cast<const unsigned char*>(s);
    while (n--) *p++ = *q++;
    return d;
}
extern "C" int __cdecl memcmp(const void* a, const void* b, size_t n) {
    const unsigned char* p = static_cast<const unsigned char*>(a);
    const unsigned char* q = static_cast<const unsigned char*>(b);
    for (; n; --n, ++p, ++q)
        if (*p != *q) return *p < *q ? -1 : 1;
    return 0;
}

// ---------------------------------------------------------------- float -> int (C cast semantics: truncate)
extern "C" __declspec(naked) void _ftol2() {
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 12
        fnstcw word ptr [ebp-2]
        mov ax, word ptr [ebp-2]
        or ax, 0x0C00
        mov word ptr [ebp-4], ax
        fldcw word ptr [ebp-4]
        fistp qword ptr [ebp-12]
        fldcw word ptr [ebp-2]
        mov eax, dword ptr [ebp-12]
        mov edx, dword ptr [ebp-8]
        leave
        ret
    }
}
extern "C" __declspec(naked) void _ftol2_sse() { __asm jmp _ftol2 }
extern "C" __declspec(naked) void _ftol() { __asm jmp _ftol2 }

// ---------------------------------------------------------------- stack probe (large locals)
extern "C" __declspec(naked) void _chkstk() {
    __asm {
        push ecx
        lea ecx, [esp+4]
        sub ecx, eax
        sbb eax, eax
        not eax
        and ecx, eax
        mov eax, esp
        and eax, 0xFFFFF000
    cs10:
        cmp ecx, eax
        jb cs20
        mov eax, ecx
        pop ecx
        xchg esp, eax
        mov eax, dword ptr [eax]
        mov dword ptr [esp], eax
        ret
    cs20:
        sub eax, 0x1000
        test dword ptr [eax], eax
        jmp cs10
    }
}
extern "C" __declspec(naked) void _alloca_probe() { __asm jmp _chkstk }

// ---------------------------------------------------------------- 64-bit helpers
extern "C" __declspec(naked) void _allmul() {
    __asm {
        mov eax, [esp+8]
        mov ecx, [esp+16]
        or ecx, eax
        mov ecx, [esp+12]
        jnz hard
        mov eax, [esp+4]
        mul ecx
        ret 16
    hard:
        push ebx
        mul ecx
        mov ebx, eax
        mov eax, [esp+8]
        mul dword ptr [esp+20]
        add ebx, eax
        mov eax, [esp+8]
        mul ecx
        add edx, ebx
        pop ebx
        ret 16
    }
}
extern "C" __declspec(naked) void _allshl() {
    __asm {
        cmp cl, 64
        jae retzero
        cmp cl, 32
        jae more32
        shld edx, eax, cl
        shl eax, cl
        ret
    more32:
        mov edx, eax
        xor eax, eax
        and cl, 31
        shl edx, cl
        ret
    retzero:
        xor eax, eax
        xor edx, edx
        ret
    }
}
extern "C" __declspec(naked) void _allshr() {
    __asm {
        cmp cl, 64
        jae retsign
        cmp cl, 32
        jae more32
        shrd eax, edx, cl
        sar edx, cl
        ret
    more32:
        mov eax, edx
        sar edx, 31
        and cl, 31
        sar eax, cl
        ret
    retsign:
        sar edx, 31
        mov eax, edx
        ret
    }
}
extern "C" __declspec(naked) void _aullshr() {
    __asm {
        cmp cl, 64
        jae retzero
        cmp cl, 32
        jae more32
        shrd eax, edx, cl
        shr edx, cl
        ret
    more32:
        mov eax, edx
        xor edx, edx
        and cl, 31
        shr eax, cl
        ret
    retzero:
        xor eax, eax
        xor edx, edx
        ret
    }
}

