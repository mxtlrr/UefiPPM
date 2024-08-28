all:
	make -C src/
	cp src/helloworld.efi ./
	bash makeiso.sh

qemu:	runqemu.sh
	bash $^

iso: makeiso.sh
	bash $^


clean:
	rm -rf src/*.efi src/*.o *.efi *.img *.iso