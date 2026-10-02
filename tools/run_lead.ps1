<#
.SYNOPSIS
  Unattended GP4 swarm: runs the lead headless, session after session, until
  state\STOP exists, the session budget cap is hit repeatedly, or -Sessions runs out.

.EXAMPLE
  powershell -ExecutionPolicy Bypass -File tools\run_lead.ps1 -Sessions 6 -BudgetUsd 40 -Goal core

.NOTES
  The lead spawns gp4-worker sub-agents with the Agent tool inside each session
  (cheap: ~12K tokens of context per worker, vs ~60K+ per headless `claude -p`
  worker - Thief3-Decomp's measurement). Workers are fenced by
  tools\hooks\guard.py. Create state\STOP to end after the current session.
#>
param(
  [int]$Sessions = 4,
  [double]$BudgetUsd = 25,
  [string]$Goal = "",
  [int]$MaxWorkers = 12,
  [string]$Model = "opus"
)
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

$claude = (Get-Command claude -ErrorAction SilentlyContinue).Source
if (-not $claude) {
  $claude = Get-ChildItem "$env:APPDATA\Claude\claude-code" -Recurse -Filter claude.exe -ErrorAction SilentlyContinue |
    Sort-Object LastWriteTime -Descending | Select-Object -First 1 -ExpandProperty FullName
}
if (-not $claude) { throw "Claude Code CLI not found (install it or put claude on PATH)" }

New-Item -ItemType Directory -Force logs | Out-Null
$allowed = @(
  "Agent", "Read", "Glob", "Grep", "Write", "Edit",
  "Bash(python -m gp4re:*)", "Bash(GP4RE_AGENT=* python -m gp4re:*)",
  "Bash(git status:*)", "Bash(git diff:*)", "Bash(git log:*)", "Bash(git add:*)", "Bash(git commit:*)"
) -join " "

for ($i = 1; $i -le $Sessions; $i++) {
  if (Test-Path "state\STOP") { Write-Host "state\STOP found - stopping"; break }
  $stamp = Get-Date -Format "yyyyMMdd-HHmmss"
  $log = "logs\lead-$stamp.jsonl"
  Write-Host "[$stamp] lead session $i/$Sessions (budget `$$BudgetUsd) -> $log"
  & $claude -p "/gp4-lead $Goal $MaxWorkers" `
      --model $Model --permission-mode acceptEdits --allowedTools $allowed `
      --max-budget-usd $BudgetUsd --output-format stream-json --verbose 2>&1 |
    Out-File -Encoding utf8 $log
  python -m gp4re status
  Start-Sleep -Seconds 5
}
python -m gp4re export
