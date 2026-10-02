"""End-to-end checks. Run from the repository root:  python -m unittest discover -s tests -v

Tests that need GP4.exe, the MSVC x86 toolchain or a completed `gp4re analyze`
skip themselves when those are missing.
"""
from __future__ import annotations

import json
import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

from gp4re import spec  # noqa: E402


def have_gp4() -> bool:
    try:
        from gp4re.paths import find_exe
        return find_exe().exists()
    except SystemExit:
        return False


def have_msvc() -> bool:
    try:
        from gp4re.msvc import msvc_env
        return bool(msvc_env().get("INCLUDE"))
    except Exception:
        return False


def have_analysis() -> bool:
    from gp4re import db
    try:
        return db.connect().execute("SELECT COUNT(*) FROM functions").fetchone()[0] > 0
    except Exception:
        return False


class SpecTests(unittest.TestCase):
    def test_parse_regs(self):
        s = spec.parse("regs(edx:ptr[0x60]:f64, st0:f64[-1..1]) -> (eax, st0, cf); tol=1e-9")
        self.assertEqual(s.conv, "regs")
        self.assertEqual([a.reg for a in s.st_inputs], ["st0"])
        self.assertEqual(s.st_outputs, ["st0"])
        self.assertEqual(s.tol, 1e-9)

    def test_parse_compiled(self):
        s = spec.parse("thiscall(ptr[0x200]:f32, i32[0..21]) -> f32; emu=skip:win32 Sleep")
        self.assertEqual(s.ret, "f32")
        self.assertEqual(s.emu_skip, "win32 Sleep")

    def test_rejects(self):
        for bad in ("cdecl(i32) -> float", "regs(xyz:i32) -> ()", "magic(i32) -> i32", "cdecl(ptr[8]:nope) -> void"):
            with self.assertRaises(spec.SpecError, msg=bad):
                spec.parse(bad)

    def test_spec_cannot_hide_or_misorder_inputs(self):
        for bad in ("regs(st1:f64) -> ()", "regs() -> (st1)",
                    "regs(eax:u32, eax:u32) -> (eax)", "regs() -> (eax, eax)",
                    "regs(edx:f64) -> ()", "regs(st0:ptr[8]) -> ()",
                    "regs(cf:u32) -> ()", "cdecl(ptr[0]) -> void",
                    "cdecl(ptr[8]:layout=../secret) -> void",
                    "cdecl(i32[2..1]) -> void", "cdecl(u8[0..256]) -> void",
                    "cdecl() -> void; cases=0", "cdecl() -> void; maxinsn=0",
                    "cdecl() -> void; tol=-1", "cdecl() -> void; tol=nan",
                    "cdecl() -> void; globals=fuzz:bytes",
                    "cdecl() -> void; cases=20; cases=1"):
            with self.assertRaises(spec.SpecError, msg=bad):
                spec.parse(bad)

    def test_flags_and_sorted_x87_inputs(self):
        s = spec.parse("regs(st1:f64, cf:u32[0..1], st0:f64) -> (sf, of, st0, st1)")
        self.assertEqual([a.reg for a in s.st_inputs], ["st0", "st1"])
        self.assertEqual(s.outs[:2], ["sf", "of"])


class GuardTests(unittest.TestCase):
    def run_guard(self, tool, inp):
        ev = {"tool_name": tool, "tool_input": inp, "cwd": str(ROOT)}
        p = subprocess.run([sys.executable, str(ROOT / "tools" / "hooks" / "guard.py")], input=json.dumps(ev),
                           capture_output=True, text=True, env={**os.environ, "CLAUDE_PROJECT_DIR": str(ROOT)})
        return p.returncode

    def test_paths(self):
        self.assertEqual(self.run_guard("Write", {"file_path": str(ROOT / "build/scratch/00401d80/v1.cpp")}), 0)
        self.assertEqual(self.run_guard("Write", {"file_path": str(ROOT / "src/gp4/c/0051/00510b48.cpp")}), 2)
        self.assertEqual(self.run_guard("Edit", {"file_path": str(ROOT / "Grand-Prix-4/Grand Prix 4/GP4.exe")}), 2)

    def test_bash(self):
        ok = "GP4RE_AGENT=w01 python -m gp4re work try 0x401d80 build/scratch/00401d80/v1.cpp && " \
             "GP4RE_AGENT=w01 python -m gp4re work accept 0x401d80 build/scratch/00401d80/v1.cpp"
        self.assertEqual(self.run_guard("Bash", {"command": ok}), 0)
        for bad in ("git commit -am x", "python -m gp4re work exclude 0x401000-0x402000",
                    "python -c 'print(1)'", "echo x > src/gp4/x.cpp", "rm -rf build",
                    "python -m gp4re work name 0x1 X --evidence y --force", "cp a.cpp include/gp4/a.h"):
            self.assertEqual(self.run_guard("Bash", {"command": bad}), 2, bad)


@unittest.skipUnless(have_gp4() and have_msvc() and have_analysis(), "needs GP4.exe, MSVC x86 and gp4re analyze")
class OracleTests(unittest.TestCase):
    def test_accepted_functions_still_pass(self):
        from gp4re import verify
        for addr in (0x00401D80, 0x00510B48):
            res = verify.run(addr, cases=60)
            self.assertEqual(res["overall"], "pass", json.dumps(res, default=str)[:800])

    def test_oracle_catches_swapped_outputs(self):
        from gp4re import verify
        src = (ROOT / "src/gp4/asm/0040/00401d80.cpp").read_text()
        bad = src.replace("0x28 + 16 * i) = s", "0x28 + 16 * i) = TMP").replace(
            "0x30 + 16 * i) = c", "0x30 + 16 * i) = s").replace("= TMP", "= c")
        self.assertNotEqual(src, bad)
        with tempfile.TemporaryDirectory(dir=ROOT / "build") as td:
            p = Path(td) / "bad.cpp"
            p.write_text(bad)
            res = verify.run(0x00401D80, candidate=p, cases=20)
        self.assertEqual(res["overall"], "fail")
        self.assertIn("emu", res["failed"])


@unittest.skipUnless(have_msvc(), "needs MSVC x86")
class NativeAdapterTest(unittest.TestCase):
    def test_regs_adapter_and_call_regs(self):
        from gp4re import msvc
        from gp4re.paths import build_dir
        env = msvc.msvc_env()
        out = build_dir() / "native"
        out.mkdir(parents=True, exist_ok=True)
        srcs = [ROOT / "tests/native/test_adapter.cpp", ROOT / "hook/src/adapter.cpp",
                ROOT / "src/gp4/runtime/gp4_rt.cpp"]
        exe = out / "test_adapter.exe"
        rc, log = msvc._run(["cl", "/nologo", "/O2", "/arch:IA32", "/MT", "/EHsc", "/std:c++17",
                             f"/I{ROOT / 'include'}", f"/Fe{exe}", f"/Fo{out}\\", *map(str, srcs)], env, ROOT)
        self.assertEqual(rc, 0, log[-2000:])
        r = subprocess.run([str(exe)], capture_output=True, text=True)
        self.assertEqual(r.returncode, 0, r.stdout)
        self.assertIn("ALL PASS", r.stdout)


if __name__ == "__main__":
    unittest.main()
