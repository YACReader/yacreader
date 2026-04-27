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

$extensions = @("*.h", "*.cpp", "*.c", "*.mm", "*.m")
$files = Get-ChildItem -Path . -Recurse -File | Where-Object {
    $name = $_.Name
    foreach ($ext in $extensions) {
        if ($name -clike $ext) {
            return $true
        }
    }
    return $false
}

foreach ($file in $files) {
    & $clangFormat -style=file -i $file.FullName
}

if ($test) {
    $baseSha = "$(git rev-parse HEAD)".Trim()
    git diff $baseSha
    if ("$(git diff $baseSha)" -ne "") {
        exit 1
    }
}
