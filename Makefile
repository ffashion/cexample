include scripts/top.mk
start: build

project-y += debug_print_stack/
# project-y += netlink/
project-y += network/
project-y += openssl/
project-y += other/
#很差劲 少一个/ 都不行
project-y += hyperscan/
