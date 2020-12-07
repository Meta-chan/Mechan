#General
COMPILER = cl
LINKER = link

#Ironic directory
IRONIC_DIRECTORY=E:\Project\_ir

#Tdlib directory
TDLIB_DIRECTORY=E:\Project\Mechan\td

#VCPKG directory
VCPKG_DIRECTORY=E:\Project\_lib\td\vcpkg

COMPILE_FLAGS = /GS /GL /W3 /Gy /Zc:wchar_t /I $(IRONIC_DIRECTORY) /I $(TDLIB_DIRECTORY)\include /Zi /Gm- /O2 /sdl /Zc:inline /fp:precise /D "_CRT_SECURE_NO_WARNINGS" /D "_MBCS" /errorReport:prompt /WX- /Zc:forScope /Gd /Oi /MD /FC /EHsc /nologo /diagnostics:classic /c

LINK_FLAGS = /MANIFEST /LTCG:incremental /NXCOMPAT /DYNAMICBASE "tdclient.lib" "tdcore.lib" "tdapi.lib" "tdnet.lib" $(VCPKG_DIRECTORY)\installed\x64-windows\lib\libssl.lib "tddb.lib" "tdactor.lib" "tdutils.lib" "Normaliz.lib" "psapi.lib" "shell32.lib" "tdsqlite.lib" $(VCPKG_DIRECTORY)\installed\x64-windows\lib\libcrypto.lib $(VCPKG_DIRECTORY)\installed\x64-windows\lib\zlib.lib "ws2_32.lib" "Mswsock.lib" "Crypt32.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "comdlg32.lib" "advapi32.lib" "odbc32.lib" "odbccp32.lib" /DEBUG:FULL /MACHINE:X64 /OPT:REF /SUBSYSTEM:CONSOLE /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /OPT:ICF /ERRORREPORT:PROMPT /NOLOGO /LIBPATH:$(TDLIB_DIRECTORY)\lib /TLBID:1 

{source}.cpp{tmp}.obj :
	if not exist tmp mkdir tmp
	$(COMPILER) $** $(COMPILE_FLAGS) /Fo:tmp\$(**B).obj

mechan.exe : tmp\mechan.obj tmp\mechan_console_interface.obj tmp\mechan_core.obj tmp\mechan_dialog.obj tmp\mechan_interface.obj tmp\mechan_ir_implement.obj tmp\mechan_log_interface.obj tmp\mechan_lowercase.obj tmp\mechan_morphology.obj tmp\mechan_neuro.obj tmp\mechan_parse.obj tmp\mechan_pipe_interface.obj tmp\mechan_synonym.obj tmp\mechan_telegram_interface.obj
	$(LINKER) $** $(LINK_FLAGS) /OUT:mechan.exe

all : mechan.exe

clean :
	if exist tmp rd /S /Q tmp
	del *.pdb
	del *.manifest
	del *.ipdb
	del *.iobj
	del *.exe