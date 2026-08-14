# Install the latest BennuGD64 release into $HOME\bennugd and configure the user environment.
# Usage:
#   irm https://raw.githubusercontent.com/humbertodias/BennuGD64/main/scripts/install.ps1 | iex
[CmdletBinding()]
param(
    [string]$Repo = $(if ($env:BENNUGD_REPO) { $env:BENNUGD_REPO } else { "humbertodias/BennuGD64" }),
    [string]$InstallDir = $(if ($env:BENNUGD_HOME) { $env:BENNUGD_HOME } else { (Join-Path $HOME "bennugd") }),
    [string]$Version = $(if ($env:BENNUGD_VERSION) { $env:BENNUGD_VERSION } else { "latest" }),
    [string]$Linkage = $(if ($env:BENNUGD_LINKAGE) { $env:BENNUGD_LINKAGE } else { "static" })
)

$ErrorActionPreference = "Stop"

function Write-Info([string]$Message) {
    Write-Host "  $Message"
}

function Resolve-Tag {
    if ($Version -ne "latest") {
        return $Version
    }

    $release = Invoke-RestMethod -Uri "https://api.github.com/repos/$Repo/releases/latest"
    if (-not $release.tag_name) {
        throw "Could not determine latest release tag"
    }
    return $release.tag_name
}

function Resolve-Arch {
    switch -Regex ([System.Runtime.InteropServices.RuntimeInformation]::OSArchitecture.ToString()) {
        "X64" { return "x86_64" }
        "Arm64" { return "arm64" }
        default { throw "Unsupported architecture: $_" }
    }
}

Write-Host "BennuGD64 installer" -ForegroundColor White
$tag = Resolve-Tag
$arch = Resolve-Arch
$platform = "windows"
if ($Linkage -notin @("static", "shared")) {
    throw "BENNUGD_LINKAGE must be static or shared (got: $Linkage)"
}
$asset = "bennugd64-$tag-$platform-$arch-$Linkage.zip"
$url = "https://github.com/$Repo/releases/download/$tag/$asset"

Write-Info "Version : $tag"
Write-Info "Platform: $platform/$arch"
Write-Info "Linkage : $Linkage"
Write-Info "Install : $InstallDir"
Write-Host ""

$tmp = Join-Path ([System.IO.Path]::GetTempPath()) ("bennugd-install-" + [guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Path $tmp | Out-Null

try {
    $zipPath = Join-Path $tmp $asset
    Write-Info "Downloading $asset"
    try {
        Invoke-WebRequest -Uri $url -OutFile $zipPath
    } catch {
        $asset = "bennugd64-$tag-$platform-$arch.zip"
        $url = "https://github.com/$Repo/releases/download/$tag/$asset"
        Write-Info "Retrying legacy asset $asset"
        Invoke-WebRequest -Uri $url -OutFile $zipPath
    }

    $extractDir = Join-Path $tmp "extract"
    Expand-Archive -LiteralPath $zipPath -DestinationPath $extractDir -Force

    $extracted = Get-ChildItem -Path $extractDir -Directory | Select-Object -First 1
    if (-not $extracted) {
        throw "Archive did not contain an install directory"
    }

    if (Test-Path $InstallDir) {
        Remove-Item -Recurse -Force $InstallDir
    }
    New-Item -ItemType Directory -Path (Split-Path -Parent $InstallDir) -Force | Out-Null
    Move-Item -Path $extracted.FullName -Destination $InstallDir
}
finally {
    Remove-Item -Recurse -Force $tmp -ErrorAction SilentlyContinue
}

[Environment]::SetEnvironmentVariable("BENNUGD_HOME", $InstallDir, "User")

$userPath = [Environment]::GetEnvironmentVariable("Path", "User")
if (-not $userPath) { $userPath = "" }
$parts = @($userPath -split ";" | Where-Object { $_ -and $_ -ne $InstallDir })
$newPath = (@($InstallDir) + $parts) -join ";"
[Environment]::SetEnvironmentVariable("Path", $newPath, "User")

$env:BENNUGD_HOME = $InstallDir
$env:Path = "$InstallDir;$env:Path"

Write-Host ""
Write-Host "Installed successfully." -ForegroundColor Green
Write-Info "BENNUGD_HOME=$InstallDir"
Write-Info "Open a new terminal for PATH changes to apply everywhere."
Write-Info "Then try: bgdc -help; bgdi -help"
