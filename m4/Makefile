complete: floppya

run: complete
	bochs -f opsys.bxrc

kernel: kernel.o kernel_asm.o
	ld86 -o kernel -d kernel.o kernel_asm.o

kernel.o: kernel.c
	bcc -ansi -c -o kernel.o kernel.c

loadFile: loadFile.c
	gcc loadFile.c -o loadFile

kernel_asm.o: kernel.asm
	as86 kernel.asm -o kernel_asm.o

bootload: bootload.asm
	nasm bootload.asm

shell.o: shell.c
	bcc -ansi -c -o shell.o shell.c

lib.o: lib.asm
	as86 lib.asm -o lib.o

shell: lib.o shell.o
	ld86 -o shell -d shell.o lib.o

floppya: kernel message.txt bootload loadFile shell
	dd if=/dev/zero of=floppya.img bs=512 count=2880
	dd if=bootload of=floppya.img bs=512 count=1 conv=notrunc
	dd if=map.img of=floppya.img bs=512 count=1 seek=1 conv=notrunc
	dd if=dir.img of=floppya.img bs=512 count=1 seek=2 conv=notrunc
	dd if=kernel of=floppya.img bs=512 conv=notrunc seek=3
	dd if=message.txt of=floppya.img bs=512 count=1 seek=30 conv=notrunc
	./loadFile message.txt
	./loadFile tstprg
	./loadFile tstpr2
	./loadFile shell

clean:
	rm -f *.o kernel shell loadFile bootload
