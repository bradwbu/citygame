# This script assumes that the powershell script executionpolicy is set correctly
# and MSBuild is reachable

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Invoke-Build {
    param(
        [string] $Solution,
        [string] $Configuration,
        [string] $Platform,
        [string] $Target
    )

    if ($Target -ne "") {
        msbuild "$Solution" /t:"$Target" /p:Configuration="$Configuration" /p:Platform="$Platform"
    } else {
        msbuild "$Solution" /p:Configuration="$Configuration" /p:Platform="$Platform"
    }
    
    if ($LASTEXITCODE -ne 0) { 
        exit
    }
}

function Invoke-BuildAll {
    param (
        [string] $Solution,
        [string] $Platform
    )

    Invoke-Build -Solution $Solution -Configuration "Debug" -Platform "$Platform"
    Invoke-Build -Solution $Solution -Configuration "Release" -Platform "$Platform"
}

Invoke-BuildAll "..\3rdparty\cryptopp\cryptlib.sln" -Platform "Win32"
Invoke-BuildAll "..\3rdparty\cryptopp\cryptlib.sln" -Platform "x64"

Invoke-BuildAll "..\3rdparty\cryptopp-8.3\build\vs2019\cryptlib.sln" -Platform "x86"
Invoke-BuildAll "..\3rdparty\cryptopp-8.3\build\vs2019\cryptlib.sln" -Platform "x64"

Invoke-BuildAll "..\3rdparty\zlibsrc\zlibvc.sln" -Platform "Win32"
Invoke-BuildAll "..\3rdparty\zlibsrc\zlibvc.sln" -Platform "x64"

Invoke-BuildAll "..\3rdparty\zlib-1.2.11\build\vs2019\zlib.sln" -Platform "x86"
Invoke-BuildAll "..\3rdparty\zlib-1.2.11\build\vs2019\zlib.sln" -Platform "x64"

Invoke-BuildAll "..\3rdparty\bzip2-1.0.6\build\vs2019\libbz2.sln" -Platform "x86"
Invoke-BuildAll "..\3rdparty\bzip2-1.0.6\build\vs2019\libbz2.sln" -Platform "x64"

Invoke-BuildAll "..\3rdparty\zeromq2-1\builds\msvc\msvc.sln" -Platform "Win32"
Invoke-BuildAll "..\3rdparty\zeromq2-1\builds\msvc\msvc.sln" -Platform "x64"

Invoke-BuildAll "..\3rdparty\IJGWin32\IJGWin32.sln" -Platform "Win32"

Invoke-BuildAll "..\3rdparty\yajl\yajl_s.sln" -Platform "Win32"
Invoke-BuildAll "..\3rdparty\yajl\yajl_s.sln" -Platform "x64"

Invoke-BuildAll "..\Utilities\StructParser\StructParser.sln" -Platform "Win32"

Invoke-BuildAll "..\AuthServer\build\vs2019\AuthServer.sln" -Platform "x86"
Invoke-BuildAll "..\AuthServer\build\vs2019\AuthServer.sln" -Platform "x64"

Invoke-Build "MasterSolution.sln" -Configuration "Opt Debug" -Platform "Win32"
Invoke-Build "MasterSolution.sln" -Configuration "Release" -Platform "Win32"
