<#
.SYNOPSIS
	Builds the mod-manager package for Path of the Companions.

.DESCRIPTION
	SKSE/Plugins/FightWithMePaths.dll and its INI, the panel's icons and fonts (the same files
	Fight With Me's main download carries), both translation templates, its README and the
	third-party notices, in Fight With Me's package/. The ability art is the separate Fight With
	Me - Effects download, which Fight With Me users already have. This repository is
	checked out into a Fight With Me checkout as paths/; it is built in that CMake tree, from
	the same trees, and both test suites must pass.
	-IncludeCoreUIUpdate adds the matching core DLL and installation notes. The source
	archive supplied with -SourceArchive is a separate download, never nested in the mod.
	It leaves the core INI out of the bundle.

.EXAMPLE
	.\paths\tools\BuildPathsPackage.ps1 -VcpkgRoot C:\vcpkg
	.\paths\tools\BuildPathsPackage.ps1 -SkipBuild
	.\paths\tools\BuildPathsPackage.ps1 -SkipBuild -IncludeCoreUIUpdate
#>
[CmdletBinding()]
param(
	[string]$Version = '1.2.0',
	# The Fight With Me release built and tested with this one; names the source download.
	[string]$CoreVersion = '0.6.0',
	[string]$VcpkgRoot = $env:VCPKG_ROOT,
	[string]$CMake = '',
	[string]$SourceArchive = '',
	[ValidatePattern('^[A-Za-z0-9-]*$')][string]$PackageSuffix = '',
	[switch]$SkipBuild,
	[switch]$IncludeCoreUIUpdate
)

$ErrorActionPreference = 'Stop'

$addonRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$repoRoot = (Resolve-Path (Join-Path $addonRoot '..')).Path  # the Fight With Me checkout this sits in
if (-not (Test-Path -LiteralPath (Join-Path $repoRoot 'src\Paths.cpp'))) {
	throw "Expected a Fight With Me checkout around $addonRoot (checked out as its paths/ folder)."
}
$packageRoot = [IO.Path]::GetFullPath((Join-Path $repoRoot 'package'))
$releaseName = "Path of the Companions-$Version"
if ($IncludeCoreUIUpdate) { $releaseName += '-Perk-Tree-Update' }
if ($PackageSuffix) { $releaseName += "-$PackageSuffix" }
$releaseRoot = Join-Path $packageRoot $releaseName
$archivePath = Join-Path $packageRoot "$releaseName.zip"
$sourceDownloadPath = Join-Path $packageRoot "$releaseName-Source.zip"
$builtDll = Join-Path $repoRoot 'build\skyrim\Release\FightWithMePaths.dll'
$coreDll = Join-Path $repoRoot 'build\skyrim\Release\FightWithMe.dll'
if (-not $SourceArchive) { $SourceArchive = Join-Path $repoRoot 'release\1.0-launch\FightWithMe-0.5.0-Paths-1.0.0-Source.zip' }
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
		if ($IncludeCoreUIUpdate) {
			& $CMake --build 'build/skyrim' --config Release --target FightWithMe --parallel 8 | Out-Null
			if ($LASTEXITCODE -ne 0) { throw 'Build of FightWithMe failed.' }
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
$actualVersion = (Get-Item -LiteralPath $builtDll).VersionInfo.FileVersion
if ($actualVersion -ne "$Version.0") { throw "Built DLL version $actualVersion does not match package $Version. Rebuild first." }
# The translation templates must say what the source says.
& (Join-Path $repoRoot 'tools\Strings.ps1') -Check
if ($IncludeCoreUIUpdate -and -not (Test-Path -LiteralPath $coreDll -PathType Leaf)) {
	throw "The core UI update is missing: $coreDll"
}
if ($IncludeCoreUIUpdate -and -not (Test-Path -LiteralPath $SourceArchive -PathType Leaf)) {
	throw "The matching release source archive is missing: $SourceArchive"
}

New-Item -ItemType Directory -Force -Path $packageRoot | Out-Null
Remove-PackageTarget $releaseRoot
Remove-PackageTarget $archivePath

$pluginDir = Join-Path $releaseRoot 'SKSE\Plugins'
New-Item -ItemType Directory -Force -Path $pluginDir | Out-Null
Copy-Item -LiteralPath $builtDll -Destination $pluginDir -Force
Copy-Item -LiteralPath (Join-Path $addonRoot 'config\FightWithMePaths.ini') -Destination $pluginDir -Force
# The English templates: the trees' text, and the panel's and the page's, which on its own Paths
# draws itself. Both plugins read both from this folder.
$translationDir = Join-Path $pluginDir 'FightWithMe\Translations'
New-Item -ItemType Directory -Force -Path $translationDir | Out-Null
Copy-Item -LiteralPath (Join-Path $addonRoot 'config\Translations\FightWithMePaths_ENGLISH.txt') -Destination $translationDir -Force
Copy-Item -LiteralPath (Join-Path $repoRoot 'config\Translations\FightWithMe_ENGLISH.txt') -Destination $translationDir -Force
# Traits belong to Paths, including when Fight With Me is absent.
New-Item -ItemType Directory -Force -Path (Join-Path $releaseRoot 'docs') | Out-Null
Copy-Item -LiteralPath (Join-Path $repoRoot 'docs\RELEASE-NOTES.md') -Destination (Join-Path $releaseRoot 'docs') -Force
$traitDir = Join-Path $pluginDir 'FightWithMe\Traits'
New-Item -ItemType Directory -Force -Path $traitDir | Out-Null
Copy-Item -LiteralPath (Join-Path $repoRoot 'config\Traits\Followers.ini') -Destination $traitDir -Force
# The icons and fonts the panel and the trees are drawn with, as Fight With Me's main download
# ships them: identical files, so either may win a conflict.
$iconDir = Join-Path $releaseRoot 'interface\FightWithMe'
New-Item -ItemType Directory -Force -Path $iconDir | Out-Null
Copy-Item -LiteralPath (Join-Path $repoRoot 'assets\interface\FightWithMe\glyphs') -Destination $iconDir -Recurse -Force
foreach ($icon in 'wolf-head.svg', 'barbed-sun.svg', 'weak-will.svg', 'last-bastion.svg') {
	Copy-Item -LiteralPath (Join-Path $repoRoot "assets\interface\FightWithMe\$icon") -Destination $iconDir -Force
}
$fontDir = Join-Path $pluginDir 'Fonts'
New-Item -ItemType Directory -Force -Path $fontDir | Out-Null
Get-ChildItem -LiteralPath (Join-Path $repoRoot 'assets\fonts') -Recurse -File | ForEach-Object {
	Copy-Item -LiteralPath $_.FullName -Destination $fontDir -Force
}
# Skald is part of Paths, including its lute recordings and new artwork.
& (Join-Path $repoRoot 'tools\skald\StageAssets.ps1') -Destination $releaseRoot
"Required download: POTC Effects 1.0.21 or newer (ability assets and the 18 class backdrops; no plugin)." |
	Set-Content -LiteralPath (Join-Path $releaseRoot 'Required-Effects.txt') -Encoding UTF8
if ($IncludeCoreUIUpdate) {
	# Optional matched core update; Paths owns the perk window.
	Copy-Item -LiteralPath $coreDll -Destination $pluginDir -Force
	Copy-Item -LiteralPath (Join-Path $repoRoot 'README.md') -Destination (Join-Path $releaseRoot 'README-FightWithMe.md') -Force
	Copy-Item -LiteralPath $SourceArchive -Destination $sourceDownloadPath -Force
	$sourceInformation = "Matching source and build instructions are provided as a separate source download.`r`nFile: $([IO.Path]::GetFileName($sourceDownloadPath))`r`nSHA256: $((Get-FileHash -LiteralPath $sourceDownloadPath).Hash)`r`nExtract that download and read BUILD-SOURCE.md. Do not install it as a game mod.`r`n"
	[IO.File]::WriteAllText((Join-Path $releaseRoot 'SOURCE-INFORMATION.txt'), $sourceInformation, [Text.UTF8Encoding]::new($false))
	$installNotes = @'
PATH OF THE COMPANIONS: MATCHED PLUGIN UPDATE

Includes Path of the Companions 1.0.0 and the matching Fight With Me 0.5.0 core build.
Paths owns abilities, progression, traits and the perk menu in every setup.
Fight With Me owns orders, roles, targets, positioning and loadout pools.

Install the current Fight With Me base package first, then this optional bundle.
Let this bundle win the FightWithMe.dll conflict. Install Effects 1.0.21 or newer.
Update both shipped INIs from the matching packages; preserve *_user.ini files.
Paths: Numpad 9 / LB + D-pad right. Orders: Numpad 7 / LB + D-pad down.
Old progression saves refund points once. Purchase and carry abilities again.
Current-format saves retain learned ranks and equipped slots.

Includes Disarm, Second Wind, Azura's Moon, Riposte, Volley per-shot wind,
and Explosive Arrow (replaces Deadeye), with generated rune textures.
'@
	[IO.File]::WriteAllText((Join-Path $releaseRoot 'INSTALL-AND-CHANGES.txt'), ($installNotes -replace "`r?`n", "`r`n"), [Text.UTF8Encoding]::new($false))
}
# Interface fonts and icons.
$interfaceDir = Join-Path $releaseRoot 'interface\FightWithMe'
New-Item -ItemType Directory -Force -Path $interfaceDir | Out-Null
Copy-Item -LiteralPath (Join-Path $addonRoot 'README.md') -Destination $releaseRoot -Force
# Its own notices once it has them; Fight With Me's until then (the same libraries).
$notices = Join-Path $addonRoot 'THIRD-PARTY-NOTICES.md'
if (-not (Test-Path -LiteralPath $notices)) { $notices = Join-Path $repoRoot 'THIRD-PARTY-NOTICES.md' }
Copy-Item -LiteralPath $notices -Destination $releaseRoot -Force
$licenseDir = Join-Path $releaseRoot 'licenses'
New-Item -ItemType Directory -Force -Path $licenseDir | Out-Null
Copy-Item -LiteralPath (Join-Path $repoRoot 'extern\CommonLibSSE-NG\COPYING') -Destination (Join-Path $licenseDir 'CommonLibSSE-NG-GPL-3.0.txt') -Force
Copy-Item -LiteralPath (Join-Path $repoRoot 'extern\CommonLibSSE-NG\EXCEPTIONS.md') -Destination (Join-Path $licenseDir 'CommonLibSSE-NG-EXCEPTIONS.md') -Force
Copy-Item -LiteralPath (Join-Path $repoRoot 'extern\CommonLibSSE-NG\licenses\LICENSE-MIT') -Destination (Join-Path $licenseDir 'CommonLibSSE-NG-legacy-MIT.txt') -Force
Copy-Item -LiteralPath (Join-Path $repoRoot 'extern\CommonLibSSE-NG\extern\openvr\LICENSE') -Destination (Join-Path $licenseDir 'OpenVR-LICENSE.txt') -Force
Copy-Item -LiteralPath (Join-Path $repoRoot 'build\skyrim\_deps\hde64-src\LICENSE.txt') -Destination (Join-Path $licenseDir 'MinHook-hde64-LICENSE.txt') -Force
$dependencyNotices = Join-Path $repoRoot 'build\skyrim\vcpkg_installed\x64-windows-static-md\share'
foreach ($dependency in 'directxmath', 'directxtk', 'fmt', 'rapidcsv', 'spdlog') {
	Copy-Item -LiteralPath (Join-Path $dependencyNotices "$dependency\copyright") -Destination (Join-Path $licenseDir "$dependency-copyright.txt") -Force
}

if (-not $IncludeCoreUIUpdate) {
    "Matching plugin source: FightWithMe-$CoreVersion-Paths-$Version-Source.zip (separate developer download; not required to play). See BUILD-SOURCE.md inside that archive." |
        Set-Content -LiteralPath (Join-Path $releaseRoot 'SOURCE-INFORMATION.txt') -Encoding UTF8
}
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
	if (@($names | Where-Object { $_ -match '\.(zip|7z|rar|tar|gz|bz2|xz|cab|exe|pdb|obj)$' }).Count -gt 0) {
		throw 'Runtime package must not contain nested archives, executables or debug/build files.'
	}
	foreach ($required in 'SKSE/Plugins/FightWithMePaths.dll', 'SKSE/Plugins/FightWithMePaths.ini',
		'SKSE/Plugins/FightWithMe/Translations/FightWithMePaths_ENGLISH.txt', 'SKSE/Plugins/FightWithMe/Translations/FightWithMe_ENGLISH.txt',
		'SKSE/Plugins/FightWithMe/Traits/Followers.ini', 'interface/FightWithMe/glyphs/f0c0.svg', 'interface/FightWithMe/wolf-head.svg', 'Required-Effects.txt') {
		if ($names -notcontains $required) { throw "Archive is missing $required." }
	}
	if (@($names | Where-Object { $_ -like 'SKSE/Plugins/Fonts/*.ttf' }).Count -eq 0) { throw 'Archive is missing the theme fonts.' }
	if ($IncludeCoreUIUpdate) {
		foreach ($required in 'SKSE/Plugins/FightWithMe.dll', 'INSTALL-AND-CHANGES.txt', 'README-FightWithMe.md', 'SOURCE-INFORMATION.txt') {
			if ($names -notcontains $required) { throw "Archive is missing $required." }
		}
		if ($names -contains 'SKSE/Plugins/FightWithMe.ini' -or $names -contains 'SKSE/Plugins/FightWithMe_user.ini') {
			throw 'The UI update must not replace the existing core configuration.'
		}
	}
}
finally { $verify.Dispose() }

Write-Output "Package : $releaseRoot"
Write-Output "Archive : $archivePath"
if ($IncludeCoreUIUpdate) { Write-Output "Source  : $sourceDownloadPath (separate download)" }
Write-Output ("Plugin  : {0:N0} bytes" -f (Get-Item $builtDll).Length)
Write-Output ("Archive : {0:N0} bytes" -f (Get-Item $archivePath).Length)
