############################################################################################
#      NSIS Installation Script created by NSIS Quick Setup Script Generator v1.09.18
#               Entirely Edited with NullSoft Scriptable Installation System                
#              by Vlasis K. Barkas aka Red Wine red_wine@freemail.gr Sep 2006               
############################################################################################

!define /date NOW "%Y%m%d-%H%M%S"
!define APP_NAME "YuvToolkit"
!define COMP_NAME "yuvtoolkit.com"
!define WEB_SITE "http://www.yuvtoolkit.com"
!define /file VERSION_1 VERSION_1
!define /file VERSION_2 VERSION_2
!define /file VERSION_3 VERSION_3
!define /file VERSION_4 VERSION_4
!define VERSION "${VERSION_1}.${VERSION_2}.${VERSION_3}.${VERSION_4}"
!define COPYRIGHT "David Yuheng Zhao © 2010-2011"
!define DESCRIPTION "YUV video player"
!define LICENSE_TXT "..\Doc\license.txt"
!define INSTALLER_NAME "YUVToolkit-${VERSION}.exe"
!define MAIN_APP_EXE "YUVToolkit.exe"
!define INSTALL_TYPE "SetShellVarContext all"
!define REG_ROOT "HKLM"
!define REG_APP_PATH "Software\Microsoft\Windows\CurrentVersion\App Paths\${MAIN_APP_EXE}"
!define UNINSTALL_PATH "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"

######################################################################

VIProductVersion  "${VERSION}"
VIAddVersionKey "ProductName"  "${APP_NAME}"
VIAddVersionKey "CompanyName"  "${COMP_NAME}"
VIAddVersionKey "LegalCopyright"  "${COPYRIGHT}"
VIAddVersionKey "FileDescription"  "${DESCRIPTION}"
VIAddVersionKey "FileVersion"  "${VERSION}"

######################################################################

SetCompressor ZLIB
Name "${APP_NAME}"
Caption "${APP_NAME}"
OutFile "${INSTALLER_NAME}"
BrandingText "${APP_NAME}"
XPStyle on
InstallDirRegKey "${REG_ROOT}" "${REG_APP_PATH}" ""
InstallDir "$PROGRAMFILES\YUVToolkit"

######################################################################

!include "MUI.nsh"

!define MUI_ABORTWARNING
!define MUI_UNABORTWARNING

!insertmacro MUI_PAGE_WELCOME

!ifdef LICENSE_TXT
!insertmacro MUI_PAGE_LICENSE "${LICENSE_TXT}"
!endif

!insertmacro MUI_PAGE_DIRECTORY

!ifdef REG_START_MENU
!define MUI_STARTMENUPAGE_NODISABLE
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "YUVToolkit"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "${REG_ROOT}"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${UNINSTALL_PATH}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "${REG_START_MENU}"
!insertmacro MUI_PAGE_STARTMENU Application $SM_Folder
!endif

!insertmacro MUI_PAGE_INSTFILES

# !define MUI_FINISHPAGE_RUN "$INSTDIR\${MAIN_APP_EXE}"
!define MUI_FINISHPAGE_SHOWREADME $INSTDIR\readme.txt
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM

!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

######################################################################

Section -MainProgram
${INSTALL_TYPE}
SetOverwrite ifnewer
SetOutPath "$INSTDIR"

!include "file_list.txt"

SectionEnd


######################################################################

Section -Icons_Reg
SetOutPath "$INSTDIR"
WriteUninstaller "$INSTDIR\uninstall.exe"

!ifdef REG_START_MENU
!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
CreateDirectory "$SMPROGRAMS\$SM_Folder"
CreateShortCut "$SMPROGRAMS\$SM_Folder\${APP_NAME}.lnk" "$INSTDIR\${MAIN_APP_EXE}"
CreateShortCut "$SMPROGRAMS\$SM_Folder\Uninstall ${APP_NAME}.lnk" "$INSTDIR\uninstall.exe"

!ifdef WEB_SITE
WriteIniStr "$INSTDIR\${APP_NAME} website.url" "InternetShortcut" "URL" "${WEB_SITE}"
CreateShortCut "$SMPROGRAMS\$SM_Folder\${APP_NAME} Website.lnk" "$INSTDIR\${APP_NAME} website.url"
!endif
!insertmacro MUI_STARTMENU_WRITE_END
!endif

!ifndef REG_START_MENU
CreateDirectory "$SMPROGRAMS\YUVToolkit"
CreateShortCut "$SMPROGRAMS\YUVToolkit\${APP_NAME}.lnk" "$INSTDIR\${MAIN_APP_EXE}"
CreateShortCut "$SMPROGRAMS\YUVToolkit\Uninstall ${APP_NAME}.lnk" "$INSTDIR\uninstall.exe"

!ifdef WEB_SITE
WriteIniStr "$INSTDIR\${APP_NAME} website.url" "InternetShortcut" "URL" "${WEB_SITE}"
CreateShortCut "$SMPROGRAMS\YUVToolkit\${APP_NAME} Website.lnk" "$INSTDIR\${APP_NAME} website.url"
!endif
!endif

WriteRegStr ${REG_ROOT} "${REG_APP_PATH}" "" "$INSTDIR\${MAIN_APP_EXE}"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "DisplayName" "${APP_NAME}"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "UninstallString" "$INSTDIR\uninstall.exe"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "DisplayIcon" "$INSTDIR\${MAIN_APP_EXE}"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "DisplayVersion" "${VERSION}"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "Publisher" "${COMP_NAME}"

!ifdef WEB_SITE
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "URLInfoAbout" "${WEB_SITE}"
!endif

# Register YUV file type
WriteRegStr "HKCR" "${APP_NAME}.yuv" "" "YUV Video File"
WriteRegStr "HKCR" "${APP_NAME}.yuv\DefaultIcon" "" "$INSTDIR\${MAIN_APP_EXE},1"
WriteRegStr "HKCR" "${APP_NAME}.yuv\shell\open\command" "" "$\"$INSTDIR\${MAIN_APP_EXE}$\" $\"%1$\""

# Register YTS file type
WriteRegStr "HKCR" "${APP_NAME}.yts" "" "YuvToolkit Script File"
WriteRegStr "HKCR" "${APP_NAME}.yts\DefaultIcon" "" "$INSTDIR\${MAIN_APP_EXE},2"
WriteRegStr "HKCR" "${APP_NAME}.yts\shell\open\command" "" "$\"$INSTDIR\${MAIN_APP_EXE}$\" $\"%1$\""

# Register RGB file type
WriteRegStr "HKCR" "${APP_NAME}.rgb" "" "RGB Video File"
WriteRegStr "HKCR" "${APP_NAME}.rgb\DefaultIcon" "" "$INSTDIR\${MAIN_APP_EXE},3"
WriteRegStr "HKCR" "${APP_NAME}.rgb\shell\open\command" "" "$\"$INSTDIR\${MAIN_APP_EXE}$\" $\"%1$\""

# Declare capabilities
WriteRegStr "HKLM" "SOFTWARE\RegisteredApplications" "${APP_NAME}" "SOFTWARE\${COMP_NAME}\${APP_NAME}\Capabilities"
WriteRegStr "HKLM" "SOFTWARE\${COMP_NAME}\${APP_NAME}" "" "${APP_NAME}"
WriteRegStr "HKLM" "SOFTWARE\${COMP_NAME}\${APP_NAME}\Capabilities" "ApplicationDescription" "Open-source and cross platform YUV player and analyzer for raw video"
WriteRegStr "HKLM" "SOFTWARE\${COMP_NAME}\${APP_NAME}\Capabilities" "ApplicationName" "${APP_NAME}"
WriteRegStr "HKLM" "SOFTWARE\${COMP_NAME}\${APP_NAME}\Capabilities\FileAssociations" ".yuv" "${APP_NAME}.yuv"
WriteRegStr "HKLM" "SOFTWARE\${COMP_NAME}\${APP_NAME}\Capabilities\FileAssociations" ".yts" "${APP_NAME}.yts"
WriteRegStr "HKLM" "SOFTWARE\${COMP_NAME}\${APP_NAME}\Capabilities\FileAssociations" ".rgb" "${APP_NAME}.rgb"

SectionEnd

######################################################################

Section Uninstall
${INSTALL_TYPE}
Delete "$INSTDIR\*.*"
!ifdef WEB_SITE
Delete "$INSTDIR\${APP_NAME} website.url"
!endif

RmDir "$INSTDIR"

!ifdef REG_START_MENU
!insertmacro MUI_STARTMENU_GETFOLDER "Application" $SM_Folder
Delete "$SMPROGRAMS\$SM_Folder\${APP_NAME}.lnk"
Delete "$SMPROGRAMS\$SM_Folder\Uninstall ${APP_NAME}.lnk"
!ifdef WEB_SITE
Delete "$SMPROGRAMS\$SM_Folder\${APP_NAME} Website.lnk"
!endif
RmDir "$SMPROGRAMS\$SM_Folder"
!endif

!ifndef REG_START_MENU
Delete "$SMPROGRAMS\YUVToolkit\${APP_NAME}.lnk"
Delete "$SMPROGRAMS\YUVToolkit\Uninstall ${APP_NAME}.lnk"
!ifdef WEB_SITE
Delete "$SMPROGRAMS\YUVToolkit\${APP_NAME} Website.lnk"
!endif
RmDir "$SMPROGRAMS\YUVToolkit"
!endif

DeleteRegKey ${REG_ROOT} "${REG_APP_PATH}"
DeleteRegKey ${REG_ROOT} "${UNINSTALL_PATH}"
DeleteRegKey "HKCR" "${APP_NAME}.yuv"
DeleteRegKey "HKCR" "${APP_NAME}.yts"
DeleteRegKey "HKCR" "${APP_NAME}.rgb"
DeleteRegValue "HKLM" "SOFTWARE\RegisteredApplications" "${APP_NAME}"
DeleteRegKey "HKLM" "SOFTWARE\${COMP_NAME}\${APP_NAME}"

SectionEnd

######################################################################

