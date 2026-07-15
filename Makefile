bin/pipl: code/pipl/main.c code/pipl/debug_helpers.c bin/model
	mkdir -p bin
	gcc code/pipl/main.c -o bin/pipl -I code/

test: bin/pipl
	cp test.pipl bin/test.pipl
	cp ship.blend bin/ship.blend
	(cd bin && ./pipl test.pipl -o out_pack)

clean:
	rm -r bin

bin/model: code/model/main.c
	mkdir -p bin
	xxd -i code/model/blender_export.py > code/model/blender_export.c
	gcc code/model/main.c -o bin/model -I code/ -lm
