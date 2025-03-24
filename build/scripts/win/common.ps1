function Initialize-VisualStudioPaths
{
    # Use vswhere to locate msbuild.exe
    $vswhere = Join-Path "$env:ProgramFiles (x86)" "Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere))
    {
        throw "Could not find vswhere.exe"
    }

    $global:Msbuild = & $vswhere -latest -requires Microsoft.Component.MSBuild -find "MSBuild\**\Bin\MSBuild.exe" | Select-Object -First 1
    if (-not (Test-Path $global:Msbuild))
    {
        throw "Could not find msbuild.exe"
    }
}

function Initialize-Prefix
{
    param
    (
        [Parameter(Mandatory = $true)]
        $Prefix
    )

    New-Directory (Join-Path $Prefix "bin") | Out-Null
    New-Directory (Join-Path $Prefix "doc") | Out-Null
    New-Directory (Join-Path $Prefix "include") | Out-Null
    New-Directory (Join-Path $Prefix "lib") | Out-Null
    New-Directory (Join-Path $Prefix "man") | Out-Null
    New-Directory (Join-Path $Prefix "share") | Out-Null
    New-Directory (Join-Path $Prefix "usr\src") | Out-Null
    New-Directory (Join-Path $Prefix "tmp") | Out-Null
    New-Directory (Join-Path $Prefix "var") | Out-Null    
}

function Initialize-Directories
{
    $global:RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot "../../..")).Path

    # Create a build directory: /win-$global:Target_arm64_x86_x64/
    $global:TargetPrefix = New-Directory (Join-Path $global:RepositoryRoot "win-$global:Target_arm64_x86_x64")
    Initialize-Prefix $global:TargetPrefix

    # Create a native prefix directory: /win-native/
    $global:NativePrefix = New-Directory (Join-Path $global:RepositoryRoot "win-native")
    Initialize-Prefix $global:NativePrefix

    # Downloads will be in the /build/downloads/ directory.
    $global:DownloadsDir = New-Directory (Join-Path $global:RepositoryRoot "build/downloads")

    # We need to read in some of these static build files.
    $global:FilesDir = New-Directory (Join-Path $global:RepositoryRoot "build/files")
}

function New-Directory
{
    param
    (
        [Parameter(Mandatory = $true)]
        $Path
    )

    if (-not (Test-Path $Path))
    {
        New-Item -ItemType Directory -Path $Path | Out-Null
    }

    return (Resolve-Path $Path).Path
}

function New-TemporaryDirectory
{
    $name = [System.IO.Path]::GetRandomFileName()
    $dir = Join-Path $global:TargetPrefix "tmp\$name"
    [System.IO.Directory]::CreateDirectory($dir) | Out-Null
    return $dir
}

function Expand-ArchiveIgnoringTopLevelDirectory
{
    param
    (
        [Parameter(Mandatory = $true)]
        $Path,

        [Parameter(Mandatory = $true)]
        $DestinationPath
    )

    # Ensure destination path exists
    if (-not (Test-Path $DestinationPath))
    {
        New-Item -ItemType Directory -Path $DestinationPath | Out-Null
    }
    
    # Get full paths
    $zipPath = (Resolve-Path $Path).Path
    $zipFilename = [System.IO.Path]::GetFileName($zipPath)
    $destPath = (Resolve-Path $DestinationPath).Path
    
    Write-Host "Expanding $zipFilename."
    
    # Open the zip file
    $zip = [System.IO.Compression.ZipFile]::OpenRead($zipPath)
    try 
    {
        # Get all entries
        $entries = $zip.Entries
        
        if ($entries.Count -eq 0) {
            throw "Zip file is empty"
        }
        
        # Get the top level directory from the first entry
        $firstEntryPath = $entries[0].FullName
        $topLevelDir = $firstEntryPath.Split('/')[0] + '/'

        # Extract entries, skipping the top level directory
        foreach ($entry in $entries)
        {
            # Verify this entry is in the same top level directory
            if (-not $entry.FullName.StartsWith($topLevelDir))
            {
                throw "Zip contains multiple top-level directories. Found: $($entry.FullName) which doesn't start with $topLevelDir"
            }
            
            # Get the relative path by removing the top level directory
            $relativePath = $entry.FullName.Substring($topLevelDir.Length)
            if ([string]::IsNullOrEmpty($relativePath))
            {
                # It's the top-level directory itself.
                continue
            }
            
            # Create the target path
            $targetPath = Join-Path $destPath $relativePath

            # If it ends in / then it's a directory.
            if ($entry.FullName.EndsWith('/'))
            {
                [System.IO.Directory]::CreateDirectory($targetPath) | Out-Null
                continue
            }

            # Ensure the target directory exists
            $targetDir = [System.IO.Path]::GetDirectoryName($targetPath)
            [System.IO.Directory]::CreateDirectory($targetDir) | Out-Null

            # Extract the file directly to the destination
            $entryStream = $entry.Open()
            $fileStream = [System.IO.File]::Create($targetPath)
            
            try
            {
                $entryStream.CopyTo($fileStream)
            }
            finally
            {
                $fileStream.Close()
                $entryStream.Close()
            }
        }
    }
    finally
    {
        # Clean up
        $zip.Dispose()
    }
}

function Expand-ArchiveRenamingTopLevelDirectory
{
    param
    (
        [Parameter(Mandatory = $true)]
        $Path,

        [Parameter(Mandatory = $true)]
        $DestinationPath,

        [Parameter(Mandatory = $true)]
        $NewName
    )

    $zipFilename = [System.IO.Path]::GetFileName($Path)
    Write-Host "Expanding $zipFilename."
    
    # Stupid way to detect the top-level directory: read all the directories before and after, and find the new one.
    $dirsBefore = [System.IO.Directory]::GetDirectories($DestinationPath)
    
    & 7za x -y "-o$DestinationPath" $Path | Out-File -FilePath $global:VerboseLogFile -Encoding UTF8 -Append
    if ($LASTEXITCODE -ne 0)
    {
        throw "Failed to extract $Path"
    }

    $dirsAfter = [System.IO.Directory]::GetDirectories($DestinationPath)
    $topLevelDir = $dirsAfter | Where-Object { $dirsBefore -notcontains $_ } | Select-Object -First 1
    if ([string]::IsNullOrEmpty($topLevelDir))
    {
        throw "Could not find top-level directory after extracting $Path"
    }

    # Rename the top-level directory
    $newDir = Join-Path $DestinationPath $NewName

    if (Test-Path $newDir)
    {
        [System.IO.Directory]::Delete($newDir, $true)
    }

    [System.IO.Directory]::Move($topLevelDir, $newDir)
}


function Expand-Tarball
{
    param
    (
        [Parameter(Mandatory = $true)]
        $Path,

        [Parameter(Mandatory = $true)]
        $DestinationPath
    )

    # Ensure destination path exists
    if (-not (Test-Path $DestinationPath))
    {
        New-Item -ItemType Directory -Path $DestinationPath | Out-Null
    }

    # Get full paths
    $tarballPath = (Resolve-Path $Path).Path
    $tarballFilename = [System.IO.Path]::GetFileName($tarballPath)
    $destPath = (Resolve-Path $DestinationPath).Path
    
    Write-Host "Expanding $tarballFilename."
    
    # Create a temporary directory for the extraction
    $tempDir = New-TemporaryDirectory
    try
    {
        # Step 1: Extract the .tar.gz to get a .tar file
        & 7za e -y "-o$tempDir" $tarballPath | Out-File -FilePath $global:VerboseLogFile -Encoding UTF8 -Append
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to extract $tarballPath"
        }
        
        # Find the .tar file in the temp directory
        $tarFile = Get-ChildItem -Path $tempDir -Filter "*.tar" | Select-Object -First 1
        if ($null -eq $tarFile)
        {
            throw "Could not find .tar file after extracting $tarballPath"
        }
        
        # Step 2: Extract the .tar file to the destination
        & 7za x -y "-o$destPath" $tarFile.FullName | Out-File -FilePath $global:VerboseLogFile -Encoding UTF8 -Append
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to extract $tarballPath"
        }
    }
    finally
    {
        # Clean up the temporary directory
        Remove-Item -Recurse -Force -Path $tempDir -ErrorAction SilentlyContinue
    }
}

function Expand-TarballRenamingTopLevelDirectory
{
    param
    (
        [Parameter(Mandatory = $true)]
        $Path,

        [Parameter(Mandatory = $true)]
        $DestinationPath,

        [Parameter(Mandatory = $true)]
        $NewName
    )
    
    # Stupid way to detect the top-level directory: read all the directories before and after, and find the new one.
    $dirsBefore = [System.IO.Directory]::GetDirectories($DestinationPath)
    Expand-Tarball -Path $Path -DestinationPath $DestinationPath
    $dirsAfter = [System.IO.Directory]::GetDirectories($DestinationPath)
    $topLevelDir = $dirsAfter | Where-Object { $dirsBefore -notcontains $_ } | Select-Object -First 1
    if ([string]::IsNullOrEmpty($topLevelDir))
    {
        throw "Could not find top-level directory after extracting $Path"
    }

    # Rename the top-level directory
    $newDir = Join-Path $DestinationPath $NewName

    if (Test-Path $newDir)
    {
        [System.IO.Directory]::Delete($newDir, $true)
    }

    [System.IO.Directory]::Move($topLevelDir, $newDir)
}

function Initialize-PackageVersions
{
    # Read the depsDownload.sh file
    $depsFile = Join-Path $PSScriptRoot "..\depsDownload.sh"
    $content = Get-Content $depsFile -Raw

    # Create a hashtable to store the versions
    $versions = @{}

    # Use regex to find all VERSION assignments
    $matches = [regex]::Matches($content, '([A-Z0-9_]+)_VERSION=([^\r\n]+)')
    
    foreach ($match in $matches)
    {
        $key = $match.Groups[1].Value
        $version = $match.Groups[2].Value.Trim("'").Trim('"')
        $versions[$key] = $version
    }

    $global:PackageVersions = $versions
}

function Get-InstalledPackageVersion
{
    param
    (
        [Parameter(Mandatory = $true)]
        $Name
    )

    $file = Join-Path $global:TargetPrefix "var\packages\$Name"
    if (-not (Test-Path $file))
    {
        Write-Host "$Name is not installed."
        return ""
    }

    $version = [System.IO.File]::ReadAllText($file).Trim()
    Write-Host "$Name $version is installed."
    return $version
}

function Set-InstalledPackage
{
    param
    (
        [Parameter(Mandatory = $true)]
        $Name,

        [Parameter(Mandatory = $true)]
        $Version
    )

    $dir = Join-Path $global:TargetPrefix "var\packages"
    [System.IO.Directory]::CreateDirectory($dir) | Out-Null

    $file = Join-Path $dir $Name
    [System.IO.File]::WriteAllText($file, $Version)

    Write-Host "Installed $Name $Version."
}

function Get-DownloadedFile
{
    param
    (
        [Parameter(Mandatory = $true)]
        $Url,

        [Parameter(Mandatory = $true)]
        $Filename
    )

    # Check if the file exists in the downloads directory already
    $filePath = Join-Path $global:DownloadsDir $Filename
    if (Test-Path $filePath)
    {
        return $filePath
    }

    # Download the file to a temporary file
    Write-Host "Downloading: $Url"
    $tempFilePath = Join-Path $global:DownloadsDir ([System.IO.Path]::GetRandomFileName())
    try
    {
        & curl.exe -Lo "$tempFilePath" "$Url"
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to download $Url"
        }

        # Move the file to the downloads directory
        Move-Item $tempFilePath $filePath

        return $filePath
    }
    catch
    {
        if (Test-Path $tempFilePath)
        {
            Remove-Item $tempFilePath
        }

        throw
    }
}
