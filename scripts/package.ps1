param(
    [string]$BuildDir = "cmake-build-package",
    [string]$DistDir = "dist\dx11_sync",
    [switch]$SkipBuild,
    [switch]$Zip
)

$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $PSScriptRoot
$BuildPath = Join-Path $Root $BuildDir
$DistPath = Join-Path $Root $DistDir

$QtRoot = "C:\Qt\6.11.1\mingw_64"
$QtBin = Join-Path $QtRoot "bin"
$MingwRoot = "C:\Qt\Tools\mingw1310_64"
$MingwBin = Join-Path $MingwRoot "bin"
$NinjaExe = "C:\Qt\Tools\Ninja\ninja.exe"
$Windeployqt = Join-Path $QtBin "windeployqt.exe"

foreach ($Path in @($QtRoot, $MingwBin, $NinjaExe, $Windeployqt)) {
    if (-not (Test-Path $Path)) {
        throw "Required path not found: $Path"
    }
}

$env:PATH = "$MingwBin;$QtBin;$(Split-Path -Parent $NinjaExe);$env:PATH"

if (-not $SkipBuild) {
    cmake -S $Root -B $BuildPath -G Ninja `
        -DCMAKE_BUILD_TYPE=Release `
        -DCMAKE_C_COMPILER="$MingwBin\gcc.exe" `
        -DCMAKE_CXX_COMPILER="$MingwBin\g++.exe" `
        -DCMAKE_MAKE_PROGRAM="$NinjaExe" `
        -DCMAKE_PREFIX_PATH="$QtRoot"

    cmake --build $BuildPath --config Release
}

$ExePath = Join-Path $BuildPath "dx11_sync.exe"
if (-not (Test-Path $ExePath)) {
    throw "Build output not found: $ExePath"
}

if (Test-Path $DistPath) {
    Remove-Item -LiteralPath $DistPath -Recurse -Force
}
New-Item -ItemType Directory -Path $DistPath | Out-Null

Copy-Item -LiteralPath $ExePath -Destination $DistPath

& $Windeployqt `
    --release `
    --compiler-runtime `
    --qmldir (Join-Path $Root "src\qml") `
    (Join-Path $DistPath "dx11_sync.exe")

if ($Zip) {
    $ZipPath = Join-Path (Split-Path -Parent $DistPath) "dx11_sync-windows-x64.zip"
    if (Test-Path $ZipPath) {
        Remove-Item -LiteralPath $ZipPath -Force
    }
    Compress-Archive -Path (Join-Path $DistPath "*") -DestinationPath $ZipPath
    Write-Host "Package ready: $ZipPath"
} else {
    Write-Host "Package ready: $DistPath"
}
