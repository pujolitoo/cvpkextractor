all:
	gcc vpk.c -o vpk.exe

run:
	gcc vpk.c -o vpk.exe
	./vpk ./hl2_sound_misc_dir.vpk