
#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>
#include <unistd.h>
#include <signal.h>
#include <event.h>

void connect_cb(const struct redisAsyncContext* ctx, int status) {
    if (status == REDIS_OK) {
        printf("connect to %s:%d ok\n", ctx->c.tcp.host, ctx->c.tcp.port);
    }
    printf("%s\n",ctx->errstr);
}

void disconnect_cb(const struct redisAsyncContext* ctx, int status) {
    if (status == REDIS_OK) {
        printf("disconnect form %s:%d ok\n", ctx->c.tcp.host, ctx->c.tcp.port);
    }
    printf("%s\n",ctx->errstr);
}

void command_cb(redisAsyncContext *ctx, void *r, void *privdata) {
    redisReply *reply = r;
    if (reply == NULL) {
        return;
    }
    switch (reply->type) {
    case REDIS_REPLY_STRING:
        printf("%s\n",reply->str);
        break;
    case REDIS_REPLY_INTEGER:
        printf("%lld", reply->integer);
        break;
    }
    
}

int	main(int argc, char **argv) {
    
    redisAsyncContext *ctx;
    int rc; 
    signal(SIGPIPE, SIG_IGN);
    struct event_base *base = event_base_new();
    if (base == NULL) {
        printf("event_base_new err\n");
    }

    ctx = redisAsyncConnect("127.0.0.1", 6379);
    if (ctx == NULL) {
        printf("error\n");
    }
    redisLibeventAttach(ctx, base);
    rc = redisAsyncSetConnectCallback(ctx, connect_cb);
    rc = redisAsyncSetDisconnectCallback(ctx, disconnect_cb);

    rc = redisAsyncCommand(ctx, command_cb, NULL, "GET foo");
    
    if (rc == REDIS_ERR) {
        printf("AsyncCommand error\n");
        exit(EXIT_FAILURE);
    }

    event_base_dispatch(base);
    return 0;
}
