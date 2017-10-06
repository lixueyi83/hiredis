#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <hiredis/hiredis.h>

int main(int argc, char **argv) 
{
    unsigned int i;
    redisContext *ctx;
    redisReply *reply;
    
    /*
    *   By default the redis-server will be: 127.0.0.1:6379.
    */
    const char *hostname = (argc > 1) ? argv[1] : "127.0.0.1";
    int port = (argc > 2) ? atoi(argv[2]) : 6379;

    /*
    *   Connect redis-server with timeout config
    */
    struct timeval timeout = { 1, 500000 }; /* 1.5 seconds */
    ctx = redisConnectWithTimeout(hostname, port, timeout);
    if(ctx == NULL || ctx->err) 
    {
        if(ctx) 
        {
            printf("Connection error: %s\n", ctx->errstr);
            redisFree(ctx);
        } 
        else 
        {
            printf("Connection error: can't allocate redis context\n");
        }
        exit(1);
    }

    /* 
    *   PING server
    */
    reply = redisCommand(ctx,"PING");
    printf("PING: %s\n", reply->str);
    freeReplyObject(reply);

    /* 
    *   Set a key 
    */
    reply = redisCommand(ctx,"SET %s %s", "foo", "hello world");
    printf("SET: %s\n", reply->str);
    freeReplyObject(reply);

    /* 
    *   Set a key using binary safe API 
    */
    reply = redisCommand(ctx,"SET %b %b", "bar", (size_t) 3, "hello", (size_t) 5);
    printf("SET (binary API): %s\n", reply->str);
    freeReplyObject(reply);

    /* 
    *   Try a GET and two INCR 
    */
    reply = redisCommand(ctx,"GET foo");
    printf("GET foo: %s\n", reply->str);
    freeReplyObject(reply);

    reply = redisCommand(ctx,"INCR counter");
    printf("INCR counter: %lld\n", reply->integer);
    freeReplyObject(reply);
    
    /* 
    *   again ... 
    */
    reply = redisCommand(ctx,"INCR counter");
    printf("INCR counter: %lld\n", reply->integer);
    freeReplyObject(reply);

    /* 
    *   Create a list of numbers, from 0 to 9 
    */
    reply = redisCommand(ctx,"DEL mylist");
    freeReplyObject(reply);
    for(i = 0; i < 10; i++) 
    {
        char buf[64];

        snprintf(buf,64,"%u",i);
        reply = redisCommand(ctx,"LPUSH mylist element-%s", buf);
        freeReplyObject(reply);
    }

    /* 
    *   Let's check what we have inside the list 
    */
    reply = redisCommand(ctx,"LRANGE mylist 0 -1");
    if(reply->type == REDIS_REPLY_ARRAY) 
    {
        for(i = 0; i < reply->elements; i++) 
        {
            printf("%u) %s\n", i, reply->element[i]->str);
        }
    }
    freeReplyObject(reply);

    /* 
    *   Disconnects and frees the context 
    */
    redisFree(ctx);

    return 0;
}
