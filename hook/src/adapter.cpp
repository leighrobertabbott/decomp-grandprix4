// adapter.cpp - bridges hand-written-asm register conventions to reconstructions
// written as `void __cdecl fn(gp4::Regs*)`. Used by the hook DLL; unit-tested by
// tests/native/test_adapter.cpp.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include "adapter.h"

namespace {
struct HookInfo {
    const void* impl;   // void __cdecl impl(gp4::Regs*)
    int32_t st_in;
    int32_t st_out;
};

// entry: [esp] = HookInfo*, [esp+4] = caller's return address; machine state = original inputs
__declspec(naked) void regs_adapter() {
    __asm {
        lea esp, [esp-0x68]         // preserve incoming EFLAGS before saving them
        mov [esp+0x00], eax
        mov [esp+0x04], ebx
        mov [esp+0x08], ecx
        mov [esp+0x0C], edx
        mov [esp+0x10], esi
        mov [esp+0x14], edi
        mov [esp+0x18], ebp
        pushfd
        pop dword ptr [esp+0x1C]
        mov eax, [esp+0x68]          // HookInfo*
        mov ecx, [eax+4]             // st_in: pop x87 inputs into Regs.st[0..]
        xor edx, edx
    l_in:
        cmp edx, ecx
        jge d_in
        fstp qword ptr [esp+0x20+edx*8]
        inc edx
        jmp l_in
    d_in:
        mov [esp+0x60], ecx
        mov ecx, [eax+8]
        mov [esp+0x64], ecx
        mov ecx, esp
        push ecx
        call dword ptr [eax]
        add esp, 4
        mov eax, [esp+0x68]
        mov ecx, [eax+8]             // st_out: push Regs.st[n-1] .. st[0]
    l_out:
        test ecx, ecx
        jz d_out
        dec ecx
        fld qword ptr [esp+0x20+ecx*8]
        jmp l_out
    d_out:
        push dword ptr [esp+0x1C]
        popfd
        mov ebx, [esp+0x04]
        mov ecx, [esp+0x08]
        mov edx, [esp+0x0C]
        mov esi, [esp+0x10]
        mov edi, [esp+0x14]
        mov ebp, [esp+0x18]
        mov eax, [esp+0x00]
        lea esp, [esp+0x6C]          // Regs + HookInfo* (lea: must not touch the restored flags)
        ret
    }
}

uint8_t* g_thunks = nullptr;
size_t g_thunk_used = 0;

}  // namespace

uint32_t make_regs_thunk(const gp4::ImplRecord& r) {
    // "regs(... st0:..., st1:...) -> (eax, st0)": count st inputs / outputs from the spec
    const char* spec = r.spec;
    const char* arrow = strstr(spec, "->");
    int st_in = 0, st_out = 0;
    for (const char* p = spec; p && *p && p < arrow; ++p)
        if (p[0] == 's' && p[1] == 't' && p[2] >= '0' && p[2] <= '7' && p[3] == ':') ++st_in;
    for (const char* p = arrow; p && *p && *p != ';'; ++p)
        if (p[0] == 's' && p[1] == 't' && p[2] >= '0' && p[2] <= '7') ++st_out;
    if (!g_thunks) g_thunks = (uint8_t*)VirtualAlloc(nullptr, 1 << 20, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    HookInfo* info = new HookInfo{r.impl, st_in, st_out};
    uint8_t* t = g_thunks + g_thunk_used;
    t[0] = 0x68;                                   // push imm32 (HookInfo*)
    uint32_t v = (uint32_t)info;
    memcpy(t + 1, &v, 4);
    t[5] = 0xE9;                                   // jmp regs_adapter
    int32_t rel = (int32_t)((uint32_t)&regs_adapter - ((uint32_t)t + 10));
    memcpy(t + 6, &rel, 4);
    g_thunk_used += 16;
    return (uint32_t)t;
}

