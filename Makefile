pipl: code/main.c code/debug_helpers.c
	gcc code/main.c -o pipl -I code/

test: pipl
	./pipl test.pipl -o out_pack
