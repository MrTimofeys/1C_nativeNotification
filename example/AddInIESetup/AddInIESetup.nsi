
!include "AddInIESetup.nsh"
; The name of the installer
Name "${ProductName} (${SuffixPF})"

; The file to write
OutFile "${TMP_DIR}\${OutFileName}${Bits}.exe"
!include "FileFunc.nsh"

; The default installation directory
InstallDir "$APPDATA\${Manufacturer}\${ProductName}\${ProductVersion}\${SuffixPF}\"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKCU "Software\${Product}_${ProductVersion}_${SuffixPF}" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin
XPStyle on
;Unicode true
;--------------------------------

; Pages

;Page components
;Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------
Function .onInit
    SetShellVarContext all
    StrCpy $INSTDIR $APPDATA\${Manufacturer}\${Product}\${ProductVersion}\${SuffixPF}
FunctionEnd 
; The stuff to install
Section "${Product}Setup"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
    
  ; Put file there
  File "${EXE_DIR}\${AddInAdaptor}"
  RegDLL "$INSTDIR\${AddInAdaptor}"
  File "${EXE_DIR}\${AddInComponent}"

  ; Write the installation path into the registry
  WriteRegStr HKCU "SOFTWARE\${ProductName} ${ProductVersion}" "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName} ${ProductVersion}" "DisplayName" "${ProductName}"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName} ${ProductVersion}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName} ${ProductVersion}" "InstallLocation" '"$INSTDIR"'
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName} ${ProductVersion}" "DisplayVersion" "${ProductVersion}"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName} ${ProductVersion}" "Publisher" "${Manufacturer}"
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName} ${ProductVersion}" "NoModify" 1
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName} ${ProductVersion}" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd
;--------------------------------

; Uninstaller

Section "Uninstall"
  SetAutoClose true
  ; Remove registry keys
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName} ${ProductVersion}"
  DeleteRegKey HKCU "SOFTWARE\${ProductName} ${ProductVersion}"

  ; Remove files and uninstaller
  UnRegDLL $INSTDIR\${AddInAdaptor}
  Delete $INSTDIR\${AddInAdaptor}
  Delete $INSTDIR\${AddInComponent}
  Delete $INSTDIR\uninstall.exe

  RMDir "$INSTDIR"
  ${GetParent} "$INSTDIR" $R0
  RMDir "$R0"

SectionEnd
Function un.OnUninstSuccess
     HideWindow
FunctionEnd
