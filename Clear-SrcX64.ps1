#requires -Version 5.1
[CmdletBinding(SupportsShouldProcess = $true, ConfirmImpact = 'Medium')]
param()

$ErrorActionPreference = 'Stop'

$RepoRoot = Split-Path -Parent $PSCommandPath
$SrcPath = Join-Path $RepoRoot 'src'

if (-not (Test-Path -LiteralPath $SrcPath -PathType Container)) {
	throw "src directory not found at '$SrcPath'."
}

$directoriesToVisit = [System.Collections.Generic.Stack[System.IO.DirectoryInfo]]::new()
$directoriesToVisit.Push((Get-Item -LiteralPath $SrcPath -Force))
$x64Candidates = [System.Collections.Generic.List[System.IO.DirectoryInfo]]::new()
$protectedPaths = [System.Collections.Generic.List[string]]::new()

while ($directoriesToVisit.Count -gt 0) {
	$directory = $directoriesToVisit.Pop()

	if (
		($directory.Attributes -band [System.IO.FileAttributes]::ReparsePoint) -or
		$directory.Name -in @('packages', '.nuget') -or
		(Get-ChildItem -LiteralPath $directory.FullName -File -Force -Filter '*.nuspec' |
			Select-Object -First 1)
	) {
		$protectedPaths.Add($directory.FullName)
		continue
	}

	if ($directory.Name -eq 'x64') {
		$x64Candidates.Add($directory)
	}

	foreach ($child in Get-ChildItem -LiteralPath $directory.FullName -Directory -Force) {
		$directoriesToVisit.Push($child)
	}
}

$x64Directories =
	$x64Candidates |
	Where-Object {
		$candidatePath = $_.FullName + [System.IO.Path]::DirectorySeparatorChar

		# Removing an ancestor would also delete the protected package or linked directory.
		foreach ($protectedPath in $protectedPaths) {
			if ($protectedPath.StartsWith($candidatePath, [System.StringComparison]::OrdinalIgnoreCase)) {
				return $false
			}
		}

		return $true
	} |
	Sort-Object { $_.FullName.Length } -Descending

if (-not $x64Directories) {
	Write-Host "No removable x64 directories found under '$SrcPath'."
	return
}

foreach ($directory in $x64Directories) {
	if (-not (Test-Path -LiteralPath $directory.FullName -PathType Container)) {
		continue
	}

	if ($PSCmdlet.ShouldProcess($directory.FullName, 'Remove directory recursively')) {
		Remove-Item -LiteralPath $directory.FullName -Recurse -Force
		Write-Host "Removed $($directory.FullName)"
	}
}
