$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$test = $false
foreach ($arg in $args) {
    switch ($arg) {
        "--test" { $test = $true; continue }
        "-test" { $test = $true; continue }
        "-t" { $test = $true; continue }
        default {
            Write-Error "Unknown argument: $arg. Supported flags: --test, -test, -t"
            exit 2
        }
    }
}

$repoRoot = (Resolve-Path "$PSScriptRoot/..").Path
Set-Location $repoRoot

$clangFormatCommand = Get-Command clang-format -ErrorAction SilentlyContinue
if (-not $clangFormatCommand) {
    Write-Error "Unable to find clang-format in PATH."
    exit 127
}

$clangFormat = $clangFormatCommand.Source
$versionOutput = & $clangFormat --version
if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($versionOutput)) {
    Write-Error "Unable to run clang-format --version. Make sure clang-format is in PATH."
    exit 127
}

$versionMatch = [regex]::Match($versionOutput, "clang-format version ([0-9]+(\.[0-9]+)*)")
if ($versionMatch.Success) {
    Write-Host "Running using clang-format $($versionMatch.Groups[1].Value)"
}
else {
    Write-Host "Running using $versionOutput"
}

$files = @(git ls-files --cached --modified --others --exclude-standard --deduplicate -- "*.h" "*.cpp" "*.c" "*.mm" "*.m")

if ($files.Count -gt 0) {
    $batchSize = 100
    for ($i = 0; $i -lt $files.Count; $i += $batchSize) {
        $end = [Math]::Min($i + $batchSize - 1, $files.Count - 1)
        & $clangFormat -style=file -i $files[$i..$end]
    }
}

if ($test) {
    $baseSha = "$(git rev-parse HEAD)".Trim()
    git diff $baseSha
    if ("$(git diff $baseSha)" -ne "") {
        exit 1
    }
}
