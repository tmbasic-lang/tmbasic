param
(
    [switch]$BuildDeps,
    [switch]$BuildApp,

    [Parameter(Mandatory = $true)]
    [ValidateSet("Release", "Debug")]
    [string]$BuildType,

    [switch]$UseR2Mirror
)

& "$PSScriptRoot/scripts/win/make.ps1" -Platform x86 -BuildDeps $BuildDeps -BuildApp $BuildApp -BuildType $BuildType -UseR2Mirror $UseR2Mirror
