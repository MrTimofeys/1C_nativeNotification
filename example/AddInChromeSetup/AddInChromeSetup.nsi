
!include "AddInChromeSetup.nsh"
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
RequestExecutionLevel user
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

; The stuff to install
Section "${Product}Setup"

  SectionIn RO
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
    
  ; Put file there
  File "${EXE_DIR}\${AddInAdaptor}"
  File "${EXE_DIR}\${AddInComponent}"
  File "${AddInJson}"
  File "${AddInFFJson}"
  
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
  WriteRegStr HKCU "Software\Google\Chrome\NativeMessagingHosts\${AddInName}" "" "$INSTDIR\${AddInJson}"
  WriteRegStr HKCU "Software\Mozilla\NativeMessagingHosts\${AddInName}" "" "$INSTDIR\${AddInFFJson}"
  WriteRegStr HKCU "SOFTWARE\Chromium\NativeMessagingHosts\${AddInName}" "" "$INSTDIR\${AddInJson}"
  WriteRegStr HKCU "SOFTWARE\Microsoft\Edge\NativeMessagingHosts\${AddInName}" "" "$INSTDIR\${AddInJson}"
  WriteUninstaller "uninstall.exe"
  
SectionEnd
;--------------------------------

; Uninstaller

Section "Uninstall"
  SetAutoClose true
  ; Remove registry keys
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName} ${ProductVersion}"
  DeleteRegKey HKCU "SOFTWARE\${ProductName} ${ProductVersion}"
  DeleteRegKey HKCU "Software\Google\Chrome\NativeMessagingHosts\${AddInName}"
  DeleteRegKey HKCU "Software\Mozilla\NativeMessagingHosts\${AddInName}"
  DeleteRegKey HKCU "SOFTWARE\Chromium\NativeMessagingHosts\${AddInName}"
  DeleteRegKey HKCU "SOFTWARE\Microsoft\Edge\NativeMessagingHosts\${AddInName}"

  ; Remove files and uninstaller
  Delete $INSTDIR\${AddInAdaptor}
  Delete $INSTDIR\${AddInComponent}
  Delete $INSTDIR\${AddInName}.json
  Delete $INSTDIR\${AddInName}.ff.json
  Delete $INSTDIR\uninstall.exe

  RMDir "$INSTDIR"
  ${GetParent} "$INSTDIR" $R0
  RMDir "$R0"

SectionEnd
Function un.OnUninstSuccess
     HideWindow
FunctionEnd