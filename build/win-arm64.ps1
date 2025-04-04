param
(
    [switch]$BuildDeps,
    [switch]$BuildApp,

    [Parameter(Mandatory = $true)]
    [ValidateSet("Release", "Debug")]
    [string]$BuildType,

    [switch]$UseS3Mirror
)

& "$PSScriptRoot/scripts/win/make.ps1" -Platform arm64 -BuildDeps $BuildDeps -BuildApp $BuildApp -BuildType $BuildType -UseS3Mirror $UseS3Mirror
