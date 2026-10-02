"""PE loading and fingerprinting (``gp4re ingest``).

Loads the image exactly as the Windows loader would place it (GP4.exe has no
relocations, so its fixed base 0x400000 is the only base it ever runs at) and
classifies sections into *in scope* (the game's own code/data) and *out of
scope* (anything that is not a standard MSVC section, e.g. third-party wrapper
stubs). Out-of-scope sections are never disassembled, emulated or patched.
"""
from __future__ import annotations

import collections
import datetime as _dt
import hashlib
import json
import math
import struct
from dataclasses import dataclass, field
from pathlib import Path

import pefile

STANDARD_SECTIONS = {".text", ".rdata", ".data", ".data1", ".rsrc", ".reloc", ".idata",
                     ".edata", ".tls", ".bss", ".CRT", ".pdata"}

# Rich-header product ids (public comp.id tables, e.g. dishather/richprint) for the
# VC6 / VS.NET-2002 era, plus working hypotheses for GP4's entries. Hypotheses are
# settled empirically (calibration against known functions), not asserted.
RICH_PRODIDS = {
    0x00: "Unmarked objects", 0x01: "Import0 (imported symbols)", 0x02: "Linker510",
    0x03: "Cvtomf510", 0x04: "Linker600", 0x05: "Cvtomf600", 0x06: "Cvtres500",
    0x07: "Utc11_Basic", 0x08: "Utc11_C", 0x09: "Utc12_Basic", 0x0A: "Utc12_C (VC6 C)",
    0x0B: "Utc12_CPP (VC6 C++)", 0x0C: "AliasObj60", 0x0D: "VisualBasic60",
    0x0E: "Masm613 (MASM 6.13)", 0x0F: "Masm710", 0x10: "Linker511", 0x11: "Cvtomf511",
    0x12: "Masm614 (MASM 6.14)", 0x13: "Linker512", 0x14: "Cvtomf512",
    0x15: "Utc12_C_Std", 0x16: "Utc12_CPP_Std", 0x17: "Utc12_C_Book", 0x18: "Utc12_CPP_Book",
    0x19: "Implib700", 0x1A: "Cvtomf700", 0x1B: "Utc13_Basic", 0x1C: "Utc13_C (VC7 C)",
    0x1D: "Utc13_CPP (VC7 C++)", 0x1E: "Linker610", 0x1F: "Cvtomf610", 0x20: "Linker601",
    0x21: "Cvtomf601", 0x22: "Utc12_1_Basic", 0x23: "Utc12_1_C", 0x24: "Utc12_1_CPP",
    0x25: "Linker620", 0x26: "Cvtomf620", 0x27: "AliasObj70", 0x28: "Linker621",
    0x29: "Cvtomf621", 0x2A: "Masm615", 0x2B: "Utc13_LTCG_C", 0x2C: "Utc13_LTCG_CPP",
    0x2D: "Masm620", 0x2E: "ILAsm100", 0x2F: "Utc12_2_Basic",
    0x30: "Utc12_2_C (VC6 SP5 Processor Pack C)", 0x31: "Utc12_2_CPP (VC6 SP5 Processor Pack C++)",
}
RICH_HYPOTHESES = {
    (0x31, 9044): "the game's own newer C++ (largest group)",
    (0x0B, 8966): "C++ from an older VC6 service pack",
    (0x0A, 8047): "VC6 C runtime objects (the CRT sits at the end of .text)",
    (0x0B, 8047): "VC6 C++ runtime objects",
    (0x0E, 7299): "MASM 6.13 objects: CRT asm helpers and/or game assembly",
    (0x12, 8444): "a single newer MASM object",
    (0x1D, 9178): "pre-release VC7 C++: the DirectX 8 SDK static D3DX library",
    (0x1C, 9178): "pre-release VC7 C: DirectX SDK static library",
    (0x19, 9210): "DirectX SDK import libraries",
}


def entropy(b: bytes) -> float:
    if not b:
        return 0.0
    n = len(b)
    return -sum(c / n * math.log2(c / n) for c in collections.Counter(b).values())


@dataclass
class Section:
    name: str
    va: int
    vsize: int
    raw_size: int
    characteristics: int
    entropy: float
    in_scope: bool

    @property
    def end(self) -> int:
        return self.va + max(self.vsize, self.raw_size)

    @property
    def executable(self) -> bool:
        return bool(self.characteristics & 0x20000000) or bool(self.characteristics & 0x20)

    @property
    def writable(self) -> bool:
        return bool(self.characteristics & 0x80000000)

    def contains(self, va: int) -> bool:
        return self.va <= va < self.end


@dataclass
class Binary:
    path: Path
    data: bytes
    base: int
    entry: int
    image: bytearray
    sections: list[Section]
    imports: dict[int, str] = field(default_factory=dict)  # IAT slot VA -> "dll!name"

    # ---------------------------------------------------------------- loading
    @classmethod
    def load(cls, path: str | Path) -> "Binary":
        path = Path(path)
        data = path.read_bytes()
        pe = pefile.PE(data=data)
        oh = pe.OPTIONAL_HEADER
        base = oh.ImageBase
        image = bytearray(oh.SizeOfImage)
        image[: oh.SizeOfHeaders] = data[: oh.SizeOfHeaders]
        sections = []
        for s in pe.sections:
            name = s.Name.rstrip(b"\0").decode("latin1")
            raw = s.get_data()[: s.SizeOfRawData]
            image[s.VirtualAddress: s.VirtualAddress + len(raw)] = raw
            sections.append(Section(
                name=name, va=base + s.VirtualAddress, vsize=s.Misc_VirtualSize,
                raw_size=s.SizeOfRawData, characteristics=s.Characteristics,
                entropy=round(entropy(raw), 3), in_scope=name in STANDARD_SECTIONS,
            ))
        imports: dict[int, str] = {}
        for e in getattr(pe, "DIRECTORY_ENTRY_IMPORT", []):
            dll = e.dll.decode("latin1").lower()
            for imp in e.imports:
                nm = imp.name.decode("latin1") if imp.name else f"#{imp.ordinal}"
                imports[imp.address] = f"{dll}!{nm}"
        b = cls(path=path, data=data, base=base, entry=base + oh.AddressOfEntryPoint,
                image=image, sections=sections, imports=imports)
        b._pe = pe
        return b

    # ---------------------------------------------------------------- access
    def section(self, name: str) -> Section | None:
        return next((s for s in self.sections if s.name == name), None)

    def section_of(self, va: int) -> Section | None:
        for s in self.sections:
            if s.contains(va):
                return s
        return None

    def in_image(self, va: int) -> bool:
        return self.base <= va < self.base + len(self.image)

    def read(self, va: int, n: int) -> bytes:
        o = va - self.base
        return bytes(self.image[o: o + n])

    def u32(self, va: int) -> int:
        o = va - self.base
        return struct.unpack_from("<I", self.image, o)[0]

    def code_sections(self) -> list[Section]:
        return [s for s in self.sections if s.in_scope and s.executable]

    def data_sections(self) -> list[Section]:
        return [s for s in self.sections if s.in_scope and not s.executable and s.name != ".rsrc"]

    def cstring(self, va: int, maxlen: int = 256) -> str | None:
        s = self.section_of(va)
        if not s or s.executable:
            return None
        raw = self.read(va, maxlen)
        end = raw.find(b"\0")
        if end < 3:
            return None
        txt = raw[:end]
        if all(32 <= c < 127 or c in (9, 10, 13) for c in txt):
            return txt.decode("latin1")
        return None

    # ---------------------------------------------------------------- fingerprint
    def rich(self) -> list[dict]:
        rh = self._pe.parse_rich_header()
        if not rh:
            return []
        out = []
        v = rh["values"]
        for i in range(0, len(v), 2):
            prodid, build, count = v[i] >> 16, v[i] & 0xFFFF, v[i + 1]
            out.append({"prodid": prodid, "build": build, "count": count,
                        "product": RICH_PRODIDS.get(prodid, f"prodid {prodid:#x}"),
                        "hypothesis": RICH_HYPOTHESES.get((prodid, build), "")})
        return out

    def fingerprint(self) -> dict:
        pe = self._pe
        fh, oh = pe.FILE_HEADER, pe.OPTIONAL_HEADER
        imports_by_dll: dict[str, list[str]] = collections.defaultdict(list)
        for name in self.imports.values():
            dll, fn = name.split("!", 1)
            imports_by_dll[dll].append(fn)
        text = self.section(".text")
        warnings = []
        if text and text.entropy > 7.2:
            warnings.append(".text entropy > 7.2: code looks packed/encrypted. This pipeline only "
                            "analyses plain code; supply an install whose code is readable.")
        oos = [s.name for s in self.sections if not s.in_scope]
        if oos:
            warnings.append(f"out-of-scope (non-game) sections excluded from all analysis: {oos}")
        if not any(s.contains(self.entry) and s.in_scope for s in self.sections):
            warnings.append("entry point is outside the in-scope sections")
        return {
            "file": str(self.path.name),
            "size": len(self.data),
            "sha256": hashlib.sha256(self.data).hexdigest(),
            "md5": hashlib.md5(self.data).hexdigest(),
            "machine": hex(fh.Machine),
            "timestamp_raw": fh.TimeDateStamp,
            "timestamp_utc": _dt.datetime.fromtimestamp(fh.TimeDateStamp, _dt.timezone.utc).isoformat(),
            "image_base": hex(self.base),
            "entry": hex(self.entry),
            "size_of_image": hex(oh.SizeOfImage),
            "linker_version": f"{oh.MajorLinkerVersion}.{oh.MinorLinkerVersion}",
            "subsystem": oh.Subsystem,
            "has_relocations": hasattr(pe, "DIRECTORY_ENTRY_BASERELOC"),
            "sections": [
                {"name": s.name, "va": hex(s.va), "vsize": hex(s.vsize), "raw_size": hex(s.raw_size),
                 "entropy": s.entropy, "exec": s.executable, "write": s.writable, "in_scope": s.in_scope}
                for s in self.sections
            ],
            "rich_header": self.rich(),
            "imports": {k: sorted(v) for k, v in sorted(imports_by_dll.items())},
            "warnings": warnings,
        }


def ingest(exe: Path, out_dir: Path) -> dict:
    b = Binary.load(exe)
    fp = b.fingerprint()
    (out_dir / "binary.json").write_text(json.dumps(fp, indent=2))
    return fp
