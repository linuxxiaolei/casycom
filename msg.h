// This file is part of the casycom project
//
// Copyright (c) 2015 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once
#include "stm.h"

//----------------------------------------------------------------------

typedef uint16_t	oid_t;

enum { oid_Broadcast };

typedef const char*	methodid_t;

typedef struct _SInterface {
    const char*	name;
    const void*	dispatch;
    const char*	method[];
} SInterface;

typedef const SInterface*	iid_t;

typedef struct _SMsg {
    iid_t	interface;
    void*	body;
    uint32_t	imethod;
    uint32_t	size;
    oid_t	src;
    oid_t	dest;
    oid_t	extid;
    uint8_t	fdoffset;
    uint8_t	reserved;
} SMsg;

enum { NoFdInMessage = UINT8_MAX };

typedef struct _PProxy {
    iid_t	interface;
    oid_t	src;
    oid_t	dest;
} PProxy;

#define MDCALL(name,...)	((MFN_##name)(dtable->name))(o, __VA_ARGS__, msg)

typedef void (*pfn_interface_dispatch)(const void* dtable, void* o, const SMsg* msg);

//----------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
namespace {
#endif

static inline RStm casymsg_read (const SMsg* msg)
{
    assert (msg && (msg->body || !msg->size));
    return (RStm) { (char*) msg->body, (char*) msg->body + msg->size };
}

#define casymsg_free(msg)	\
    do { if (msg) xfree (msg->body); xfree (msg); } while (false)

#ifdef __cplusplus
} // namespace
#endif

WStm	casymsg_begin (const PProxy* pp, uint32_t imethod, uint32_t sz) noexcept;
void	casymsg_end (const WStm* stm) noexcept;

uint32_t casyiface_count_methods (iid_t iid) noexcept;

#ifdef __cplusplus
} // extern "C"
#endif
