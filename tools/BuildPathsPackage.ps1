<#
.SYNOPSIS
	Builds the mod-manager package for Paths - Follower Ability Skilltree.

.DESCRIPTION
	The perk tree add-on: SKSE/Plugins/FightWithMePaths.dll and its INI, with its README
	and the third-party notices beside them, in Fight With Me's package/. This repository is
	checked out into a Fight With Me checkout as paths/; it is built in that CMake tree, from
	the same trees, and both test suites must pass.

.EXAMPLE
	.\paths\tools\BuildPathsPackage.ps1 -VcpkgRoot C:\vcpkg
	.\paths\tools\BuildPathsPackage.ps1 -SkipBuild
#>
[CmdletBinding()]
param(
	[string]$Version = '0.1.0',
	[string]$VcpkgRoot = $env:VCPKG_ROOT,
	[string]$CMake = '',
	[switch]$SkipBuild
)

$ErrorActionPreference = 'Stop'

$addonRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$repoRoot = (Resolve-Path (Join-Path $addonRoot '..')).Path  # the Fight With Me checkout this sits in
if (-not (Test-Path -LiteralPath (Join-Path $repoRoot 'src\Paths.cpp'))) {
	throw "Expected a Fight With Me checkout around $addonRoot (checked out as its paths/ folder)."
}
$packageRoot = [IO.Path]::GetFullPath((Join-Path $repoRoot 'package'))
$releaseName = "Paths - Follower Ability Skilltree-$Version"
$releaseRoot = Join-Path $packageRoot $releaseName
$archivePath = Join-Path $packageRoot "$releaseName.zip"
$builtDll = Join-Path $repoRoot 'build\skyrim\Release\FightWithMePaths.dll'
$tests = @(
	(Join-Path $repoRoot 'build\skyrim\Release\FightWithMePathsTests.exe'),
	(Join-Path $repoRoot 'build\skyrim\Release\FightWithMeTests.exe'))

function Remove-PackageTarget([string]$Path) {
	$full = [IO.Path]::GetFullPath($Path)
	$prefix = $packageRoot.TrimEnd('\') + '\'
	if (-not $full.StartsWith($prefix, [StringComparison]::OrdinalIgnoreCase)) {
		throw "Refusing to remove a path outside the package folder: $full"
	}
	if (Test-Path -LiteralPath $full) {
		Remove-Item -LiteralPath $full -Recurse -Force
	}
}

function Find-CMake {
	if ($CMake) { return $CMake }
	$onPath = Get-Command cmake -ErrorAction SilentlyContinue
	if ($onPath) { return $onPath.Source }
	$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
	if (Test-Path -LiteralPath $vswhere) {
		$found = & $vswhere -latest -products * -find 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' | Select-Object -First 1
		if ($found) { return $found }
	}
	return $null
}

if (-not $SkipBuild) {
	$CMake = Find-CMake
	if (-not $CMake -or -not (Test-Path -LiteralPath $CMake -PathType Leaf)) {
		throw 'CMake was not found. Pass -CMake, put cmake on PATH, or install Visual Studio with its CMake tools.'
	}
	if (-not $VcpkgRoot) {
		throw 'vcpkg was not found. Pass -VcpkgRoot or set VCPKG_ROOT.'
	}
	$env:VCPKG_ROOT = $VcpkgRoot
	Push-Location $repoRoot
	$strict = $ErrorActionPreference
	$ErrorActionPreference = 'Continue'
	try {
		& $CMake --preset skyrim | Out-Null
		if ($LASTEXITCODE -ne 0) { throw 'Configuration failed.' }
		foreach ($target in 'FightWithMePaths', 'FightWithMePathsTests', 'FightWithMeTests') {
			& $CMake --build 'build/skyrim' --config Release --target $target --parallel 8 | Out-Null
			if ($LASTEXITCODE -ne 0) { throw "Build of $target failed." }
		}
	}
	finally {
		$ErrorActionPreference = $strict
		Pop-Location
	}
	foreach ($test in $tests) {
		& $test | Out-Null
		if ($LASTEXITCODE -ne 0) { throw "Tests failed ($test); not packaging." }
	}
}

if (-not (Test-Path -LiteralPath $builtDll -PathType Leaf)) {
	throw "The built plugin is missing: $builtDll"
}

New-Item -ItemType Directory -Force -Path $packageRoot | Out-Null
Remove-PackageTarget $releaseRoot
Remove-PackageTarget $archivePath

$pluginDir = Join-Path $releaseRoot 'SKSE\Plugins'
New-Item -ItemType Directory -Force -Path $pluginDir | Out-Null
Copy-Item -LiteralPath $builtDll -Destination $pluginDir -Force
Copy-Item -LiteralPath (Join-Path $addonRoot 'config\FightWithMePaths.ini') -Destination $pluginDir -Force
# Fight With Me draws this behind the tree for a companion on unlimited points.
$interfaceDir = Join-Path $releaseRoot 'Interface\FightWithMe'
New-Item -ItemType Directory -Force -Path $interfaceDir | Out-Null
Copy-Item -LiteralPath (Join-Path $addonRoot 'assets\cheater.png') -Destination $interfaceDir -Force
Copy-Item -LiteralPath (Join-Path $addonRoot 'README.md') -Destination $releaseRoot -Force
# Its own notices once it has them; Fight With Me's until then (the same libraries).
$notices = Join-Path $addonRoot 'THIRD-PARTY-NOTICES.md'
if (-not (Test-Path -LiteralPath $notices)) { $notices = Join-Path $repoRoot 'THIRD-PARTY-NOTICES.md' }
Copy-Item -LiteralPath $notices -Destination $releaseRoot -Force

# Forward slashes, one entry at a time, as in BuildPackage.ps1.
Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem

$archive = [IO.Compression.ZipFile]::Open($archivePath, [IO.Compression.ZipArchiveMode]::Create)
try {
	$prefixLength = $releaseRoot.TrimEnd('\').Length + 1
	foreach ($file in Get-ChildItem -LiteralPath $releaseRoot -Recurse -File) {
		$relative = $file.FullName.Substring($prefixLength).Replace('\', '/')
		[void][IO.Compression.ZipFileExtensions]::CreateEntryFromFile(
			$archive, $file.FullName, $relative, [IO.Compression.CompressionLevel]::Optimal)
	}
}
finally { $archive.Dispose() }

$verify = [IO.Compression.ZipFile]::OpenRead($archivePath)
try {
	$names = @($verify.Entries | ForEach-Object { $_.FullName })
	if (@($names | Where-Object { $_ -like '*\*' }).Count -gt 0) { throw 'Archive has entries with backslash separators.' }
	foreach ($required in 'SKSE/Plugins/FightWithMePaths.dll', 'SKSE/Plugins/FightWithMePaths.ini', 'Interface/FightWithMe/cheater.png') {
		if ($names -notcontains $required) { throw "Archive is missing $required." }
	}
}
finally { $verify.Dispose() }

Write-Output "Package : $releaseRoot"
Write-Output "Archive : $archivePath"
Write-Output ("Plugin  : {0:N0} bytes" -f (Get-Item $builtDll).Length)
Write-Output ("Archive : {0:N0} bytes" -f (Get-Item $archivePath).Length)
