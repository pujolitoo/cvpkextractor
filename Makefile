all:
	gcc vpk.c -o vpk.exe -I/

run:
	gcc vpk.c -o vpk.exe
	./vpk ./hl2_sound_misc_dir.vpk