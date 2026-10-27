[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string] $Tag
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# Examples: v0.1.0, v0.1.0-preview.3
$pattern = '^v(?<base>(?:0|[1-9][0-9]*)\.(?:0|[1-9][0-9]*)\.(?:0|[1-9][0-9]*))(?:-(?<prerelease>[0-9A-Za-z-]+(?:\.[0-9A-Za-z-]+)*))?$'

$match = [regex]::Match($Tag, $pattern)

if (-not $match.Success)
{
    throw (
        "Invalid release tag '$Tag'. " +
        "Expected vMAJOR.MINOR.PATCH with an optional prerelease suffix, " +
        "for example v0.1.0 or v0.1.0-preview.3."
    )
}

$baseVersion = $match.Groups["base"].Value
$prerelease = $match.Groups["prerelease"].Value
$version = $baseVersion

if ($prerelease.Length -gt 0)
{
    $version += "-$prerelease"
}

[pscustomobject]@{
    Tag = $Tag
    Version = $version
    Prerelease = $prerelease
    IsPrerelease = ($prerelease.Length -gt 0)
}