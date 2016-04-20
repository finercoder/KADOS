complete: kernel
	dd if=kernel of=floppya.img bs=512 conv=notrunc seek=3

run: complete
	bochs -f opsys.bxrc

kernel: kernel.o kernel_asm.o
	ld86 -o kernel -d kernel.o kernel_asm.o

kernel.o: kernel.c
	bcc -ansi -c -o kernel.o kernel.c

kernel_asm.o: kernel.asm
	as86 kernel.asm -o kernel_asm.o

clean:
	rm -f *.o kernel
