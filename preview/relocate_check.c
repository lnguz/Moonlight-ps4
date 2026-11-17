/* host_relocate(): finds the saved PC by <uniqueid> after an IP change.
 * A fake Sunshine/Apollo answers /serverinfo on 127.0.0.5:47989; the search
 * starts from 127.0.0.1 (the "old" address). Loopback /8 lets every
 * 127.0.0.x candidate be probed for real (closed ports refuse at once). */
#include "../src/gamestream/discovery.h"
#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void log_printf(const char *fmt, ...) { (void)fmt; }
void log_notify(const char *fmt, ...) { (void)fmt; }

static int s_listen = -1;
static void *fake_host(void *arg) {
    (void)arg;
    for (;;) {
        int c = accept(s_listen, NULL, NULL);
        if (c < 0)
            return NULL;
        char req[512];
        (void)recv(c, req, sizeof(req), 0);
        const char *body =
            "<?xml version=\"1.0\" encoding=\"utf-8\"?><root status_code=\"200\">"
            "<hostname>XOROINGUZ</hostname><appversion>7.1.431.-1</appversion>"
            "<uniqueid>AAAABBBB-CCCC-DDDD-EEEE-FFFF00001111</uniqueid></root>";
        char resp[1024];
        int n = snprintf(resp, sizeof(resp),
                         "HTTP/1.1 200 OK\r\nContent-Length: %zu\r\n\r\n%s",
                         strlen(body), body);
        (void)send(c, resp, (size_t)n, 0);
        close(c);
    }
}

int main(void) {
    s_listen = socket(AF_INET, SOCK_STREAM, 0);
    int one = 1;
    setsockopt(s_listen, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    struct sockaddr_in a;
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_port = htons(47989);
    inet_pton(AF_INET, "127.0.0.5", &a.sin_addr);
    if (bind(s_listen, (struct sockaddr *)&a, sizeof(a)) != 0 || listen(s_listen, 64) != 0) {
        puts("Relocate: skipped (cannot bind 127.0.0.5:47989)");
        return 0;
    }
    pthread_t th;
    pthread_create(&th, NULL, fake_host, NULL);
    pthread_detach(th);

    char ip[16];
    assert(host_relocate("127.0.0.1", "aaaabbbb-cccc-dddd-eeee-ffff00001111", "", ip, sizeof(ip), 4000) == 0);
    assert(!strcmp(ip, "127.0.0.5"));
    assert(host_relocate("127.0.0.1", "", "XOROINGUZ", ip, sizeof(ip), 4000) == 0 && !strcmp(ip, "127.0.0.5"));
    assert(host_relocate("127.0.0.1", "11111111-2222-3333-4444-555555555555", "", ip, sizeof(ip), 3000) != 0 && !ip[0]);
    assert(host_relocate("127.0.0.1", "", "", ip, sizeof(ip), 1000) != 0);
    assert(host_relocate("not-an-ip", "x", "", ip, sizeof(ip), 1000) != 0);
    usleep(300000); /* let the remaining probe threads finish under ASan */
    puts("Relocate: finds the PC by uniqueid or hostname, rejects other PCs passed");
    return 0;
}
