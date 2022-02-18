#include <uv.h>

void timer_cb(uv_timer_t *handle) {
    printf("hello world\n");
}
int	main(int argc, char **argv) {
    uv_loop_t loop;
    uv_timer_t timer;
    uv_loop_init(&loop);
    uv_timer_init(&loop, &timer);

    uv_timer_start(&timer, timer_cb, 1000, 2000); //1s 后运行回调 然后每过2s 运行一次回调

    uv_run(&loop, UV_RUN_DEFAULT);
    return 0;
}
