all:
	clang -g -O2 -target bpf -D__TARGET_ARCH_x86 -c uprobe.bpf.c -o uprobe.tmp.bpf.o
	bpftool gen object uprobe.bpf.o uprobe.tmp.bpf.o
	bpftool gen skeleton uprobe.bpf.o > uprobe.skel.h
	clang -g -Wall -I./ -c uprobe.c -o uprobe.o
	clang -g -Wall uprobe.o -lbpf -o uprobe

clean:
	rm -rf *.o uprobe *.skel.h