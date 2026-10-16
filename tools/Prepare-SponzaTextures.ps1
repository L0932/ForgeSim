<#
.SYNOPSIS
Creates a lower-resolution copy of Intel's Sponza glTF scene.

.DESCRIPTION
Copies the external glTF scene and binary data, then resizes PNG
textures while preserving their relative paths. The Intel Sponza
assets are not distributed with ForgeSim and must be downloaded
separately from Intel's GPU Research Samples page.

The generated output is intended for local development and should
not be committed to the ForgeSim repository.
#>

param(
    [Parameter(Mandatory = $true)]
    [string] $SourceDirectory,

    [Parameter(Mandatory = $true)]
    [string] $DestinationDirectory,

    [ValidateRange(1, 16384)]
    [int] $MaximumTextureSize = 1024
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path -LiteralPath $SourceDirectory -PathType Container))
{
    throw "Source directory was not found: $SourceDirectory"
}

if (-not (Get-Command "magick" -ErrorAction SilentlyContinue))
{
    throw (
        "ImageMagick was not found. Install it and ensure that " +
        "'magick' is available from PowerShell."
    )
}

$resolvedSourceDirectory =
    (Resolve-Path -LiteralPath $SourceDirectory).Path

$sourceTextureDirectory =
    Join-Path $resolvedSourceDirectory "textures"

if (-not (
    Test-Path `
        -LiteralPath $sourceTextureDirectory `
        -PathType Container))
{
    throw (
        "The source texture directory was not found: " +
        $sourceTextureDirectory
    )
}

New-Item `
    -ItemType Directory `
    -Path $DestinationDirectory `
    -Force |
    Out-Null

$resolvedDestinationDirectory =
    (Resolve-Path -LiteralPath $DestinationDirectory).Path

$destinationTextureDirectory =
    Join-Path $resolvedDestinationDirectory "textures"

New-Item `
    -ItemType Directory `
    -Path $destinationTextureDirectory `
    -Force |
    Out-Null

Write-Host "Copying glTF scene files..."

$sceneFiles =
    Get-ChildItem `
        -LiteralPath $resolvedSourceDirectory `
        -File |
    Where-Object {
        $_.Extension -in @(
            ".gltf",
            ".glb",
            ".bin"
        )
    }

if ($sceneFiles.Count -eq 0)
{
    throw (
        "No glTF or binary scene files were found in: " +
        $resolvedSourceDirectory
    )
}

foreach ($sceneFile in $sceneFiles)
{
    Copy-Item `
        -LiteralPath $sceneFile.FullName `
        -Destination $resolvedDestinationDirectory `
        -Force

    Write-Host "Copied $($sceneFile.Name)"
}

# Preserve any attribution, license, or documentation files stored
# alongside the scene.
$documentationFiles =
    Get-ChildItem `
        -LiteralPath $resolvedSourceDirectory `
        -File |
    Where-Object {
        $_.Name -match `
            '^(LICENSE|LICENCE|NOTICE|README|COPYING)(\..+)?$'
    }

foreach ($documentationFile in $documentationFiles)
{
    Copy-Item `
        -LiteralPath $documentationFile.FullName `
        -Destination $resolvedDestinationDirectory `
        -Force

    Write-Host "Copied $($documentationFile.Name)"
}

$textureFiles =
    Get-ChildItem `
        -LiteralPath $sourceTextureDirectory `
        -Filter "*.png" `
        -File `
        -Recurse

if ($textureFiles.Count -eq 0)
{
    throw (
        "No PNG textures were found in: " +
        $sourceTextureDirectory
    )
}

# System.IO.Path.GetRelativePath is unavailable in Windows
# PowerShell 5.1, so calculate paths relative to the texture
# directory by removing its normalized prefix.
$directorySeparator =
    [System.IO.Path]::DirectorySeparatorChar

$sourceTextureRoot =
    (Resolve-Path -LiteralPath $sourceTextureDirectory).Path.TrimEnd(
        $directorySeparator
    ) + $directorySeparator

$processedTextureCount = 0

Write-Host (
    "Preparing $($textureFiles.Count) textures with a maximum " +
    "dimension of $MaximumTextureSize pixels..."
)

foreach ($textureFile in $textureFiles)
{
    $relativePath =
        $textureFile.FullName.Substring(
            $sourceTextureRoot.Length
        )

    $destinationTexturePath =
        Join-Path $destinationTextureDirectory $relativePath

    $destinationTextureParent =
        Split-Path `
            -Path $destinationTexturePath `
            -Parent

    New-Item `
        -ItemType Directory `
        -Path $destinationTextureParent `
        -Force |
        Out-Null

    Write-Host (
        "Processing texture " +
        "$($processedTextureCount + 1)/$($textureFiles.Count): " +
        $relativePath
    )

# Preserve the original aspect ratio and avoid enlarging images
# that are already within the requested limit.

    & magick `
        $textureFile.FullName `
        -resize "$($MaximumTextureSize)x$($MaximumTextureSize)>" `
        -strip `
        $destinationTexturePath

    if ($LASTEXITCODE -ne 0)
    {
        throw (
            "ImageMagick failed while processing: " +
            $textureFile.FullName
        )
    }

    $processedTextureCount++
}

Write-Host ""
Write-Host "Sponza texture preparation completed."
Write-Host "Textures processed: $processedTextureCount"
Write-Host "Output directory: $resolvedDestinationDirectory"