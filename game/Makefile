# export ASAN_OPTIONS=symbolize=1:halt_on_error=0:detect_leaks=1:leak_check_at_exit=1:debug=1:atexit=1:print_stats=1:log_path=stdout:verify_asan_link_order=1

all:
	gcc -Wall -fsanitize=address -fsanitize=leak -fsanitize-recover=address -g 2048.c -o 2048