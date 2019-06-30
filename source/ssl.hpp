#ifndef SSL_HPP
#define SSL_HPP

#define IOCTLV_SSL_NEW 1
#define IOCTLV_SSL_CONNECT 2
#define IOCTLV_SSL_HANDSHAKE 3
#define IOCTLV_SSL_READ 4
#define IOCTLV_SSL_WRITE 5
#define IOCTLV_SSL_SHUTDOWN 6
#define IOCTLV_SSL_SETROOTCA 10
#define IOCTLV_SSL_SETBUILTINCLIENTCERT 14

#define SSL_HEAP_SIZE 0xB000

class SSL {
public:
    static u32 Init();
    static u32 Open();
    static u32 Close();
    static s32 New(u8* CN, u32 verify_options);
    static s32 SetBuiltInClientCert(s32 ssl_context, s32 index);
    static s32 SetRootCA(s32 ssl_context, const void *root, u32 length);
    static s32 Connect(s32 ssl_context, s32 socket);
    static s32 Handshake(s32 ssl_context);
    static s32 Read(s32 ssl_context, void* buffer, u32 length);
    static s32 Write(s32 ssl_context, const void* buffer, u32 length);
    static s32 Shutdown(s32 ssl_context);
};

#endif