#include <gccore.h>
#include <string.h>

#include "ssl.hpp"

#define ISALIGNED(x) ((((u32)x)&0x1F)==0)

static char __ssl_fs[] ATTRIBUTE_ALIGN(32) = "/dev/net/ssl";
static s32 __ssl_fd = -1;
static s32 __ssl_hid = -1;

u32 SSL::Init() {
	if(__ssl_hid < 0 ) {
		__ssl_hid = iosCreateHeap(SSL_HEAP_SIZE);
		if(__ssl_hid < 0) return __ssl_hid;
	}
	return 0;
}

u32 SSL::Open() {
	s32 ret;
	if (__ssl_fd < 0) {
		ret = IOS_Open(__ssl_fs,0);
		if(ret<0) return ret;
		__ssl_fd = ret;
	}
	return 0;
}

u32 SSL::Close(void) {
	s32 ret;
	if(__ssl_fd < 0) return 0;
	ret = IOS_Close(__ssl_fd);
	__ssl_fd = -1;
	if(ret<0) return ret;
	return 0;
}

s32 SSL::New(u8 * CN, u32 ssl_verify_options) {
	s32 ret;
	s32 aContext[8] ATTRIBUTE_ALIGN(32);
	u32 aVerify_options[8] ATTRIBUTE_ALIGN(32);
	ret = SSL::Open();
	if(ret) return ret;
	aVerify_options[0] = ssl_verify_options;
	if(ISALIGNED(CN)) /* Avoid alignment if the input is aligned */
		ret = IOS_IoctlvFormat(__ssl_hid, __ssl_fd, IOCTLV_SSL_NEW, "d:dd", aContext, 0x20, aVerify_options, 0x20, CN, 0x100);
	else {
		u8 *aCN = NULL;
		aCN = (u8*)iosAlloc(__ssl_hid, 0x100);
		if (!aCN) return IPC_ENOMEM;
		memcpy(aCN, CN, 0x100);
		ret = IOS_IoctlvFormat(__ssl_hid, __ssl_fd, IOCTLV_SSL_NEW, "d:dd", aContext, 0x20, aVerify_options, 0x20, aCN, 0x100);
		if(aCN) iosFree(__ssl_hid, aCN);
	}
	SSL::Close();
	return (ret ? ret : aContext[0]);
}


s32 SSL::SetBuiltInClientCert(s32 ssl_context, s32 index) {
	s32 aSsl_context[8] ATTRIBUTE_ALIGN(32);
	s32 aIndex[8] ATTRIBUTE_ALIGN(32);
	s32 aResponse[8] ATTRIBUTE_ALIGN(32);
	s32 ret;
	ret = SSL::Open();
	if(ret) return ret;
	aSsl_context[0] = ssl_context;
	aIndex[0] = index;
	ret = IOS_IoctlvFormat(__ssl_hid, __ssl_fd, IOCTLV_SSL_SETBUILTINCLIENTCERT, "d:dd", aResponse, 32, aSsl_context, 32, aIndex, 32);
	SSL::Close();
	return (ret ? ret : aResponse[0]);
}


s32 SSL::SetRootCA(s32 ssl_context, const void *root, u32 length) {
	s32 aSsl_context[8] ATTRIBUTE_ALIGN(32);
	s32 aResponse[8] ATTRIBUTE_ALIGN(32);
	s32 ret;
	ret = SSL::Open();
	if(ret) return ret;
	aSsl_context[0] = ssl_context;
	if(ISALIGNED(root)) /* Avoid alignment if the input is aligned */
		ret = IOS_IoctlvFormat(__ssl_hid, __ssl_fd, IOCTLV_SSL_SETROOTCA, "d:dd", aResponse, 0x20, aSsl_context, 0x20, root, length);
	else {
		u8 *aRoot = NULL;
		aRoot = (u8*)iosAlloc(__ssl_hid, length);
		if (!aRoot) return IPC_ENOMEM;
		memcpy(aRoot, root, length);
		ret = IOS_IoctlvFormat(__ssl_hid, __ssl_fd, IOCTLV_SSL_SETROOTCA, "d:dd", aResponse, 0x20, aSsl_context, 0x20, aRoot, length);
		if(aRoot) iosFree(__ssl_hid, aRoot);
	}
	SSL::Close();
	return (ret ? ret : aResponse[0]);
}


s32 SSL::Connect(s32 ssl_context, s32 socket) {
	s32 aSsl_context[8] ATTRIBUTE_ALIGN(32);
	s32 aSocket[8] ATTRIBUTE_ALIGN(32);
	s32 aResponse[8] ATTRIBUTE_ALIGN(32);
	s32 ret;
	ret = SSL::Open();
	if(ret) return ret;
	aSsl_context[0] = ssl_context;
	aSocket[0] = socket;
	ret = IOS_IoctlvFormat(__ssl_hid, __ssl_fd, IOCTLV_SSL_CONNECT, "d:dd", aResponse, 0x20, aSsl_context, 0x20, aSocket, 0x20);
	SSL::Close();
	return (ret ? ret : aResponse[0]);
}


s32 SSL::Handshake(s32 ssl_context) {
	s32 aSsl_context[8] ATTRIBUTE_ALIGN(32);
	s32 aResponse[8] ATTRIBUTE_ALIGN(32);
	s32 ret;
	ret = SSL::Open();
	if(ret) return ret;
	aSsl_context[0] = ssl_context;
	ret = IOS_IoctlvFormat(__ssl_hid, __ssl_fd, IOCTLV_SSL_HANDSHAKE, "d:d", aResponse, 0x20, aSsl_context, 0x20);
	SSL::Close();
	return (ret ? ret : aResponse[0]);
}


s32 SSL::Read(s32 ssl_context, void* buffer, u32 length) {
	s32 aSsl_context[8] ATTRIBUTE_ALIGN(32);
	s32 aResponse[8] ATTRIBUTE_ALIGN(32);
	s32 ret;
	ret = SSL::Open();
	if(ret) return ret;
	if(!buffer) return IPC_EINVAL;
	u8 *aBuffer = NULL;
	aBuffer = (u8*)iosAlloc(__ssl_hid, length);
	if (!aBuffer) return IPC_ENOMEM;
	aSsl_context[0] = ssl_context;
	ret = IOS_IoctlvFormat(__ssl_hid, __ssl_fd, IOCTLV_SSL_READ, "dd:d", aResponse, 0x20, aBuffer, length, aSsl_context, 0x20);
	SSL::Close();
	if(ret == IPC_OK) memcpy(buffer, aBuffer, aResponse[0]);
	if(aBuffer) iosFree(__ssl_hid, aBuffer);
	return (ret ? ret : aResponse[0]);
}

s32 SSL::Write(s32 ssl_context, const void *buffer, u32 length) {
	s32 aSsl_context[8] ATTRIBUTE_ALIGN(32);
	s32 aResponse[8] ATTRIBUTE_ALIGN(32);
	s32 ret;
	ret = SSL::Open();
	if(ret) return ret;
	if(!buffer)return IPC_EINVAL;
	aSsl_context[0] = ssl_context;
	if(ISALIGNED(buffer)) //Avoid alignment if the input is aligned
		ret = IOS_IoctlvFormat(__ssl_hid, __ssl_fd, IOCTLV_SSL_WRITE, "d:dd", aResponse, 0x20, aSsl_context, 0x20, buffer, length);
	else {
		u8 *aBuffer = NULL;
		aBuffer = (u8*)iosAlloc(__ssl_hid, length);
		if (!aBuffer) return IPC_ENOMEM;
		memcpy(aBuffer, buffer, length);
		ret = IOS_IoctlvFormat(__ssl_hid, __ssl_fd, IOCTLV_SSL_WRITE, "d:dd", aResponse, 0x20, aSsl_context, 0x20, aBuffer, length);
	}
	SSL::Close();
	return (ret ? ret : aResponse[0]);
}

s32 SSL::Shutdown(s32 ssl_context) {
	s32 aSsl_context[8] ATTRIBUTE_ALIGN(32);
	s32 aResponse[8] ATTRIBUTE_ALIGN(32);
	s32 ret;
	ret = SSL::Open();
	if(ret) return ret;
	aSsl_context[0] = ssl_context;
	ret = IOS_IoctlvFormat(__ssl_hid, __ssl_fd, IOCTLV_SSL_SHUTDOWN, "d:d", aResponse, 0x20, aSsl_context, 0x20);
	SSL::Close();
	return (ret ? ret : aResponse[0]);
}