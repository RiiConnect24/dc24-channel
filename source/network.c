#include "network.h"
#include "network/ssl.h"

s32 doRequest(const void* hostname, const void* path, const u16 port, void* buffer, u32 length, const char* requestType) {
    s32 error = net_get_status();
    if (error < 0) {
        printf("net_get_status: %i\n", error);
        return error;
    }

    struct hostent* dns;
    dns = net_gethostbyname(hostname);
    if (dns->h_length <= 0) {
        printf("net_gethostbyname: couldn't get the address\n");
        return -1;
    }

    struct sockaddr_in sin;
    s32 sock = net_socket(AF_INET, SOCK_STREAM, 0);

    memcpy(&sin.sin_addr.s_addr, dns->h_addr, dns->h_length);
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);

    net_connect(sock, (struct sockaddr*)&sin, sizeof(sin));

    char response[length];
    memset(response, 0, length);
    char request[300] = "";
    sprintf(request, "%s %s HTTP/1.1\r\n\
User-Agent: WiiPatcher/1.0 (Nintendo Wii)\r\n\
Host: %s\r\n\
Content-Type: application/x-www-form-urlencoded\r\n\
Content-Length: %i\r\n\
Cache-Control: no-cache\r\n\r\n%s",
            requestType, (char*)path, (char*)hostname, strlen(buffer), (char*)buffer);

    if (port == 443) {
        error = ssl_init();
        if (error < 0) {
            printf("ssl_init: error %i", error);
            net_close(sock);
            return error;
        }

        s32 sslContext = ssl_new((u8*)hostname, 0);
        if (sslContext < 0) {
            printf("ssl_new: error %i\n", error);
            net_close(sock);
            return error;
        }

        error = ssl_setbuiltinclientcert(sslContext, 0);
        if (error < 0) {
            printf("ssl_setbuiltinclientcert: error %i\n", error);
            ssl_shutdown(sslContext);
            net_close(sock);
            return error;
        }

        error = ssl_connect(sslContext, sock);
        if (error < 0) {
            printf("ssl_connect: error %i\n", error);
            ssl_shutdown(sslContext);
            net_close(sock);
            return error;
        }

        error = ssl_handshake(sslContext);
        if (error < 0) {
            printf("ssl_handshake: error %i\n", error);
            ssl_shutdown(sslContext);
            net_close(sock);
            return error;
        }

        error = ssl_write(sslContext, request, sizeof(request));
        if (error < 0) {
            printf("ssl_write: error %i\n", error);
            ssl_shutdown(sslContext);
            net_close(sock);
            return error;
        }

        error = ssl_read(sslContext, response, length);
        if (error < 0) {
            printf("ssl_read: error %i\n", error);
            ssl_shutdown(sslContext);
            net_close(sock);
            return error;
        }

        ssl_shutdown(sslContext);
    } else {
        error = net_send(sock, request, sizeof(request), 0);
        if (error < 0) {
            printf("net_send: error %i\n", error);
            net_close(sock);
            return error;
        }

        error = net_recv(sock, response, length, 0);
        if (error < 0) {
            printf("net_recv: error %i\n", error);
            net_close(sock);
            return error;
        }
    }

    int minor_version;
    int status;
    const char* msg;
    size_t msg_len;
    struct phr_header headers[20];
    size_t num_headers;

    num_headers = sizeof(headers) / sizeof(headers[0]);
    int ret = phr_parse_response(response, strlen(response), &minor_version, &status, &msg,
                                 &msg_len, headers, &num_headers, 0);
    if (ret < 0) {
        printf("phr_parse_response: error %i\n", ret);
        net_close(sock);
        return error;
    }

    if (status != 200) {
        printf("The request wasn't successful: %i\n", status);
        return 0 - status;
    }

    if (strcmp(requestType, "GET") == 0) {
        const char* body = &msg[ret];
        memcpy(buffer, body, length);
    } else if (strcmp(requestType, "POST") == 0) {
        memcpy(buffer, response + ret, length);
    }

    net_close(sock);

    return 0;
}

s32 getRequest(const void* hostname, const void* path, const u16 port, void* buffer, u32 length) {
    return doRequest(hostname, path, port, buffer, length, "GET");
}

s32 postRequest(const void* hostname, const void* path, const u16 port, void* buffer, u32 length) {
    return doRequest(hostname, path, port, buffer, length, "POST");
}

bool initNetwork() {
    bool ok = false;

    for (int i = 0; i < 50 && !ok; i++)
        if (net_init() >= 0) ok = true;

    return ok;
}