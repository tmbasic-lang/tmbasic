param
(
    [Parameter(Mandatory = $true)]
    $Platform,

    [Parameter(Mandatory = $true)]
    $BuildDeps,

    [Parameter(Mandatory = $true)]
    $BuildApp,

    [Parameter(Mandatory = $true)]
    $BuildType,

    [Parameter(Mandatory = $true)]
    $UseS3Mirror
)

# 1 = Print commands as they are executed
Set-PSDebug -Trace 0

# PowerShell configuration
$global:ErrorActionPreference = "Stop"
$global:ProgressPreference = "SilentlyContinue"
Add-Type -AssemblyName System.IO.Compression.FileSystem

# Imports
. "$PSScriptRoot/common.ps1"

# Global variables
$global:BuildDeps = $BuildDeps # boolean
$global:BuildApp = $BuildApp # boolean
$global:BuildType = $BuildType # "Release" or "Debug"
$global:UseS3Mirror = $UseS3Mirror # boolean
$global:Host_arm64_x86_x64 = $null # "arm64", "x86", or "x64"
$global:Target_arm64_x86_x64 = $null # "arm64", "x86", or "x64"
$global:Target_ARM64_Win32_x64 = $null # "ARM64", "Win32", or "x64"
$global:TargetPrefix = $null # Unix-style root directory
$global:NativePrefix = $null # Unix-style root directory
$global:RepositoryRoot = $null # /
$global:DownloadsDir = $null # /build/downloads/
$global:FilesDir = $null # /build/files/
$global:PackageVersions = @{}
$global:Msbuild = $null # Path to msbuild.exe
$global:RuntimeFlag = $null # MSVC runtime library flag for CMake

function Main
{
    if ($Platform -notin @("x86", "x64", "arm64"))
    {
        throw "Invalid platform: $Platform"
    }

    switch ($env:PROCESSOR_ARCHITECTURE)
    {
        "ARM64" { $global:Host_arm64_x86_x64 = "arm64" }
        "AMD64" { $global:Host_arm64_x86_x64 = "x64" }
        default { throw "Unsupported processor architecture: $env:PROCESSOR_ARCHITECTURE" }
    }

    if ($Platform -eq "arm64")
    {
        $global:Target_ARM64_Win32_x64 = "ARM64"
        $global:Target_arm64_x86_x64 = "arm64"
    }
    elseif ($Platform -eq "x86")
    {
        $global:Target_ARM64_Win32_x64 = "Win32"
        $global:Target_arm64_x86_x64 = "x86"
    }
    elseif ($Platform -eq "x64")
    {
        $global:Target_ARM64_Win32_x64 = "x64"
        $global:Target_arm64_x86_x64 = "x64"
    }

    # Set the runtime flag based on build type
    if ($global:BuildType -eq "Debug")
    {
        $global:RuntimeFlag = "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug"
    }
    else
    {
        $global:RuntimeFlag = "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded"
    }

    Initialize-VisualStudioPaths
    Initialize-Directories
    Initialize-PackageVersions

    $env:PATH = (Join-Path $global:NativePrefix "bin") + ";" + $env:PATH

    Install-SevenZip
    Install-Cmake
    Install-GoogleTest
    Install-Fmt
    Install-Immer
    Install-Mpdecimal
    Install-Tvision
    Install-Turbo
    Install-Nameof
    Install-Zlib
    Install-Microtar
    Install-Libzip
    Install-Cli11
    Install-Abseil
    Install-Utf8proc

    if ($BuildApp)
    {
        Install-Tmbasic
    }
}

function Install-Cmake
{
    $version = $global:PackageVersions["CMAKE"]

    if ((Get-InstalledPackageVersion -Name "cmake") -eq $version)
    {
        return
    }
    
    if (-not $global:BuildDeps)
    {
        throw "CMake needs to be installed, but building is disabled."
    }

    # Get the native platform (not the target platform) as "arm64" or "x86_64"
    $nativePlatform = $null
    $nonNativePlatform = $null
    switch ($env:PROCESSOR_ARCHITECTURE)
    {
        "ARM64" { $nativePlatform = "arm64"; $nonNativePlatform = "x86_64" }
        "AMD64" { $nativePlatform = "x86_64"; $nonNativePlatform = "arm64" }
        default { throw "Unsupported processor architecture: $env:PROCESSOR_ARCHITECTURE" }
    }

    if (-not $global:UseS3Mirror)
    {
        # Download the non-native platform version of CMake, too, for the purpose of uploading to the S3 mirror.
        $nonNativeUrl = "https://github.com/Kitware/CMake/releases/download/v$version/cmake-$version-windows-$nonNativePlatform.zip"
        $nonNativeZipFilename = "cmake-$version-windows-$nonNativePlatform.zip"
        Get-DownloadedFile -Url $nonNativeUrl -Filename $nonNativeZipFilename
    }

    $url = "https://github.com/Kitware/CMake/releases/download/v$version/cmake-$version-windows-$nativePlatform.zip"
    $zipFilename = "cmake-$version-windows-$nativePlatform.zip"
    $filePath = Get-DownloadedFile -Url $url -Filename $zipFilename

    # Extract to a temporary directory
    Write-Host "Expanding $zipFilename."
    $tempDir = New-TemporaryDirectory

    Expand-Archive -Path $filePath -DestinationPath $tempDir

    # This has created a single folder called "cmake-*". Figure out what it is.
    $cmakeDir = Get-ChildItem -Path $tempDir -Recurse -Include "cmake-*" | Select-Object -First 1
    if ([string]::IsNullOrEmpty($cmakeDir))
    {
        throw "Failed to find cmake directory"
    }

    # Copy the contents of the cmake directory to the native prefix
    Write-Host "Copying cmake files to $global:NativePrefix."
    Copy-Item -Path (Join-Path $cmakeDir "*") -Destination $global:NativePrefix -Recurse -Force

    # Delete the temporary directory
    Write-Host "Deleting directory $tempDir."
    Remove-Item -Recurse -Force -Path $tempDir

    Set-InstalledPackage -Name "cmake" -Version $version
}

function Install-SevenZip
{
    # This old version from 2010 is the last one distributed as a zip.
    # It's an x86 binary which runs on all of our build platforms.
    $version = "9.20"

    if ((Get-InstalledPackageVersion -Name "7za") -eq $version)
    {
        return
    }
    
    if (-not $global:BuildDeps)
    {
        throw "7-Zip needs to be installed, but building is disabled."
    }

    $url = "https://www.7-zip.org/a/7za920.zip"
    $filePath = Get-DownloadedFile -Url $url -Filename "7za920.zip"
    $tempDir = New-TemporaryDirectory
    try
    {
        Expand-Archive -Path $filePath -DestinationPath $tempDir
        Move-Item (Join-Path $tempDir "7za.exe") (Join-Path $global:NativePrefix "bin\7za.exe") -Force
    }
    finally
    {
        Remove-Item -Recurse -Force -Path $tempDir
    }

    Set-InstalledPackage -Name "7za" -Version $version
}

function Install-GoogleTest
{
    $version = $global:PackageVersions["GOOGLETEST"]

    if ((Get-InstalledPackageVersion -Name "googletest") -eq $version)
    {
        return
    }
    
    if (-not $global:BuildDeps)
    {
        throw "GoogleTest needs to be installed, but building is disabled."
    }

    $url = "https://github.com/google/googletest/archive/refs/tags/v$version.zip"
    $filePath = Get-DownloadedFile -Url $url -Filename "googletest-$version.zip"
    Expand-ArchiveRenamingTopLevelDirectory -Path $filePath -DestinationPath (Join-Path $global:TargetPrefix "usr\src") -NewName "googletest"
    $srcDir = Join-Path $global:TargetPrefix "usr\src\googletest"

    $buildDir = Join-Path $srcDir "build"
    New-Directory $buildDir | Out-Null
    Push-Location $buildDir
    try
    {
        & cmake `
            -G "Visual Studio 17 2022" `
			-A "$global:Target_ARM64_Win32_x64" `
			"-DCMAKE_PREFIX_PATH=$global:TargetPrefix" `
			"-DCMAKE_INSTALL_PREFIX=$global:TargetPrefix" `
            $global:RuntimeFlag `
            ..
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to configure googletest"
        }

        & cmake --build . --config $global:BuildType
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to build googletest"
        }

        & cmake --install . --config $global:BuildType
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to install googletest"
        }
    }
    finally
    {
        Pop-Location
    }

    Set-InstalledPackage -Name "googletest" -Version $version
}

function Install-Fmt
{
    $version = $global:PackageVersions["FMT"]

    if ((Get-InstalledPackageVersion -Name "fmt") -eq $version)
    {
        return
    }
    
    if (-not $global:BuildDeps)
    {
        throw "fmt needs to be installed, but building is disabled."
    }

    $url = "https://github.com/fmtlib/fmt/archive/refs/tags/$version.zip"
    $filePath = Get-DownloadedFile -Url $url -Filename "fmt-$version.zip"
    Expand-ArchiveRenamingTopLevelDirectory -Path $filePath -DestinationPath (Join-Path $global:TargetPrefix "usr\src") -NewName "fmt"
    $srcDir = Join-Path $global:TargetPrefix "usr\src\fmt"

    $buildDir = Join-Path $srcDir "build"
    New-Directory $buildDir | Out-Null
    Push-Location $buildDir
    try
    {
        & cmake `
            -G "Visual Studio 17 2022" `
			-A "$global:Target_ARM64_Win32_x64" `
            -DFMT_TEST=OFF -DFMT_FUZZ=OFF -DFMT_CUDA_TEST=OFF -DFMT_DOC=OFF `
			"-DCMAKE_PREFIX_PATH=$global:TargetPrefix" `
			"-DCMAKE_INSTALL_PREFIX=$global:TargetPrefix" `
            $global:RuntimeFlag `
            ..
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to configure fmt"
        }

        & cmake --build . --config $global:BuildType
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to build fmt"
        }

        & cmake --install . --config $global:BuildType
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to install fmt"
        }

        # In debug builds, this produces "fmtd.lib" but we want "fmt.lib".
        if ($global:BuildType -eq "Debug")
        {
            Rename-Item -Path (Join-Path $global:TargetPrefix "lib\fmtd.lib") -NewName "fmt.lib"
        }
    }
    finally
    {
        Pop-Location
    }

    Set-InstalledPackage -Name "fmt" -Version $version
}

function Install-Immer
{
    $version = $global:PackageVersions["IMMER"]
    
    if ((Get-InstalledPackageVersion -Name "immer") -eq $version)
    {
        return
    }
    
    if (-not $global:BuildDeps)
    {
        throw "immer needs to be installed, but building is disabled."
    }

    $url = "https://github.com/arximboldi/immer/archive/refs/tags/v$version.zip"
    $filePath = Get-DownloadedFile -Url $url -Filename "immer-$version.zip"
    Expand-ArchiveRenamingTopLevelDirectory -Path $filePath -DestinationPath (Join-Path $global:TargetPrefix "usr\src") -NewName "immer"
    $srcDir = Join-Path $global:TargetPrefix "usr\src\immer"
    Copy-Item -Path (Join-Path $srcDir "immer") -Destination (Join-Path $global:TargetPrefix "include") -Recurse -Force

    Set-InstalledPackage -Name "immer" -Version $version
}

function Install-Mpdecimal
{
    $version = $global:PackageVersions["MPDECIMAL"]

    if ((Get-InstalledPackageVersion -Name "mpdecimal") -eq $version)
    {
        return
    }
    
    if (-not $global:BuildDeps)
    {
        throw "mpdecimal needs to be installed, but building is disabled."
    }

    $url = "https://www.bytereef.org/software/mpdecimal/releases/mpdecimal-$version.zip"
    $filePath = Get-DownloadedFile -Url $url -Filename "mpdecimal-$version.zip"
    Expand-ArchiveRenamingTopLevelDirectory -Path $filePath -DestinationPath (Join-Path $global:TargetPrefix "usr\src") -NewName "mpdecimal"
    $srcDir = Join-Path $global:TargetPrefix "usr\src\mpdecimal"

    Push-Location (Join-Path $srcDir "vcbuild")
    try
    {
        $batFile = $null
        $distDir = $null
        if ($global:Target_arm64_x86_x64 -eq "x86")
        {
            $batFile = (Join-Path $srcDir "vcbuild\vcbuild32.bat")
            $distDir = Join-Path $srcDir "vcbuild\dist32"
        }
        elseif ($global:Target_arm64_x86_x64 -eq "x64")
        {
            $batFile = (Join-Path $srcDir "vcbuild\vcbuild64.bat")
            $distDir = Join-Path $srcDir "vcbuild\dist64"
        }
        elseif ($global:Target_arm64_x86_x64 -eq "arm64")
        {
            $batFile = (Join-Path $srcDir "vcbuild\vcbuild_arm64.bat")
            $distDir = Join-Path $srcDir "vcbuild\dist_arm64"
        }

        # We're supposed to be able to pass /d to get a debug build, but for some reason it isn't working.
        # Let's instead modify the bat scripts directly.
        $batContent = [System.IO.File]::ReadAllText($batFile)
        if ($global:BuildType -eq "Debug")
        {
            $batContent = $batContent.Replace("set dbg=0", "set dbg=1")
        }
        else
        {
            $batContent = $batContent.Replace("set dbg=1", "set dbg=0")
        }
        [System.IO.File]::WriteAllText($batFile, $batContent)

        & cmd /c "$batFile"
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to build mpdecimal"
        }

        Copy-Item -Path (Join-Path $distDir "mpdecimal.h") -Destination (Join-Path $global:TargetPrefix "include\mpdecimal.h") -Force
        Copy-Item -Path (Join-Path $distDir "decimal.hh") -Destination (Join-Path $global:TargetPrefix "include\decimal.hh") -Force
        Copy-Item -Path (Join-Path $distDir "libmpdec-$version.lib") -Destination (Join-Path $global:TargetPrefix "lib\mpdec.lib") -Force
        Copy-Item -Path (Join-Path $distDir "libmpdec++-$version.lib") -Destination (Join-Path $global:TargetPrefix "lib\mpdec++.lib") -Force
    }
    finally
    {
        Pop-Location
    }

    Set-InstalledPackage -Name "mpdecimal" -Version $version
}

function Install-Tvision
{
    $version = $global:PackageVersions["TVISION"]

    if ((Get-InstalledPackageVersion -Name "tvision") -eq $version)
    {
        return
    }
    
    if (-not $global:BuildDeps)
    {
        throw "TVision needs to be installed, but building is disabled."
    }
    
    $url = "https://github.com/magiblot/tvision/archive/$version.zip"
    $filePath = Get-DownloadedFile -Url $url -Filename "tvision-$version.zip"
    Expand-ArchiveRenamingTopLevelDirectory -Path $filePath -DestinationPath (Join-Path $global:TargetPrefix "usr\src") -NewName "tvision"
    $srcDir = Join-Path $global:TargetPrefix "usr\src\tvision"

    Set-InstalledPackage -Name "tvision" -Version $version

    $buildTargetDir = Join-Path $srcDir "build"
    New-Directory $buildTargetDir | Out-Null
    Push-Location $buildTargetDir
    try
    {
        & cmake `
            -G "Visual Studio 17 2022" `
			-A "$global:Target_ARM64_Win32_x64" `
			"-DCMAKE_PREFIX_PATH=$global:TargetPrefix" `
			"-DCMAKE_INSTALL_PREFIX=$global:TargetPrefix" `
            -DTV_USE_STATIC_RTL=ON `
            -DTV_BUILD_EXAMPLES=OFF `
            $global:RuntimeFlag `
            ..
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to configure tvision"
        }

        & cmake --build . --config $global:BuildType
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to build tvision"
        }

        & cmake --install . --config $global:BuildType
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to install tvision"
        }
    }
    finally
    {
        Pop-Location
    }

    Set-InstalledPackage -Name "tvision" -Version $version
}

function Install-Turbo
{
    $version = $global:PackageVersions["TURBO"]

    if ((Get-InstalledPackageVersion -Name "turbo") -eq $version)
    {
        return
    }
    
    if (-not $global:BuildDeps)
    {
        throw "Turbo needs to be installed, but building is disabled."
    }

    $url = "https://github.com/magiblot/turbo/archive/$version.zip"
    $filePath = Get-DownloadedFile -Url $url -Filename "turbo-$version.zip"
    Expand-ArchiveRenamingTopLevelDirectory -Path $filePath -DestinationPath (Join-Path $global:TargetPrefix "usr\src") -NewName "turbo"
    $srcDir = Join-Path $global:TargetPrefix "usr\src\turbo"

    $buildDir = Join-Path $srcDir "build"
    New-Directory $buildDir | Out-Null
    Push-Location $buildDir
    try
    {
        & cmake `
            -G "Visual Studio 17 2022" `
			-A "$global:Target_ARM64_Win32_x64" `
			"-DCMAKE_PREFIX_PATH=$global:TargetPrefix" `
			"-DCMAKE_INSTALL_PREFIX=$global:TargetPrefix" `
            -DTURBO_USE_STATIC_RTL=ON `
			-DTURBO_USE_SYSTEM_TVISION=ON `
			-DTURBO_BUILD_APP=OFF `
            $global:RuntimeFlag `
            ..
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to configure turbo"
        }

        & cmake --build . --config $global:BuildType
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to build turbo"
        }

        & cmake --install . --config $global:BuildType
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to install turbo"
        }
    }
    finally
    {
        Pop-Location
    }

    Set-InstalledPackage -Name "turbo" -Version $version
}

function Install-Nameof
{
    $version = $global:PackageVersions["NAMEOF"]

    if ((Get-InstalledPackageVersion -Name "nameof") -eq $version)
    {
        return
    }
    
    if (-not $global:BuildDeps)
    {
        throw "nameof needs to be installed, but building is disabled."
    }

    $url = "https://github.com/Neargye/nameof/archive/refs/tags/v$version.zip"
    $filePath = Get-DownloadedFile -Url $url -Filename "nameof-$version.zip"
    Expand-ArchiveRenamingTopLevelDirectory -Path $filePath -DestinationPath (Join-Path $global:TargetPrefix "usr\src") -NewName "nameof"
    $srcDir = Join-Path $global:TargetPrefix "usr\src\nameof"

    Copy-Item -Path (Join-Path $srcDir "include\nameof.hpp") -Destination (Join-Path $global:NativePrefix "include") -Recurse -Force
    Copy-Item -Path (Join-Path $srcDir "include\nameof.hpp") -Destination (Join-Path $global:TargetPrefix "include") -Recurse -Force

    Set-InstalledPackage -Name "nameof" -Version $version
}

function Install-Zlib
{
    $version = $global:PackageVersions["ZLIB"]

    if ((Get-InstalledPackageVersion -Name "zlib") -eq $version)
    {
        return
    }
    
    if (-not $global:BuildDeps)
    {
        throw "zlib needs to be installed, but building is disabled."
    }
    
    $urlVersion = $version.Replace(".", "")
    $url = "https://zlib.net/zlib$urlVersion.zip"
    echo $url
    $filePath = Get-DownloadedFile -Url $url -Filename "zlib-$version.zip"
    Expand-ArchiveRenamingTopLevelDirectory -Path $filePath -DestinationPath (Join-Path $global:TargetPrefix "usr\src") -NewName "zlib"
    $srcDir = Join-Path $global:TargetPrefix "usr\src\zlib"

    # There is a zlibvc.vcxproj in usr\src\zlib\contrib\vstudio\vc17
    # Use that to build the static library.
    Push-Location (Join-Path $srcDir "contrib\vstudio\vc17")
    try
    {
        # The stock vcxproj builds a DLL but we want a LIB.
        # Rewrite zlibvc.vcxproj to replace "DynamicLibrary" with "StaticLibrary".
        $vcxprojPath = Join-Path $srcDir "contrib\vstudio\vc17\zlibvc.vcxproj"
        $vcxprojContent = [System.IO.File]::ReadAllText($vcxprojPath)
        $vcxprojContent = $vcxprojContent.Replace("DynamicLibrary", "StaticLibrary")
        
        # Also make sure to use the static runtime (/MT or /MTd)
        $newVcxprojContent = $vcxprojContent
        if ($global:BuildType -eq "Debug")
        {
            $newVcxprojContent = $newVcxprojContent.Replace("<RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>", "<RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>")
            $newVcxprojContent = $newVcxprojContent.Replace("<RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>", "<RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>")
        }
        else
        {
            $newVcxprojContent = $newVcxprojContent.Replace("<RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>", "<RuntimeLibrary>MultiThreaded</RuntimeLibrary>")
            $newVcxprojContent = $newVcxprojContent.Replace("<RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>", "<RuntimeLibrary>MultiThreaded</RuntimeLibrary>")
        }

        if ($newVcxprojContent -eq $vcxprojContent)
        {
            throw "Failed to patch zlibvc.vcxproj"
        }
        
        [System.IO.File]::WriteAllText($vcxprojPath, $newVcxprojContent)
    
        & "$global:Msbuild" zlibvc.vcxproj /p:Configuration=$global:BuildType /p:Platform=$global:Target_ARM64_Win32_x64
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to build zlib"
        }

        # Copy the static library to the lib directory
        Copy-Item -Path (Join-Path $srcDir "contrib\vstudio\vc17\$global:Target_arm64_x86_x64\ZlibDll$global:BuildType\zlibwapi.lib") -Destination (Join-Path $global:TargetPrefix "lib\z.lib") -Force

        # Copy the header files to the include directory
        Copy-Item -Path (Join-Path $srcDir "zlib.h") -Destination (Join-Path $global:TargetPrefix "include") -Force
        Copy-Item -Path (Join-Path $srcDir "zconf.h") -Destination (Join-Path $global:TargetPrefix "include") -Force
    }
    finally
    {
        Pop-Location
    }

    Set-InstalledPackage -Name "zlib" -Version $version
}

function Install-Microtar
{
    $version = $global:PackageVersions["MICROTAR"]

    if ((Get-InstalledPackageVersion -Name "microtar") -eq $version)
    {
        return
    }
    
    if (-not $global:BuildDeps)
    {
        throw "microtar needs to be installed, but building is disabled."
    }

    $url = "https://github.com/rxi/microtar/archive/$version.zip"
    $filePath = Get-DownloadedFile -Url $url -Filename "microtar-$version.zip"
    Expand-ArchiveRenamingTopLevelDirectory -Path $filePath -DestinationPath (Join-Path $global:TargetPrefix "usr\src") -NewName "microtar"
    $srcDir = Join-Path $global:TargetPrefix "usr\src\microtar"

    # Build the microtar library
    $cmakeListsContent = @"
cmake_minimum_required(VERSION 3.15)
project(microtar C)

add_library(microtar STATIC src/microtar.c)
target_include_directories(microtar PUBLIC src)
target_compile_definitions(microtar PRIVATE _CRT_SECURE_NO_WARNINGS)

install(TARGETS microtar
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib)
install(FILES src/microtar.h DESTINATION include)
"@
    Set-Content -Path (Join-Path $srcDir "CMakeLists.txt") -Value $cmakeListsContent

    $buildLibraryDir = Join-Path $srcDir "build-library"
    New-Directory $buildLibraryDir | Out-Null
    Push-Location $buildLibraryDir
    try
    {
        & cmake `
            -G "Visual Studio 17 2022" `
            -A "$global:Target_ARM64_Win32_x64" `
            "-DCMAKE_PREFIX_PATH=$global:TargetPrefix" `
            "-DCMAKE_INSTALL_PREFIX=$global:TargetPrefix" `
            $global:RuntimeFlag `
            ..
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to configure microtar"
        }

        & cmake --build . --config $global:BuildType
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to build microtar"
        }

        & cmake --install . --config $global:BuildType
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to install microtar"
        }

        Copy-Item -Path (Join-Path $buildLibraryDir "$global:BuildType\microtar.lib") -Destination (Join-Path $global:TargetPrefix "lib") -Force
    }
    finally
    {
        Pop-Location
    }

    # Copy mtar.c into the source directory.
    Copy-Item -Path (Join-Path $global:FilesDir "mtar.c") -Destination (Join-Path $srcDir "src") -Force

    # Build the mtar native executable
    $cmakeListsContent = @"
cmake_minimum_required(VERSION 3.15)
project(mtar C)

add_executable(mtar src/mtar.c src/microtar.c)
target_include_directories(mtar PUBLIC src)
target_compile_definitions(mtar PRIVATE _CRT_SECURE_NO_WARNINGS)
"@
    Set-Content -Path (Join-Path $srcDir "CMakeLists.txt") -Value $cmakeListsContent

    $buildProgramDir = Join-Path $srcDir "build-program"
    New-Directory $buildProgramDir | Out-Null
    Push-Location $buildProgramDir
    try
    {
        & cmake `
            -G "Visual Studio 17 2022" `
            "-DCMAKE_PREFIX_PATH=$global:TargetPrefix" `
            "-DCMAKE_INSTALL_PREFIX=$global:TargetPrefix" `
            $global:RuntimeFlag `
            ..
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to configure mtar"
        }

        & cmake --build . --config $global:BuildType
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to build mtar"
        }

        & cmake --install . --config $global:BuildType
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to install mtar"
        }

        Copy-Item -Path (Join-Path $buildProgramDir "$global:BuildType\mtar.exe") -Destination (Join-Path $global:NativePrefix "bin") -Force
    }
    finally
    {
        Pop-Location
    }   

    Set-InstalledPackage -Name "microtar" -Version $version
}

function Install-Libzip
{
    $version = $global:PackageVersions["LIBZIP"]

    if ((Get-InstalledPackageVersion -Name "libzip") -eq $version)
    {
        return
    }
    
    if (-not $global:BuildDeps)
    {
        throw "libzip needs to be installed, but building is disabled."
    }

    $url = "https://github.com/nih-at/libzip/releases/download/v$version/libzip-$version.tar.gz"
    $filePath = Get-DownloadedFile -Url $url -Filename "libzip-$version.tar.gz"
    Expand-TarballRenamingTopLevelDirectory -Path $filePath -DestinationPath (Join-Path $global:TargetPrefix "usr\src") -NewName "libzip"
    $srcDir = Join-Path $global:TargetPrefix "usr\src\libzip"

    # We must patch the CMakeLists.txt
    # 1. Upgrade `cmake_minimum_required(VERSION 3.10)` to `cmake_minimum_required(VERSION 3.15)`
    # 2. Insert `cmake_policy(SET CMP0091 NEW)` directly after that.
    $cmakeListsContent = [System.IO.File]::ReadAllText((Join-Path $srcDir "CMakeLists.txt"))
    $newCmakeListsContent = $cmakeListsContent.Replace("cmake_minimum_required(VERSION 3.10)", "cmake_minimum_required(VERSION 3.15)`ncmake_policy(SET CMP0091 NEW)`n")
    if ($newCmakeListsContent -eq $cmakeListsContent)
    {
        throw "Failed to patch libzip's CMakeLists.txt"
    }
    [System.IO.File]::WriteAllText((Join-Path $srcDir "CMakeLists.txt"), $newCmakeListsContent)

    $buildDir = Join-Path $srcDir "build"
    New-Directory $buildDir | Out-Null
    Push-Location $buildDir
    try
    {
        & cmake `
            -G "Visual Studio 17 2022" `
			-A "$global:Target_ARM64_Win32_x64" `
			-DBUILD_SHARED_LIBS=OFF `
			-DENABLE_COMMONCRYPTO=OFF `
			-DENABLE_GNUTLS=OFF `
			-DENABLE_MBEDTLS=OFF `
			-DENABLE_OPENSSL=OFF `
			-DENABLE_WINDOWS_CRYPTO=OFF `
			-DENABLE_BZIP2=OFF `
			-DENABLE_LZMA=OFF `
			-DENABLE_ZSTD=OFF `
			-DBUILD_TOOLS=OFF `
			-DBUILD_REGRESS=OFF `
            -DBUILD_OSSFUZZ=OFF `
			-DBUILD_EXAMPLES=OFF `
			-DBUILD_DOC=OFF `
			-DCMAKE_BUILD_TYPE=$global:BuildType `
			"-DCMAKE_PREFIX_PATH=$global:TargetPrefix" `
			"-DCMAKE_INSTALL_PREFIX=$global:TargetPrefix" `
			"-DZLIB_INCLUDE_DIR=$global:TargetPrefix\include" `
            $global:RuntimeFlag `
            ..
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to configure libzip"
        }

        & cmake --build . --config $global:BuildType
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to build libzip"
        }

        & cmake --install . --config $global:BuildType
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to install libzip"
        }
    }
    finally
    {
        Pop-Location
    }

    Set-InstalledPackage -Name "libzip" -Version $version
}

function Install-Cli11
{
    $version = $global:PackageVersions["CLI11"]

    if ((Get-InstalledPackageVersion -Name "cli11") -eq $version)
    {
        return
    }
    
    if (-not $global:BuildDeps)
    {
        throw "CLI11 needs to be installed, but building is disabled."
    }

    $url = "https://github.com/CLIUtils/CLI11/releases/download/v$version/CLI11.hpp"
    $filePath = Get-DownloadedFile -Url $url -Filename "cli11-$version.hpp"
    Copy-Item -Path $filePath -Destination (Join-Path $global:TargetPrefix "include\CLI11.hpp") -Force

    Set-InstalledPackage -Name "cli11" -Version $version
}

function Install-Abseil
{
    $version = $global:PackageVersions["ABSEIL"]

    if ((Get-InstalledPackageVersion -Name "abseil") -eq $version)
    {
        return
    }
    
    if (-not $global:BuildDeps)
    {
        throw "abseil needs to be installed, but building is disabled."
    }

    $url = "https://github.com/abseil/abseil-cpp/archive/$version.zip"
    $filePath = Get-DownloadedFile -Url $url -Filename "abseil-$version.zip"
    Expand-ArchiveRenamingTopLevelDirectory -Path $filePath -DestinationPath (Join-Path $global:TargetPrefix "usr\src") -NewName "abseil"
    $srcDir = Join-Path $global:TargetPrefix "usr\src\abseil"

    $buildDir = Join-Path $srcDir "build"
    New-Directory $buildDir | Out-Null
    Push-Location $buildDir
    try
    {
        & cmake `
            -G "Visual Studio 17 2022" `
            -A "$global:Target_ARM64_Win32_x64" `
			-DCMAKE_CXX_STANDARD=17 `
			-DABSL_PROPAGATE_CXX_STD=ON `
            "-DCMAKE_PREFIX_PATH=$global:TargetPrefix" `
            "-DCMAKE_INSTALL_PREFIX=$global:TargetPrefix" `
            -DABSL_MSVC_STATIC_RUNTIME=ON `
            $global:RuntimeFlag `
            ..
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to configure abseil"
        }

        & cmake --build . --config $global:BuildType
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to build abseil"
        }

        & cmake --install . --config $global:BuildType
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to install abseil"
        }
    }
    finally
    {
        Pop-Location
    }

    Set-InstalledPackage -Name "abseil" -Version $version
}

function Install-Utf8proc
{
    $version = $global:PackageVersions["UTF8PROC"]

    if ((Get-InstalledPackageVersion -Name "utf8proc") -eq $version)
    {
        return
    }

    $url = "https://github.com/JuliaStrings/utf8proc/archive/refs/tags/v$version.zip"
    $filePath = Get-DownloadedFile -Url $url -Filename "utf8proc-$version.zip"
    Expand-ArchiveRenamingTopLevelDirectory -Path $filePath -DestinationPath (Join-Path $global:TargetPrefix "usr\src") -NewName "utf8proc"
    $srcDir = Join-Path $global:TargetPrefix "usr\src\utf8proc"

    # We must patch the CMakeLists.txt
    # 1. Upgrade `cmake_minimum_required(VERSION 3.10)` to `cmake_minimum_required(VERSION 3.15)`
    # 2. Insert `cmake_policy(SET CMP0091 NEW)` directly after that.
    $cmakeListsContent = [System.IO.File]::ReadAllText((Join-Path $srcDir "CMakeLists.txt"))
    $newCmakeListsContent = $cmakeListsContent.Replace("cmake_minimum_required (VERSION 3.10)", "cmake_minimum_required(VERSION 3.15)`ncmake_policy(SET CMP0091 NEW)`n")
    if ($newCmakeListsContent -eq $cmakeListsContent)
    {
        throw "Failed to patch utf8proc's CMakeLists.txt"
    }
    [System.IO.File]::WriteAllText((Join-Path $srcDir "CMakeLists.txt"), $newCmakeListsContent)

    $buildDir = Join-Path $srcDir "build"
    New-Directory $buildDir | Out-Null
    Push-Location $buildDir
    try
    {
        & cmake `
            -G "Visual Studio 17 2022" `
            -A "$global:Target_ARM64_Win32_x64" `
            "-DCMAKE_PREFIX_PATH=$global:TargetPrefix" `
            "-DCMAKE_INSTALL_PREFIX=$global:TargetPrefix" `
            "-DBUILD_SHARED_LIBS=OFF" `
            $global:RuntimeFlag `
            ..
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to configure utf8proc"
        }

        & cmake --build . --config $global:BuildType
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to build utf8proc"
        }

        & cmake --install . --config $global:BuildType
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to install utf8proc"
        }

        Rename-Item -Path (Join-Path $global:TargetPrefix "lib\utf8proc_static.lib") -NewName "utf8proc.lib"
    }
    finally
    {
        Pop-Location
    }

    Set-InstalledPackage -Name "utf8proc" -Version $version
}

function Install-Tmbasic
{
    # Create cmake build directory
    $cmakeDir = Join-Path $global:RepositoryRoot "cmake"
    New-Directory $cmakeDir | Out-Null
    Push-Location $cmakeDir
    try
    {
        & cmake `
            -G "Visual Studio 17 2022" `
            -A "$global:Target_ARM64_Win32_x64" `
            "-DCMAKE_PREFIX_PATH=$global:TargetPrefix" `
            "-DTARGET_OS=win" `
            "-DARCH=$global:Host_arm64_x86_x64" `
            "-DTARGET_PREFIX=$global:TargetPrefix" `
            "-DCMAKE_BUILD_TYPE=$global:BuildType" `
            $global:RuntimeFlag `
            ..
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to configure tmbasic"
        }

        & cmake --build . --config $global:BuildType
        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to build tmbasic"
        }
    }
    finally
    {
        Pop-Location
    }
}

Main
