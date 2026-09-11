$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$testDirectory = Join-Path ([IO.Path]::GetTempPath()) "mlfq-test-$([Guid]::NewGuid())"
$executable = Join-Path $testDirectory 'mlfq_test.exe'
$input = @"
4
20
0
8
1
4
2
9
3
5
"@

function Normalize-Text($path) {
    return ((Get-Content -Raw $path) -replace "`r`n", "`n").TrimEnd()
}

function Assert-FileMatches($actualPath, $expectedPath) {
    $actual = Normalize-Text $actualPath
    $expected = Normalize-Text $expectedPath

    if ($actual -ne $expected) {
        Write-Host "FAIL: $([IO.Path]::GetFileName($actualPath))"
        Write-Host "Resultado esperado: $expectedPath"
        Write-Host "Resultado obtenido: $actualPath"
        return $false
    }

    Write-Host "PASS: $([IO.Path]::GetFileName($actualPath))"
    return $true
}

Push-Location $root
try {
    New-Item -ItemType Directory -Path $testDirectory | Out-Null

    & gcc -std=c99 -Wall -Wextra -pedantic main.c process.c queue.c scheduler.c -o $executable
    if ($LASTEXITCODE -ne 0) {
        throw 'La compilacion fallo.'
    }

    Push-Location $testDirectory
    $input | & $executable | Out-Null
    if ($LASTEXITCODE -ne 0) {
        throw 'La ejecucion fallo.'
    }

    $resultsPassed = Assert-FileMatches (Join-Path $testDirectory 'results.csv') (Join-Path $PSScriptRoot 'expected_results.csv')
    $schedulePassed = Assert-FileMatches (Join-Path $testDirectory 'schedule.txt') (Join-Path $PSScriptRoot 'expected_schedule.txt')

    if ($resultsPassed -and $schedulePassed) {
        Write-Host 'PASS: escenario completo'
        exit 0
    }

    Write-Host 'FAIL: escenario completo'
    exit 1
}
finally {
    while ((Get-Location).Path -ne $root) {
        Pop-Location
    }

    Pop-Location
    Remove-Item -Force $executable -ErrorAction SilentlyContinue
    Remove-Item -Recurse -Force $testDirectory -ErrorAction SilentlyContinue
}
