pipl: code/main.c
	gcc code/main.c -o pipl -I code/

test: pipl
	./pipl test.pipl -o out_pack
