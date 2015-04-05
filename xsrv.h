// This file is part of the casycom project
//
// Copyright (c) 2015 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once
#include "xcom.h"
#ifdef __cplusplus
extern "C" {
#endif

enum EExternServerMethod {
    method_ExternServer_Open,
    method_ExternServer_Close,
    method_ExternServer_N
};
typedef void (*MFN_ExternServer_Open)(void* vo, int fd, const iid_t* exportedInterfaces);
typedef void (*MFN_ExternServer_Close)(void* vo);
typedef struct _DExternServer {
    iid_t interface;
    MFN_ExternServer_Open ExternServer_Open;
    MFN_ExternServer_Close ExternServer_Close;
} DExternServer;

void PExternServer_Open (const PProxy* pp, int fd, const iid_t* exportedInterfaces) noexcept NONNULL(1);
void PExternServer_Close (const PProxy* pp) noexcept NONNULL();
int  PExternServer_Bind (const PProxy* pp, const struct sockaddr* addr, socklen_t addrlen, const iid_t* exportedInterfaces) noexcept NONNULL();
int  PExternServer_BindLocal (const PProxy* pp, const char* path, const iid_t* exportedInterfaces) noexcept NONNULL();
int  PExternServer_BindUserLocal (const PProxy* pp, const char* sockname, const iid_t* exportedInterfaces) noexcept NONNULL();
int  PExternServer_BindSystemLocal (const PProxy* pp, const char* sockname, const iid_t* exportedInterfaces) noexcept NONNULL();

extern const SInterface i_ExternServer;
extern const SFactory f_ExternServer;

//{{{ PExternServer_Bind variants --------------------------------------

#ifdef __cplusplus
namespace {
#endif

/// Create local IPv4 socket at given ip and port
static inline int PExternServer_BindIP4 (const PProxy* pp, in_addr_t ip, in_port_t port, const iid_t* exportedInterfaces)
{
    struct sockaddr_in addr = {
	.sin_family = PF_INET,
	.sin_addr = { ip },
	.sin_port = port
    };
    return PExternServer_Bind (pp, (const struct sockaddr*) &addr, sizeof(addr), exportedInterfaces);
}

/// Create local IPv4 socket at given port on the loopback interface
static inline int PExternServer_BindLocalIP4 (const PProxy* pp, in_port_t port, const iid_t* exportedInterfaces)
    { return PExternServer_BindIP4 (pp, INADDR_LOOPBACK, port, exportedInterfaces); }

/// Create local IPv6 socket at given ip and port
static inline int PExternServer_BindIP6 (const PProxy* pp, struct in6_addr ip, in_port_t port, const iid_t* exportedInterfaces)
{
    struct sockaddr_in6 addr = {
	.sin6_family = PF_INET6,
	.sin6_addr = ip,
	.sin6_port = port
    };
    return PExternServer_Bind (pp, (const struct sockaddr*) &addr, sizeof(addr), exportedInterfaces);
}

/// Create local IPv6 socket at given ip and port
static inline int PExternServer_BindLocalIP6 (const PProxy* pp, in_port_t port, const iid_t* exportedInterfaces)
{
    struct sockaddr_in6 addr = {
	.sin6_family = PF_INET6,
	.sin6_addr = IN6ADDR_LOOPBACK_INIT,
	.sin6_port = port
    };
    return PExternServer_Bind (pp, (const struct sockaddr*) &addr, sizeof(addr), exportedInterfaces);
}

#ifdef __cplusplus
} // namespace
} // extern "C"
#endif
//}}}-------------------------------------------------------------------
