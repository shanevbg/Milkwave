# Script to extract plugin.cpp into multiple files
# Run from vis_milk2 directory

$srcFile = "C:\Source\Milkwave\Visualizer\milkwave\plugin.cpp"
$lines = Get-Content $srcFile
$totalLines = $lines.Count
Write-Output "Source file: $totalLines lines"

function ExtractLines($start, $end) {
    # 1-based inclusive
    return $lines[($start-1)..($end-1)]
}

function WriteFile($path, $header, [string[]]$sections) {
    $content = @()
    $content += $header
    foreach ($s in $sections) {
        $content += $s
    }
    Set-Content -Path $path -Value ($content -join "`n") -Encoding UTF8
    Write-Output "  Created: $path ($($content.Count) lines)"
}

# Common header for all extracted files
$commonIncludes = @'
#include "plugin.h"
#include "plugin_internals.h"
'@

# =====================================================================
# plugin_config.cpp: Configuration, initialization, menus
# =====================================================================
Write-Output "Extracting plugin_config.cpp..."
$configHeader = @"
// plugin_config.cpp - Configuration, initialization, and menu building
// Extracted from plugin.cpp

$commonIncludes
#include "utility.h"
#include "support.h"
#include "resource.h"
#include "defines.h"
#include "shell_defines.h"
#include "wasabi.h"

"@

$configLines = @()
# OverrideDefaults through SaveWindowSizeAndPosition: L1013-1665
$configLines += ExtractLines 1013 1665
$configLines += ""
# Randomize: L8089-8102 (up to SetMenusForPresetVersion)
$configLines += ExtractLines 8089 8103
# SetMenusForPresetVersion: L8104-8127
$configLines += ExtractLines 8104 8128
# BuildMenus: L8129-8335
$configLines += ExtractLines 8129 8336
# WriteRealtimeConfig: L8337-8341
$configLines += ExtractLines 8337 8342
# dumpmsg: L8343-8352
$configLines += ExtractLines 8343 8353

WriteFile "plugin_config.cpp" $configHeader $configLines

# =====================================================================
# plugin_dx9.cpp: DirectX 9 resource management
# =====================================================================
Write-Output "Extracting plugin_dx9.cpp..."
$dx9Header = @"
// plugin_dx9.cpp - DirectX 9 resource allocation and cleanup
// Extracted from plugin.cpp

$commonIncludes
#include "utility.h"
#include "support.h"
#include "resource.h"
#include "defines.h"
#include "shell_defines.h"
#include "wasabi.h"

"@

$dx9Lines = @()
# AllocateMyNonDx9Stuff: L1730-1803 (CancelThread is at 1804, keep in plugin.cpp)
$dx9Lines += ExtractLines 1730 1803
$dx9Lines += ""
# CleanUpMyNonDx9Stuff: L1822-1883 (SquishToCenter is at 1884, keep in plugin.cpp)
$dx9Lines += ExtractLines 1822 1883
$dx9Lines += ""
# AllocateMyDX9Stuff: L1901-2827
$dx9Lines += ExtractLines 1901 2827
$dx9Lines += ""
# AddNoiseTex: L2862-2986
$dx9Lines += ExtractLines 2862 2986
$dx9Lines += ""
# AddNoiseVol: L2987-3135 (VShaderInfo::Clear is at 3136)
$dx9Lines += ExtractLines 2987 3135
$dx9Lines += ""
# CleanUpMyDX9Stuff: L4110-4265
$dx9Lines += ExtractLines 4110 4265

WriteFile "plugin_dx9.cpp" $dx9Header $dx9Lines

# =====================================================================
# plugin_shaders.cpp: Shader compilation and parameter caching
# =====================================================================
Write-Output "Extracting plugin_shaders.cpp..."
$shadersHeader = @"
// plugin_shaders.cpp - Shader compilation, parameter caching, and texture management
// Extracted from plugin.cpp

$commonIncludes
#include "utility.h"
#include "support.h"
#include "resource.h"
#include "defines.h"
#include "shell_defines.h"
#include "wasabi.h"
#include <cstdint>

"@

$shadersLines = @()
# VShaderInfo::Clear through CShaderParams::CacheParams end (L3136-3724)
$shadersLines += ExtractLines 3136 3724
$shadersLines += ""
# RecompileVShader through LoadShaderFromMemory end (L3725-4109)
$shadersLines += ExtractLines 3725 4109
$shadersLines += ""
# CompilePresetShadersToFile: L9881-9892
$shadersLines += ExtractLines 9881 9892
$shadersLines += ""
# GenWarpPShaderText: L13063-13086
$shadersLines += ExtractLines 13063 13086
$shadersLines += ""
# GenCompPShaderText: L13087-13129
$shadersLines += ExtractLines 13087 13129
$shadersLines += ""
# SaveShaderBytecodeToFile: L13253-13275
$shadersLines += ExtractLines 13253 13275
$shadersLines += ""
# LoadShaderBytecodeFromFile: L13276-13298
$shadersLines += ExtractLines 13276 13298
$shadersLines += ""
# crc32: L13299-13312
$shadersLines += ExtractLines 13299 13312

WriteFile "plugin_shaders.cpp" $shadersHeader $shadersLines

# =====================================================================
# plugin_render.cpp: Main rendering (MyRenderFn + helpers)
# =====================================================================
Write-Output "Extracting plugin_render.cpp..."
$renderHeader = @"
// plugin_render.cpp - Main frame rendering
// Extracted from plugin.cpp

$commonIncludes
#include "utility.h"
#include "support.h"
#include "resource.h"
#include "defines.h"
#include "shell_defines.h"
#include "wasabi.h"

"@

$renderLines = @()
# MyRenderFn: L4266-4609
$renderLines += ExtractLines 4266 4609
$renderLines += ""
# DrawTooltip: L4610-4728
$renderLines += ExtractLines 4610 4728
$renderLines += ""
# OnAltK: L4729-4731
$renderLines += ExtractLines 4729 4732
$renderLines += ""
# AddNotification overloads + AddNotificationAudioDevice: L4733-4779
$renderLines += ExtractLines 4733 4779
$renderLines += ""
# AddError: L4780-4795
$renderLines += ExtractLines 4780 4795
$renderLines += ""
# ClearErrors: L4796-4806
$renderLines += ExtractLines 4796 4806

WriteFile "plugin_render.cpp" $renderHeader $renderLines

# =====================================================================
# plugin_renderui.cpp: UI rendering overlay (MyRenderUI)
# =====================================================================
Write-Output "Extracting plugin_renderui.cpp..."
$renderuiHeader = @"
// plugin_renderui.cpp - UI overlay rendering (MyRenderUI)
// Extracted from plugin.cpp

$commonIncludes
#include "utility.h"
#include "support.h"
#include "resource.h"
#include "defines.h"
#include "shell_defines.h"
#include "wasabi.h"

"@

$renderuiLines = @()
# MyRenderUI: L4807-5854
$renderuiLines += ExtractLines 4807 5854

WriteFile "plugin_renderui.cpp" $renderuiHeader $renderuiLines

# =====================================================================
# plugin_input.cpp: Input handling (MyWindowProc + HandleRegularKey)
# =====================================================================
Write-Output "Extracting plugin_input.cpp..."
$inputHeader = @"
// plugin_input.cpp - Window procedure and keyboard input handling
// Extracted from plugin.cpp

$commonIncludes
#include "utility.h"
#include "support.h"
#include "resource.h"
#include "defines.h"
#include "shell_defines.h"
#include "wasabi.h"
#include <shellapi.h>

"@

$inputLines = @()
# ToggleAlwaysOnTop: L5855-5871
$inputLines += ExtractLines 5855 5871
$inputLines += ""
# ToggleTransparency (free fn): L5872-5903
$inputLines += ExtractLines 5872 5903
$inputLines += ""
# SetOpacity: L5904-5952
$inputLines += ExtractLines 5904 5952
$inputLines += ""
# ToggleWindowOpacity (free fn): L5953-5983
$inputLines += ExtractLines 5953 5983
$inputLines += ""
# IsBorderlessFullscreen: L5984-5999
$inputLines += ExtractLines 5984 5999
$inputLines += ""
# LoadPresetFilesViaDragAndDrop (free fn): L6000-6045
$inputLines += ExtractLines 6000 6045
$inputLines += ""
# MyWindowProc: L6046-7484
$inputLines += ExtractLines 6046 7484
$inputLines += ""
# KillAllSprites: L7485-7490
$inputLines += ExtractLines 7485 7490
$inputLines += ""
# KillAllSupertexts: L7491-7497
$inputLines += ExtractLines 7491 7497
$inputLines += ""
# ChangePresetDir: L7498-7534
$inputLines += ExtractLines 7498 7534
$inputLines += ""
# ToggleSpout: L7535-7559
$inputLines += ExtractLines 7535 7559
$inputLines += ""
# SetSpoutFixedSize: L7560-7609
$inputLines += ExtractLines 7560 7609
$inputLines += ""
# HandleRegularKey: L7610-8039
$inputLines += ExtractLines 7610 8039
$inputLines += ""
# SaveCurrentPresetToQuicksave: L8040-8088
$inputLines += ExtractLines 8040 8088

WriteFile "plugin_input.cpp" $inputHeader $inputLines

# =====================================================================
# plugin_blendpatterns.cpp: Blend transition patterns
# =====================================================================
Write-Output "Extracting plugin_blendpatterns.cpp..."
$blendHeader = @"
// plugin_blendpatterns.cpp - Preset blend transition pattern generation
// Extracted from plugin.cpp

$commonIncludes
#include <cmath>

"@

$blendLines = @()
# RandomizeBlendPattern: L8498-9834
$blendLines += ExtractLines 8498 9834
$blendLines += ""
# GenPlasma: L9835-9880
$blendLines += ExtractLines 9835 9880

WriteFile "plugin_blendpatterns.cpp" $blendHeader $blendLines

# =====================================================================
# plugin_presets.cpp: Preset loading and list management
# =====================================================================
Write-Output "Extracting plugin_presets.cpp..."
$presetsHeader = @"
// plugin_presets.cpp - Preset loading, list management, and milk2 parsing
// Extracted from plugin.cpp

$commonIncludes
#include "utility.h"
#include "support.h"
#include "resource.h"
#include "defines.h"
#include "shell_defines.h"
#include "wasabi.h"
#include <process.h>

"@

$presetsLines = @()
# PrevPreset: L8354-8381
$presetsLines += ExtractLines 8354 8381
$presetsLines += ""
# NextPreset: L8382-8386
$presetsLines += ExtractLines 8382 8386
$presetsLines += ""
# LoadRandomPreset: L8387-8497
$presetsLines += ExtractLines 8387 8497
$presetsLines += ""
# ClearPreset: L9893-9916
$presetsLines += ExtractLines 9893 9916
$presetsLines += ""
# RemoveAngleBrackets: L9917-9935
$presetsLines += ExtractLines 9917 9935
$presetsLines += ""
# Milk2PatternNameToMixtype (static free fn): L9936-9972
$presetsLines += ExtractLines 9936 9972
$presetsLines += ""
# ParseMilk2File: L9973-10208
$presetsLines += ExtractLines 9973 10208
$presetsLines += ""
# LoadPreset: L10209-10410
$presetsLines += ExtractLines 10209 10410
$presetsLines += ""
# OnFinishedLoadingPreset: L10411-10425
$presetsLines += ExtractLines 10411 10425
$presetsLines += ""
# SendMessageToMilkwaveRemote (2 overloads): L10426-10453
$presetsLines += ExtractLines 10426 10453
$presetsLines += ""
# PostMessageToMilkwaveRemote: L10454-10470
$presetsLines += ExtractLines 10454 10470
$presetsLines += ""
# LoadPresetTick: L10471-10567
$presetsLines += ExtractLines 10471 10567
$presetsLines += ""
# SeekToPreset: L10568-10582
$presetsLines += ExtractLines 10568 10582
$presetsLines += ""
# FindValidPresetDir: L10583-10916
$presetsLines += ExtractLines 10583 10916
$presetsLines += ""
# UpdatePresetList: L10917-10982
$presetsLines += ExtractLines 10917 10982
$presetsLines += ""
# MergeSortPresets: L10983-11042
$presetsLines += ExtractLines 10983 11042
$presetsLines += ""
# SavePresetAs: L11285-11305
$presetsLines += ExtractLines 11285 11305
$presetsLines += ""
# DeletePresetFile: L11306-11327
$presetsLines += ExtractLines 11306 11327
$presetsLines += ""
# RenamePresetFile: L11328-11383
$presetsLines += ExtractLines 11328 11383
$presetsLines += ""
# UpdatePresetRatings: L11384-11426
$presetsLines += ExtractLines 11384 11426
$presetsLines += ""
# SetCurrentPresetRating: L11427-11467
$presetsLines += ExtractLines 11427 11467

WriteFile "plugin_presets.cpp" $presetsHeader $presetsLines

# =====================================================================
# plugin_waitstring.cpp: Text editing functions
# =====================================================================
Write-Output "Extracting plugin_waitstring.cpp..."
$waitstringHeader = @"
// plugin_waitstring.cpp - Text editing (WaitString) functions
// Extracted from plugin.cpp

$commonIncludes

"@

$waitstringLines = @()
# WaitString_* functions: L11043-11284
$waitstringLines += ExtractLines 11043 11284

WriteFile "plugin_waitstring.cpp" $waitstringHeader $waitstringLines

# =====================================================================
# plugin_messages.cpp: Custom messages and song title animations
# =====================================================================
Write-Output "Extracting plugin_messages.cpp..."
$messagesHeader = @"
// plugin_messages.cpp - Custom messages and song title animations
// Extracted from plugin.cpp

$commonIncludes
#include "utility.h"
#include "support.h"
#include "resource.h"
#include "defines.h"
#include "shell_defines.h"
#include "wasabi.h"
#include <strsafe.h>

"@

$messagesLines = @()
# ReadCustomMessages: L11468-11570
$messagesLines += ExtractLines 11468 11570
$messagesLines += ""
# LaunchCustomMessage: L11571-11663
$messagesLines += ExtractLines 11571 11663
$messagesLines += ""
# LaunchSongTitleAnim: L11664-11697
$messagesLines += ExtractLines 11664 11697
$messagesLines += ""
# LaunchMessage: L11698-12257
$messagesLines += ExtractLines 11698 12257

WriteFile "plugin_messages.cpp" $messagesHeader $messagesLines

# =====================================================================
# Rebuild plugin.cpp with remaining code only
# =====================================================================
Write-Output ""
Write-Output "Building reduced plugin.cpp..."

$remainingLines = @()

# L1-1012: License, includes, globals, free utility functions  
$remainingLines += ExtractLines 1 1012

# L1667-1729: ConvertLLCto1310, StripComments (utility), CancelThread starts at 1804 but first:
$remainingLines += ""
$remainingLines += "//----------------------------------------------------------------------"
$remainingLines += ""
$remainingLines += ExtractLines 1667 1729

# L1804-1900: CancelThread, SquishToCenter, GetNearestPow2Size
$remainingLines += ""
$remainingLines += ExtractLines 1804 1900

# L2828-2861: fCubicInterpolate, dwCubicInterpolate  
$remainingLines += ""
$remainingLines += ExtractLines 2828 2861

# L12258-13252: Misc remaining methods (remote messaging, screenshots, sprites, sound analysis, etc.)
# But skip 13063-13129 (GenWarpPShaderText, GenCompPShaderText) and 13253-13312 (shader bytecode/crc32) - moved to shaders
$remainingLines += ""
$remainingLines += "//----------------------------------------------------------------------"
$remainingLines += "// Remaining utility and misc methods"
$remainingLines += "//----------------------------------------------------------------------"
$remainingLines += ""
$remainingLines += ExtractLines 12258 13062

# L13130-13252: GetSongTitle, OpenSender, OpenMilkwaveRemote, SetAudioDeviceDisplayName, SetAMDFlag
$remainingLines += ""
$remainingLines += ExtractLines 13130 13252

# L13313-13409: CheckDX9DLL, CheckForDirectX9c, ShowDirectXMissingMessage, CompileInputMixShader
$remainingLines += ""
$remainingLines += ExtractLines 13313 13409

# Backup original and write new
Copy-Item $srcFile "$srcFile.bak" -Force
Write-Output "  Backed up original to plugin.cpp.bak"

Set-Content -Path $srcFile -Value ($remainingLines -join "`n") -Encoding UTF8
Write-Output "  Wrote reduced plugin.cpp ($($remainingLines.Count) lines)"

Write-Output ""
Write-Output "Extraction complete!"
Write-Output "Original: $totalLines lines"
Write-Output "Reduced plugin.cpp: $($remainingLines.Count) lines"
