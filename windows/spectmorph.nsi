; setup-myprog.nsi
;--------------------------------
!include "MUI2.nsh"
!include x64.nsh
!insertmacro MUI_UNPAGE_CONFIRM

Var pluginDir

; The name of the installer
Name "SpectMorph 0.4.0 Setup"
; The file to write
OutFile "setup-spectmorph-0.4.0.exe"
; The default installation directory
InstallDir "$PROGRAMFILES64\SpectMorph"

;;; Directory page for the VST Plugin
!define MUI_DIRECTORYPAGE_TEXT_TOP \
"Setup will install the SpectMorph 64 bit VST plugin. Please select the VST path to your preferred DAW software."
!define MUI_DIRECTORYPAGE_TEXT_DESTINATION \
"Choose the folder in which to the VST Plugin."
!define MUI_DIRECTORYPAGE_VARIABLE $pluginDir
!insertmacro MUI_PAGE_DIRECTORY

; Directory page for SpectMorph
!define MUI_DIRECTORYPAGE_TEXT_TOP \
"The SpectMorph plugin requires additional Data and Program Files."
!define MUI_DIRECTORYPAGE_TEXT_DESTINATION \
"Choose the folder in which to install Data and Program Files."
!insertmacro MUI_PAGE_DIRECTORY

; Instfiles page
!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_TITLE "Welcome to SpectMorph"
!define MUI_FINISHPAGE_TEXT "SpectMorph is installed in$\r$\n$\r$\n$pluginDir$\r$\n$\r$\nnow."
!define MUI_FINISHPAGE_LINK "SpectMorph Homepage"
!define MUI_FINISHPAGE_LINK_LOCATION "http://www.spectmorph.org"
!define MUI_FINISHPAGE_NOREBOOTSUPPORT
!insertmacro MUI_PAGE_FINISH

Function .onInit
	ClearErrors
	StrCpy $pluginDir "$PROGRAMFILES64\Steinberg\VstPlugins"
FunctionEnd

;--------------------------------
; The stuff to install
Section "Install" ;No components page, name is not important
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  File /r instruments
  File /r templates

  SetOutPath $pluginDir
  ; Put archive file there
  File SpectMorph.dll
  CreateShortCut "SpectMorph.Data.lnk" "$INSTDIR"

  CreateShortCut "$SMPROGRAMS\SpectMorph\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd ; end the section

;--------------------------------
;Uninstaller Section

Section "Uninstall"
  RMDir /r "$SMPROGRAMS\SpectMorph"
  RMDir /r "$INSTDIR"
SectionEnd
