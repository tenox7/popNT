all: prince.exe

OBJ = main.obj data.obj seg000.obj seg001.obj seg002.obj seg003.obj seg004.obj seg005.obj seg006.obj seg007.obj seg008.obj seg009.obj seqtbl.obj replay.obj options.obj lighting.obj screenshot.obj menu.obj midi.obj opl3.obj stb_vorbis.obj compat.obj

CC = cl /nologo /c /Od /Zi /W0 /I. /I..\..\SDL\include

.c.obj:
	$(CC) $<

prince.exe: $(OBJ)
	link /nologo /debug /subsystem:windows /out:prince.exe $(OBJ) ..\..\SDL\SDL2.lib kernel32.lib user32.lib gdi32.lib winmm.lib shell32.lib advapi32.lib

clean:
	del *.obj
	del prince.exe

