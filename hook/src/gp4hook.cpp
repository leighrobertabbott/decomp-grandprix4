// gp4hook - in-game verification harness (tier T2), loaded as dinput8.dll from a
// COPY of the game (runtime/GP4, prepared by `gp4re runtime install`). The original
// install in Grand-Prix-4/ is never touched.
//
// What it does, driven by gp4hook.ini (written by gp4re):
//   [hooks]      0x00401d80=1     -> run the reconstruction instead of the original
//                                    (jmp patch; "regs" functions go through an adapter)
//   [coverage]   enabled=1        -> one-shot INT3 on every game function start; logs the
//                                    first hit of each to coverage.log. F9 writes a phase
//                                    marker ("--- mark N ---") so boot/menu/session/race
//                                    traces can be told apart (gp4re goal import).
//   [telemetry]  callsite=0x...   -> redirects one `call rel32` (e.g. the sim tick in the
//                                    main loop) through a stub that calls the original and
//                                    then samples the [watch] expressions to telemetry.csv
//   [watch]      name=f32@0x6a1234 or name=f64@[0x6a1234]+0x2c  (pointer deref then offset)
//
// Patterned on re3/gta-reversed incremental replacement: the game keeps running
// on original code everywhere a reconstruction is not enabled.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <string>
#include <unordered_map>

#include <gp4/gp4.h>
#include "adapter.h"

extern "C" const gp4::ImplRecord gp4_impl_begin;
extern "C" const gp4::ImplRecord gp4_impl_end;

namespace {

char g_dir[MAX_PATH];
FILE* g_log = nullptr;

void logf(const char* fmt, ...) {
    if (!g_log) return;
    va_list ap;
    va_start(ap, fmt);
    vfprintf(g_log, fmt, ap);
    va_end(ap);
    fflush(g_log);
}

std::string ini_path() { return std::string(g_dir) + "\\gp4hook.ini"; }

std::vector<std::pair<std::string, std::string>> ini_section(const char* sec) {
    std::vector<std::pair<std::string, std::string>> out;
    std::vector<char> buf(1 << 20);
    DWORD n = GetPrivateProfileSectionA(sec, buf.data(), (DWORD)buf.size(), ini_path().c_str());
    for (const char* p = buf.data(); p < buf.data() + n && *p; p += strlen(p) + 1) {
        std::string line(p);
        size_t eq = line.find('=');
        if (eq == std::string::npos || line[0] == ';') continue;
        std::string v = line.substr(eq + 1);
        size_t sc = v.find(';');
        if (sc != std::string::npos) v = v.substr(0, sc);
        while (!v.empty() && (v.back() == ' ' || v.back() == '\t')) v.pop_back();
        out.emplace_back(line.substr(0, eq), v);
    }
    return out;
}

bool write_code(uint32_t addr, const void* bytes, size_t n) {
    DWORD old;
    if (!VirtualProtect((void*)addr, n, PAGE_EXECUTE_READWRITE, &old)) return false;
    memcpy((void*)addr, bytes, n);
    VirtualProtect((void*)addr, n, old, &old);
    FlushInstructionCache(GetCurrentProcess(), (void*)addr, n);
    return true;
}

void write_jmp(uint32_t from, uint32_t to) {
    uint8_t b[5] = {0xE9};
    int32_t rel = (int32_t)(to - (from + 5));
    memcpy(b + 1, &rel, 4);
    write_code(from, b, 5);
}

// regs adapter lives in adapter.cpp (shared with tests/native)

void install_hooks() {
    std::unordered_map<uint32_t, const gp4::ImplRecord*> reg;
    for (const gp4::ImplRecord* r = &gp4_impl_begin; r < &gp4_impl_end; ++r)
        if (r->addr && r->impl) reg[r->addr] = r;
    int n = 0;
    for (auto& kv : ini_section("hooks")) {
        uint32_t addr = strtoul(kv.first.c_str(), nullptr, 16);
        if (atoi(kv.second.c_str()) != 1) continue;
        auto it = reg.find(addr);
        if (it == reg.end()) { logf("hook %08x: no reconstruction compiled in\n", addr); continue; }
        const gp4::ImplRecord* r = it->second;
        uint32_t target = strncmp(r->spec, "regs", 4) == 0 ? make_regs_thunk(*r) : (uint32_t)r->impl;
        write_jmp(addr, target);
        ++n;
        logf("hook %08x -> %s [%s]\n", addr, r->name, r->spec);
    }
    logf("%d hooks installed (%u reconstructions compiled in)\n", n, (unsigned)reg.size());
}

// ------------------------------------------------------------------ coverage
std::unordered_map<uint32_t, uint8_t> g_bp;     // addr -> original byte
CRITICAL_SECTION g_cov_lock;
FILE* g_cov = nullptr;
int g_mark = 0;

LONG CALLBACK on_exception(EXCEPTION_POINTERS* e) {
    if (e->ExceptionRecord->ExceptionCode != EXCEPTION_BREAKPOINT) return EXCEPTION_CONTINUE_SEARCH;
    uint32_t at = (uint32_t)e->ExceptionRecord->ExceptionAddress;
    EnterCriticalSection(&g_cov_lock);
    auto it = g_bp.find(at);
    if (it == g_bp.end()) { LeaveCriticalSection(&g_cov_lock); return EXCEPTION_CONTINUE_SEARCH; }
    write_code(at, &it->second, 1);
    g_bp.erase(it);
    if (g_cov) fprintf(g_cov, "0x%08x %lu\n", at, GetTickCount());
    LeaveCriticalSection(&g_cov_lock);
    e->ContextRecord->Eip = at;
    return EXCEPTION_CONTINUE_EXECUTION;
}

DWORD WINAPI marker_thread(void*) {
    for (;;) {
        if (GetAsyncKeyState(VK_F9) & 1) {
            EnterCriticalSection(&g_cov_lock);
            if (g_cov) { fprintf(g_cov, "--- mark %d ---\n", ++g_mark); fflush(g_cov); }
            LeaveCriticalSection(&g_cov_lock);
        }
        Sleep(50);
        if (g_cov) { EnterCriticalSection(&g_cov_lock); fflush(g_cov); LeaveCriticalSection(&g_cov_lock); }
    }
}

void install_coverage() {
    if (GetPrivateProfileIntA("coverage", "enabled", 0, ini_path().c_str()) != 1) return;
    InitializeCriticalSection(&g_cov_lock);
    std::string list = std::string(g_dir) + "\\coverage_funcs.txt";
    FILE* f = fopen(list.c_str(), "r");
    if (!f) { logf("coverage: %s missing\n", list.c_str()); return; }
    g_cov = fopen((std::string(g_dir) + "\\coverage.log").c_str(), "a");
    fprintf(g_cov, "--- session start %lu ---\n", GetTickCount());
    AddVectoredExceptionHandler(1, on_exception);
    char line[64];
    uint8_t cc = 0xCC;
    while (fgets(line, sizeof line, f)) {
        uint32_t a = strtoul(line, nullptr, 16);
        if (!a) continue;
        g_bp[a] = *(uint8_t*)a;
        write_code(a, &cc, 1);
    }
    fclose(f);
    CreateThread(nullptr, 0, marker_thread, nullptr, 0, nullptr);
    logf("coverage: %u breakpoints armed\n", (unsigned)g_bp.size());
}

// ------------------------------------------------------------------ telemetry
struct Watch { std::string name, type; uint32_t addr; bool deref; uint32_t off; };
std::vector<Watch> g_watch;
FILE* g_tel = nullptr;
uint32_t g_tick_target = 0;
unsigned long g_tick = 0;

void sample() {
    if (!g_tel) return;
    uint16_t cw;
    __asm fnstcw cw
    fprintf(g_tel, "%lu,%lu,0x%04x", g_tick++, GetTickCount(), cw);
    for (auto& w : g_watch) {
        uint32_t a = w.addr;
        if (w.deref) { a = *(uint32_t*)a; if (!a) { fprintf(g_tel, ","); continue; } a += w.off; }
        if (w.type == "f32") fprintf(g_tel, ",%.9g", *(float*)a);
        else if (w.type == "f64") fprintf(g_tel, ",%.17g", *(double*)a);
        else if (w.type == "u8") fprintf(g_tel, ",%u", *(uint8_t*)a);
        else if (w.type == "i16") fprintf(g_tel, ",%d", *(int16_t*)a);
        else fprintf(g_tel, ",%d", *(int32_t*)a);
    }
    fprintf(g_tel, "\n");
    if ((g_tick & 63) == 0) fflush(g_tel);
}

uint32_t g_saved_ret = 0;   // single level: the patched call site must not be re-entered

// Replaces the patched `call rel32`. Takes the caller's return address off the
// stack so the original sees an identical frame (correct for cdecl and stdcall
// targets), calls it, samples with all registers/flags preserved, then returns
// to the caller. A float returned in st0 survives: sample() leaves the x87
// stack balanced.
__declspec(naked) void tick_stub() {
    __asm {
        pop g_saved_ret
        call g_tick_target
        pushad
        pushfd
        call sample
        popfd
        popad
        push g_saved_ret
        ret
    }
}

void install_telemetry() {
    char buf[64];
    GetPrivateProfileStringA("telemetry", "callsite", "", buf, sizeof buf, ini_path().c_str());
    uint32_t site = strtoul(buf, nullptr, 16);
    if (!site) return;
    if (*(uint8_t*)site != 0xE8) { logf("telemetry: %08x is not a call rel32\n", site); return; }
    g_tick_target = site + 5 + *(int32_t*)(site + 1);
    for (auto& kv : ini_section("watch")) {
        Watch w{kv.first};
        const std::string& v = kv.second;           // type@addr | type@[addr]+off
        size_t at = v.find('@');
        if (at == std::string::npos) continue;
        w.type = v.substr(0, at);
        std::string rest = v.substr(at + 1);
        w.deref = rest[0] == '[';
        w.addr = strtoul(rest.c_str() + (w.deref ? 1 : 0), nullptr, 16);
        size_t plus = rest.find("]+");
        w.off = plus == std::string::npos ? 0 : strtoul(rest.c_str() + plus + 2, nullptr, 16);
        g_watch.push_back(w);
    }
    g_tel = fopen((std::string(g_dir) + "\\telemetry.csv").c_str(), "w");
    fprintf(g_tel, "tick,ms,fpucw");
    for (auto& w : g_watch) fprintf(g_tel, ",%s", w.name.c_str());
    fprintf(g_tel, "\n");
    uint8_t b[5] = {0xE8};
    int32_t rel = (int32_t)((uint32_t)&tick_stub - (site + 5));
    memcpy(b + 1, &rel, 4);
    write_code(site, b, 5);
    logf("telemetry: call site %08x -> stub (original target %08x), %u watches\n", site, g_tick_target,
         (unsigned)g_watch.size());
}

// ------------------------------------------------------------------ proxy
typedef HRESULT(WINAPI* DI8Create)(HINSTANCE, DWORD, const GUID&, void**, void*);
DI8Create g_real = nullptr;
bool g_init = false;

void init_once() {
    if (g_init) return;
    g_init = true;
    g_log = fopen((std::string(g_dir) + "\\gp4hook.log").c_str(), "w");
    uint16_t cw;
    __asm fnstcw cw
    logf("gp4hook loaded; x87 control word at DirectInput8Create time: 0x%04x\n", cw);
    install_hooks();
    install_coverage();
    install_telemetry();
}

}  // namespace

extern "C" HRESULT WINAPI DirectInput8Create(HINSTANCE h, DWORD v, const GUID& r, void** o, void* u) {
    if (!g_real) {
        char sys[MAX_PATH];
        GetSystemDirectoryA(sys, MAX_PATH);           // SysWOW64 for this 32-bit process
        strcat_s(sys, "\\dinput8.dll");
        HMODULE m = LoadLibraryA(sys);
        g_real = m ? (DI8Create)GetProcAddress(m, "DirectInput8Create") : nullptr;
    }
    init_once();
    return g_real ? g_real(h, v, r, o, u) : E_FAIL;
}

BOOL WINAPI DllMain(HINSTANCE h, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(h);
        GetModuleFileNameA(h, g_dir, MAX_PATH);
        char* slash = strrchr(g_dir, '\\');
        if (slash) *slash = 0;
    }
    return TRUE;
}
