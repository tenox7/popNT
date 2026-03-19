all: prince.exe

OBJ = main.obj data.obj seg000.obj seg001.obj seg002.obj seg003.obj seg004.obj seg005.obj seg006.obj seg007.obj seg008.obj seg009.obj seqtbl.obj replay.obj options.obj lighting.obj screenshot.obj menu.obj midi.obj opl3.obj stb_vorbis.obj compat.obj

CC = cl /nologo /c /Od /W0 /I. /I..\..\SDL\include

.c.obj:
	$(CC) $<

icon.res: icon.rc icon.ico
	rc icon.rc

prince.exe: $(OBJ) icon.res
	link /nologo /force:multiple /subsystem:windows /out:prince.exe $(OBJ) icon.res ..\..\SDL\SDL2_static.lib kernel32.lib user32.lib gdi32.lib winmm.lib shell32.lib advapi32.lib ole32.lib oleaut32.lib imm32.lib version.lib uuid.lib

clean:
	del *.obj
	del *.res
        del *.pdb
	del prince.exe

