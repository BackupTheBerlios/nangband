# Microsoft Developer Studio Project File - Name="tolua" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=tolua - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "tolua.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "tolua.mak" CFG="tolua - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tolua - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "tolua - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "tolua - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /out:"src/tolua.exe"

!ELSEIF  "$(CFG)" == "tolua - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "tolua___Win32_Debug"
# PROP BASE Intermediate_Dir "tolua___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ  /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ  /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"src/tolua.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "tolua - Win32 Release"
# Name "tolua - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\lua\lapi.c
# End Source File
# Begin Source File

SOURCE=.\src\lua\lauxlib.c
# End Source File
# Begin Source File

SOURCE=.\src\lua\lbaselib.c
# End Source File
# Begin Source File

SOURCE=.\src\lua\lcode.c
# End Source File
# Begin Source File

SOURCE=.\src\lua\ldblib.c
# End Source File
# Begin Source File

SOURCE=.\src\lua\ldebug.c
# End Source File
# Begin Source File

SOURCE=.\src\lua\ldo.c
# End Source File
# Begin Source File

SOURCE=.\src\lua\lfunc.c
# End Source File
# Begin Source File

SOURCE=.\src\lua\lgc.c
# End Source File
# Begin Source File

SOURCE=.\src\lua\liolib.c
# End Source File
# Begin Source File

SOURCE=.\src\lua\llex.c
# End Source File
# Begin Source File

SOURCE=.\src\lua\lmem.c
# End Source File
# Begin Source File

SOURCE=.\src\lua\lobject.c
# End Source File
# Begin Source File

SOURCE=.\src\lua\lparser.c
# End Source File
# Begin Source File

SOURCE=.\src\lua\lstate.c
# End Source File
# Begin Source File

SOURCE=.\src\lua\lstring.c
# End Source File
# Begin Source File

SOURCE=.\src\lua\lstrlib.c
# End Source File
# Begin Source File

SOURCE=.\src\lua\ltable.c
# End Source File
# Begin Source File

SOURCE=.\src\lua\ltm.c
# End Source File
# Begin Source File

SOURCE=.\src\lua\lundump.c
# End Source File
# Begin Source File

SOURCE=.\src\lua\lvm.c
# End Source File
# Begin Source File

SOURCE=.\src\lua\lzio.c
# End Source File
# Begin Source File

SOURCE=.\src\lua\tolua.c
# End Source File
# Begin Source File

SOURCE=.\src\lua\tolua_bd.c
# End Source File
# Begin Source File

SOURCE=.\src\lua\tolua_eh.c
# End Source File
# Begin Source File

SOURCE=.\src\lua\tolua_gp.c
# End Source File
# Begin Source File

SOURCE=.\src\lua\tolua_lb.c
# End Source File
# Begin Source File

SOURCE=.\src\lua\tolua_rg.c
# End Source File
# Begin Source File

SOURCE=.\src\lua\tolua_tm.c
# End Source File
# Begin Source File

SOURCE=.\src\lua\tolua_tt.c
# End Source File
# Begin Source File

SOURCE=.\src\lua\tolualua.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\lua\lapi.h
# End Source File
# Begin Source File

SOURCE=.\src\lua\lauxlib.h
# End Source File
# Begin Source File

SOURCE=.\src\lua\lcode.h
# End Source File
# Begin Source File

SOURCE=.\src\lua\ldebug.h
# End Source File
# Begin Source File

SOURCE=.\src\lua\ldo.h
# End Source File
# Begin Source File

SOURCE=.\src\lua\lfunc.h
# End Source File
# Begin Source File

SOURCE=.\src\lua\lgc.h
# End Source File
# Begin Source File

SOURCE=.\src\lua\llex.h
# End Source File
# Begin Source File

SOURCE=.\src\lua\llimits.h
# End Source File
# Begin Source File

SOURCE=.\src\lua\lmem.h
# End Source File
# Begin Source File

SOURCE=.\src\lua\lobject.h
# End Source File
# Begin Source File

SOURCE=.\src\lua\lopcodes.h
# End Source File
# Begin Source File

SOURCE=.\src\lua\lparser.h
# End Source File
# Begin Source File

SOURCE=.\src\lua\lstate.h
# End Source File
# Begin Source File

SOURCE=.\src\lua\lstring.h
# End Source File
# Begin Source File

SOURCE=.\src\lua\ltable.h
# End Source File
# Begin Source File

SOURCE=.\src\lua\ltm.h
# End Source File
# Begin Source File

SOURCE=.\src\lua\lua.h
# End Source File
# Begin Source File

SOURCE=.\src\lua\luadebug.h
# End Source File
# Begin Source File

SOURCE=.\src\lua\lualib.h
# End Source File
# Begin Source File

SOURCE=.\src\lua\lundump.h
# End Source File
# Begin Source File

SOURCE=.\src\lua\lvm.h
# End Source File
# Begin Source File

SOURCE=.\src\lua\lzio.h
# End Source File
# Begin Source File

SOURCE=.\src\lua\tolua.h
# End Source File
# Begin Source File

SOURCE=.\src\lua\tolua_eh.h
# End Source File
# Begin Source File

SOURCE=.\src\lua\tolua_rg.h
# End Source File
# Begin Source File

SOURCE=.\src\lua\tolua_tm.h
# End Source File
# Begin Source File

SOURCE=.\src\lua\tolua_tt.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
