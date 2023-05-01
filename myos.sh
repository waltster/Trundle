#!/bin/bash
set -e

echo "MyOS Helper Script"

print_help () {
    echo "Options: "
    echo "  - clean:    Clean up files"
    echo "  - emul:     Emulate the OS"
    echo "  - help:     Display this message"
    echo "  - image:    Create bootable iso"
    echo "  - kernel:   Build kernel"
    echo "  - libc:     Build libc"
}

print_help_emulate () {
    echo "Emulation options: "
    echo "  - (q)emu"
    echo "  - (b)ochs"
}

export_general () {
    export PATH="/Users/walt/opt/cross/bin:$PATH"
    export MAKE=${MAKE:-make}
    export HOST=${HOST:-i686-elf}
    export HOST_ARCH=${HOST_ARCH:-i386}
    export AR=${HOST}-ar
    export AS=${HOST}-as
    export CC=${HOST}-gcc
    export SYSROOT="$(pwd)/sysroot"
    export DEST_DIR=$SYSROOT
}

export_kernel_build () {
    export_general
    (cd kernel && DEST_DIR="$SYSROOT" $MAKE install)
}

export_libc_build () {
    export_general
    (cd libc && DEST_DIR="$SYSROOT" $MAKE install)
}

emulate_qemu () {
    qemu-system-i386 -cdrom bootable.iso -d int -no-shutdown -no-reboot
}

emulate_bochs () {
    bochs -q -f img/bochsrc.txt
}

make_image () {
    export_libc_build
    export_kernel_build
    mkdir -p ./sysroot/boot/grub
    cp ./img/stage2_eltorito ./sysroot/boot/grub/stage2_eltorito
    cat << EOF > ./sysroot/boot/grub/menu.lst
default 0
title myos
kernel /usr/local/boot/myos.kernel
EOF
    sudo mkisofs -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table -o ./bootable.iso ./sysroot
}

if [ "$1" == "kernel" ]; then
    echo "kernel build mode activated"
    export_kernel_build 
elif [ "$1" == "libc" ]; then
    export_libc_build
elif [ "$1" == "emul" ]; then
    if [ "$2" == "q" ] || [ "$2" == "qemu" ]; then
        emulate_qemu
    elif [ "$2" == "b" ] || [ "$2" == "bochs" ]; then
        emulate_bochs
    else
        print_help_emulate
    fi
elif [ "$1" == "image" ]; then
    make_image
elif [ "$1" == "clean" ]; then
    export_general
    (cd libc && DEST_DIR="$SYSROOT" $MAKE clean)
    (cd kernel && DEST_DIR="$SYSROOT" $MAKE clean)
    rm -f bootable.iso
else
    print_help
fi
