/*
 * Copyright (C) 2013 Kay Sievers
 * Copyright (C) 2013 Greg Kroah-Hartman <gregkh@linuxfoundation.org>
 * Copyright (C) 2013 Linux Foundation
 * Copyright (C) 2013 Lennart Poettering
 * Copyright (C) 2013 Daniel Mack <daniel@zonque.org>
 *
 * kdbus is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at
 * your option) any later version.
 */

#ifndef _KDBUS_H_
#define _KDBUS_H_

#ifndef __KERNEL__
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/types.h>
#endif

#define KDBUS_IOC_MAGIC			0x95
#define KDBUS_SRC_ID_KERNEL		(0)
#define KDBUS_DST_ID_NAME		(0)
#define KDBUS_MATCH_SRC_ID_ANY		(~0ULL)
#define KDBUS_DST_ID_BROADCAST		(~0ULL)

/**
 * struct kdbus_notify_name_change - name registry change message
 * @old_id:		Former owner of a name
 * @new_id:		New owner of a name
 * @flags:		flags from KDBUS_NAME_*
 * @name:		Well-known name
 *
 * Sent from kernel to userspace when the owner or starter of
 * a well-known name changes.
 *
 * Attached to:
 *   KDBUS_ITEM_NAME_ADD
 *   KDBUS_ITEM_NAME_REMOVE
 *   KDBUS_ITEM_NAME_CHANGE
 */
struct kdbus_notify_name_change {
	__u64 old_id;
	__u64 new_id;
	__u64 flags;
	char name[0];
};

/**
 * struct kdbus_notify_id_change - name registry change message
 * @id:			New or former owner of the name
 * @flags:		flags field from KDBUS_HELLO_*
 *
 * Sent from kernel to userspace when the owner or starter of
 * a well-known name changes.
 *
 * Attached to:
 *   KDBUS_ITEM_ID_ADD
 *   KDBUS_ITEM_ID_REMOVE
 */
struct kdbus_notify_id_change {
	__u64 id;
	__u64 flags;
};

/**
 * struct kdbus_creds - process credentials
 * @uid:		User ID
 * @gid:		Group ID
 * @pid:		Process ID
 * @tid:		Thread ID
 * @starttime:		Starttime of the process
 *
 * The starttime of the process PID. This is useful to detect PID overruns
 * from the client side. i.e. if you use the PID to look something up in
 * /proc/$PID/ you can afterwards check the starttime field of it, to ensure
 * you didn't run into a PID overrun.
 *
 * Attached to:
 *   KDBUS_ITEM_CREDS
 */
struct kdbus_creds {
	__u64 uid;
	__u64 gid;
	__u64 pid;
	__u64 tid;
	__u64 starttime;
};

/**
 * struct kdbus_audit - audit information
 * @sessionid:		The audit session ID
 * @loginuid:		The audit login uid
 *
 * Attached to:
 *   KDBUS_ITEM_AUDIT
 */
struct kdbus_audit {
	__u64 sessionid;
	__u64 loginuid;
};

/**
 * struct kdbus_timestamp
 * @monotonic_ns:	Monotonic timestamp, in nanoseconds
 * @realtime_ns:	Realtime timestamp, in nanoseconds
 *
 * Attached to:
 *   KDBUS_ITEM_TIMESTAMP
 */
struct kdbus_timestamp {
	__u64 monotonic_ns;
	__u64 realtime_ns;
};

/**
 * struct kdbus_vec - I/O vector for kdbus payload items
 * @size:		The size of the vector
 * @address:		Memory address for memory addresses
 * @offset:		Offset in the in-message payload memory
 *
 * Attached to:
 *   KDBUS_ITEM_PAYLOAD_VEC
 */
struct kdbus_vec {
	__u64 size;
	union {
		__u64 address;
		__u64 offset;
	};
};

/**
 * struct kdbus_memfd - a kdbus memfd
 * @size:		The memfd's size
 * @fd:			The file descriptor number
 * @__pad:		Padding to make the struct aligned
 *
 * Attached to:
 *   KDBUS_ITEM_PAYLOAD_MEMFD
 */
struct kdbus_memfd {
	__u64 size;
	int fd;
	__u32 __pad;
};

/**
 * struct kdbus_name - a registered well-known name with its flags
 * @flags:		flags from KDBUS_NAME_*
 * @name:		well-known name
 *
 * Attached to:
 *   KDBUS_ITEM_NAME
 */
struct kdbus_name {
	__u64 flags;
	char name[0];
};

/**
 * struct kdbus_policy_access - policy access item
 * @type:		One of KDBUS_POLICY_ACCESS_* types
 * @bits:		Access to grant. One of KDBUS_POLICY_*
 * @id:			For KDBUS_POLICY_ACCESS_USER, the uid
 * 			For KDBUS_POLICY_ACCESS_GROUP, the gid
 *
 * Embedded in:
 *   struct kdbus_policy
 */
struct kdbus_policy_access {
	__u64 type;	/* USER, GROUP, WORLD */
	__u64 bits;	/* RECV, SEND, OWN */
	__u64 id;	/* uid, gid, 0 */
};

/**
 * struct kdbus_policy - a policy item
 * @access:		Policy access details
 * @name:		Well-known name to grant access to
 *
 * Attached to:
 *   KDBUS_POLICY_ACCESS
 *   KDBUS_ITEM_POLICY_NAME
 */
struct kdbus_policy {
	union {
		struct kdbus_policy_access access;
		char name[0];
	};
};

/* item types to chain data in lists of items */
enum {
	_KDBUS_ITEM_NULL,
	_KDBUS_ITEM_USER_BASE,
	KDBUS_ITEM_PAYLOAD_VEC	= _KDBUS_ITEM_USER_BASE, /* .data_vec */
	KDBUS_ITEM_PAYLOAD_OFF,		/* data at returned offset in the pool */
	KDBUS_ITEM_PAYLOAD_MEMFD,	/* data as sealed memfd */
	KDBUS_ITEM_FDS,			/* attached file descriptors */
	KDBUS_ITEM_BLOOM,		/* for broadcasts, carries bloom filter */
	KDBUS_ITEM_DST_NAME,		/* destination's well-known name */
	KDBUS_ITEM_PRIORITY,		/* queue priority for message */

	_KDBUS_ITEM_POLICY_BASE	= 0x400,
	KDBUS_ITEM_POLICY_NAME = _KDBUS_ITEM_POLICY_BASE,
	KDBUS_ITEM_POLICY_ACCESS,

	_KDBUS_ITEM_ATTACH_BASE	= 0x600,
	KDBUS_ITEM_NAME		= _KDBUS_ITEM_ATTACH_BASE,
	KDBUS_ITEM_STARTER_NAME,
	KDBUS_ITEM_TIMESTAMP,
	KDBUS_ITEM_CREDS,
	KDBUS_ITEM_PID_COMM,
	KDBUS_ITEM_TID_COMM,
	KDBUS_ITEM_EXE,
	KDBUS_ITEM_CMDLINE,
	KDBUS_ITEM_CGROUP,
	KDBUS_ITEM_CAPS,
	KDBUS_ITEM_SECLABEL,
	KDBUS_ITEM_AUDIT,

	/* Special messages from kernel */
	_KDBUS_ITEM_KERNEL_BASE	= 0x800,
	KDBUS_ITEM_NAME_ADD	= _KDBUS_ITEM_KERNEL_BASE,
	KDBUS_ITEM_NAME_REMOVE,		/* .name_change */
	KDBUS_ITEM_NAME_CHANGE,		/* .name_change */
	KDBUS_ITEM_ID_ADD,		/* .id_change */
	KDBUS_ITEM_ID_REMOVE,		/* .id_change */
	KDBUS_ITEM_REPLY_TIMEOUT,	/* empty, reply_cookie is filled in */
	KDBUS_ITEM_REPLY_DEAD,		/* dito */
};

/**
 * struct kdbus_item - chain of data blocks
 * @size:	:	Overall data record size
 * @type:		Kdbus_item type of data
 * @data:		Generic bytes
 * @data32:		Generic 32 bit array
 * @data64:		Generic 64 bit array
 * @str:		Generic string
 * @id:			Connection ID
 * @vec:		KDBUS_ITEM_PAYLOAD_VEC
 * @creds:		KDBUS_ITEM_CREDS
 * @audit:		KDBUS_ITEM_AUDIT
 * @timestamp:		KDBUS_ITEM_TIMESTAMP
 * @name:		KDBUS_ITEM_NAME
 * @memfd:		KDBUS_ITEM_PAYLOAD_MEMFD
 * @name_change:	KDBUS_ITEM_NAME_ADD
 * 			KDBUS_ITEM_NAME_REMOVE
 * 			KDBUS_ITEM_NAME_CHANGE
 * @id_change:		KDBUS_ITEM_ID_ADD
 * 			KDBUS_ITEM_ID_REMOVE
 * @policy:		KDBUS_ITEM_POLICY_NAME
 * 			KDBUS_ITEM_POLICY_ACCESS
 */
struct kdbus_item {
	__u64 size;
	__u64 type;
	union {
		__u8 data[0];
		__u32 data32[0];
		__u64 data64[0];
		char str[0];

		__u64 id;
		struct kdbus_vec vec;
		struct kdbus_creds creds;
		struct kdbus_audit audit;
		struct kdbus_timestamp timestamp;
		struct kdbus_name name;
		struct kdbus_memfd memfd;
		int fds[0];
		struct kdbus_notify_name_change name_change;
		struct kdbus_notify_id_change id_change;
		struct kdbus_policy policy;
	};
};

enum {
	KDBUS_MSG_FLAGS_EXPECT_REPLY	= 1 << 0,
	KDBUS_MSG_FLAGS_NO_AUTO_START	= 1 << 1,
};

enum {
	KDBUS_PAYLOAD_KERNEL,
	KDBUS_PAYLOAD_DBUS1	= 0x4442757356657231ULL, /* 'DBusVer1' */
	KDBUS_PAYLOAD_GVARIANT	= 0x4756617269616e74ULL, /* 'GVariant' */
};

/**
 * struct kdbus_msg - the representation of a kdbus message
 * @size:		Total size of the message
 * @flags:		Message flags (KDBUS_MSG_FLAGS_*)
 * @dst_id:		64-bit ID of the destination connection
 * @src_id:		64-bit ID of the source connection
 * @payload_type:	Payload type (KDBUS_PAYLOAD_*)
 * @cookie:		Userspace-supplied cookie
 * @cookie_reply:	For kernel-generated messages, this is the cookie
 * 			the message is a reply to
 * @timeout_ns:		For non-kernel-generated messages, this denotes the
 * 			message timeout in nanoseconds
 * @items:		A list of kdbus_items containing the message payload
 */
struct kdbus_msg {
	__u64 size;
	__u64 flags;
	__u64 dst_id;
	__u64 src_id;
	__u64 payload_type;
	__u64 cookie;
	union {
		__u64 cookie_reply;
		__u64 timeout_ns;
	};
	struct kdbus_item items[0];
};

enum {
	_KDBUS_POLICY_ACCESS_NULL,
	KDBUS_POLICY_ACCESS_USER,
	KDBUS_POLICY_ACCESS_GROUP,
	KDBUS_POLICY_ACCESS_WORLD,
};

enum {
	KDBUS_POLICY_RECV		= 1 <<  2,
	KDBUS_POLICY_SEND		= 1 <<  1,
	KDBUS_POLICY_OWN		= 1 <<  0,
};

/**
 * struct kdbus_cmd_policy - a series of policies to upload
 * @size:		The total size of the structure
 * @policies:		The policies to upload
 *
 * A KDBUS_POLICY_NAME must always preceed a KDBUS_POLICY_ACCESS entry.
 * A new KDBUS_POLICY_NAME can be added after KDBUS_POLICY_ACCESS for
 * chaining multiple policies together.
 */
struct kdbus_cmd_policy {
	__u64 size;
	struct kdbus_item policies[0];
};

/* Flags for struct kdbus_cmd_hello */
enum {
	KDBUS_HELLO_STARTER		=  1 <<  0,
	KDBUS_HELLO_ACCEPT_FD		=  1 <<  1,
};

/* Flags for message attachments */
enum {
	KDBUS_ATTACH_TIMESTAMP		=  1 <<  0,
	KDBUS_ATTACH_CREDS		=  1 <<  1,
	KDBUS_ATTACH_NAMES		=  1 <<  2,
	KDBUS_ATTACH_COMM		=  1 <<  3,
	KDBUS_ATTACH_EXE		=  1 <<  4,
	KDBUS_ATTACH_CMDLINE		=  1 <<  5,
	KDBUS_ATTACH_CGROUP		=  1 <<  6,
	KDBUS_ATTACH_CAPS		=  1 <<  7,
	KDBUS_ATTACH_SECLABEL		=  1 <<  8,
	KDBUS_ATTACH_AUDIT		=  1 <<  9,
};

/**
 * struct kdbus_cmd_hello - struct to say hello to kdbus
 * @size:		The total size of the structure
 * @conn_flags:		Connection flags (KDBUS_HELLO_*). The kernel will
 * 			return its capabilities in that field.
 * @attach_flags:	Mask of metdata to attach to each message sent
 * 			(KDBUS_ATTACH_*)
 * @bus_flags:		The flags field copied verbatim from the original
 * 			KDBUS_CMD_BUS_MAKE ioctl. It's intended to be useful
 *			to do negotiation of features of the payload that is
 *			transferred (kernel → userspace)
 * @id:			The ID of this connection (kernel → userspace)
 * @bloom_size:		The bloom filter size chosen by the owner
 * 			(kernel → userspace)
 * @pool_size:		Maximum size of the pool buffer (kernel → userspace)
 * @id128:		Unique 128-bit ID of the bus (kernel → userspace)
 * @items:		A list of items
 *
 * This struct is used with the KDBUS_CMD_HELLO ioctl. See the ioctl
 * documentation for more information.
 */
struct kdbus_cmd_hello {
	__u64 size;

	/* userspace → kernel, kernel → userspace */
	__u64 conn_flags;
	__u64 attach_flags;

	/* kernel → userspace */
	__u64 bus_flags;
	__u64 id;
	__u64 bloom_size;
	__u64 pool_size;
	__u8 id128[16];

	struct kdbus_item items[0];
};

/* Flags for KDBUS_CMD_{BUS,EP,NS}_MAKE */
enum {
	KDBUS_MAKE_ACCESS_GROUP		= 1 <<  0,
	KDBUS_MAKE_ACCESS_WORLD		= 1 <<  1,
	KDBUS_MAKE_POLICY_OPEN		= 1 <<  2,
};

/* Items to append to KDBUS_CMD_{BUS,EP,NS}_MAKE */
enum {
	_KDBUS_MAKE_NULL,
	KDBUS_MAKE_NAME,
};

/**
 * struct kdbus_cmd_bus_make - struct to make a bus
 * @size:		The total size of the struct
 * @flags:		Properties for the bus to create
 * @bloom_size:		Size of the bloom filter for this bus
 * @items:		Items describing details such as the name of the bus
 *
 * This structure is used with the KDBUS_CMD_BUS_MAKE ioctl. Refer to the
 * documentation for more information.
 */
struct kdbus_cmd_bus_make {
	__u64 size;
	__u64 flags;
	__u64 bloom_size;
	struct kdbus_item items[0];
};

/**
 * struct kdbus_cmd_ep_make - struct to make an endpoint
 * @size:		The total size of the struct
 * @flags:		Unused for now
 * @items:		Items describing details such as the
 * 			name of the endpoint
 *
 * This structure is used with the KDBUS_CMD_EP_MAKE ioctl. Refer to the
 * documentation for more information.
 */
struct kdbus_cmd_ep_make {
	__u64 size;
	__u64 flags;
	struct kdbus_item items[0];
};

/**
 * struct kdbus_cmd_ns_make - struct to make a namespace
 * @size:		The total size of the struct
 * @flags:		Unused for now
 * @items:		Items describing details such as the
 * 			name of the namespace
 *
 * This structure is used with the KDBUS_CMD_NS_MAKE ioctl. Refer to the
 * documentation for more information.
 */
struct kdbus_cmd_ns_make {
	__u64 size;
	__u64 flags;
	struct kdbus_item items[0];
};

enum {
	/* userspace → kernel */
	KDBUS_NAME_REPLACE_EXISTING		= 1 <<  0,
	KDBUS_NAME_QUEUE			= 1 <<  1,
	KDBUS_NAME_ALLOW_REPLACEMENT		= 1 <<  2,

	/* kernel → userspace */
	KDBUS_NAME_IN_QUEUE			= 1 << 16,
};

/**
 * struct kdbus_cmd_name - struct to describe a well-known name
 * @size:		The total size of the struct
 * @flags:		Flags for a name entry (KDBUS_NAME_*)
 * @id:			Priviledged users may use this field to (de)register
 * 			names on behalf of other peers.
 * @conn_flags:		The flags of the owning connection (KDBUS_HELLO_*)
 * @name:		The well-known name
 *
 * This structure is used with the KDBUS_CMD_NAME_ACQUIRE ioctl.
 * Refer to the documentation for more information.
 */
struct kdbus_cmd_name {
	__u64 size;
	__u64 flags;
	__u64 id;
	__u64 conn_flags;
	char name[0];
};

enum {
	KDBUS_NAME_LIST_UNIQUE		= 1 <<  0,
	KDBUS_NAME_LIST_NAMES		= 1 <<  1,
	KDBUS_NAME_LIST_STARTERS	= 1 <<  2,
	KDBUS_NAME_LIST_QUEUED		= 1 <<  3,
};

/**
 * struct kdbus_cmd_name_list - request a list of name entries
 * @flags:		Flags for the query (KDBUS_NAME_LIST_*)
 * @offset:		The returned offset in the caller's pool buffer.
 *			The user must use KDBUS_CMD_FREE to free the
 *			allocated memory.
 * 
 * This structure is used with the KDBUS_CMD_NAME_LIST ioctl.
 */
struct kdbus_cmd_name_list {
	__u64 flags;
	__u64 offset;
};

/**
 * struct kdbus_name_list - information returned by KDBUS_CMD_NAME_LIST
 * @size:		The total size of the structure
 * @names:		A list of names
 *
 * Note that the user is responsible for freeing the allocated memory with
 * the KDBUS_CMD_FREE ioctl.
 */
struct kdbus_name_list {
	__u64 size;
	struct kdbus_cmd_name names[0];
};

/**
 * struct kdbus_cmd_conn_info - struct used for KDBUS_CMD_CONN_INFO ioctl
 * @size:		The total size of the struct
 * @flags:		KDBUS_ATTACH_* flags
 * @id:			The 64-bit ID of the connection. If set to zero, passing
 * 			@name is required. kdbus will look up the name to determine
 * 			the ID in this case.
 * @offset:		Returned offset in the caller's pool buffer where the
 * 			kdbus_name_info struct result is stored. The user must
 * 			use KDBUS_CMD_FREE to free the allocated memory.
 * @name:		The optional well-known name to look up. Only needed in
 * 			case @id is zero.
 *
 * On success, the KDBUS_CMD_CONN_INFO ioctl will return 0 and @offset will
 * tell the user the offset in the connection pool buffer at which to find the
 * result in a struct kdbus_conn_info.
 */
struct kdbus_cmd_conn_info {
	__u64 size;
	__u64 flags;
	__u64 id;
	__u64 offset;
	char name[0];
};

/**
 * struct kdbus_conn_info - information returned by KDBUS_CMD_CONN_INFO
 * @size:		The total size of the struct
 * @id:			The connection's 64-bit ID
 * @flags:		The connection's flags
 * @items:		A list of struct kdbus_item
 *
 * Note that the user is responsible for freeing the allocated memory with
 * the KDBUS_CMD_FREE ioctl.
 */
struct kdbus_conn_info {
	__u64 size;
	__u64 id;
	__u64 flags;
	struct kdbus_item items[0];
};

enum {
	_KDBUS_MATCH_NULL,
	KDBUS_MATCH_BLOOM,		/* Matches a mask blob against KDBUS_MSG_BLOOM */
	KDBUS_MATCH_SRC_NAME,		/* Matches a name string against KDBUS_ITEM_NAME */
	KDBUS_MATCH_NAME_ADD,		/* Matches a name string against KDBUS_ITEM_NAME_ADD */
	KDBUS_MATCH_NAME_REMOVE,	/* Matches a name string against KDBUS_ITEM_NAME_REMOVE */
	KDBUS_MATCH_NAME_CHANGE,	/* Matches a name string against KDBUS_ITEM_NAME_CHANGE */
	KDBUS_MATCH_ID_ADD,		/* Matches an ID against KDBUS_MSG_ID_ADD */
	KDBUS_MATCH_ID_REMOVE,		/* Matches an ID against KDBUS_MSG_ID_REMOVE */
};

/**
 * struct kdbus_cmd_match - struct to add or remove matches
 * @size:		The total size of the struct
 * @id:			Priviledged users may (de)register matches on behalf
 * 			of other peers.
 * 			In other cases, set to 0.
 * @cookie:		Userspace supplied cookie. When removing, the cookie is
 * 			suffices as information
 * @src_id:		The source ID to match against. Use KDBUS_MATCH_SRC_ID_ANY or
 * 			any other value for a unique match.
 * @items:		A list of items for additional information
 *
 * This structure is used with the KDBUS_CMD_ADD_MATCH and
 * KDBUS_CMD_REMOVE_MATCH ioctl. Refer to the documentation for more
 * information.
 */
struct kdbus_cmd_match {
	__u64 size;
	__u64 id;
	__u64 cookie;
	__u64 src_id;
	struct kdbus_item items[0];
};

enum {
	KDBUS_MONITOR_ENABLE		= 1 <<  0,
};

/**
 * struct kdbus_cmd_monitor - struct to enable or disable eavesdropping
 * @id:			Priviledged users may enable or disable the monitor feature
 * 			on behalf of other peers
 * @flags:		Use KDBUS_MONITOR_ENABLE to enable eavesdropping
 *
 * This structure is used with the KDBUS_CMD_MONITOR ioctl.
 * Refer to the documentation for more information.
 */
struct kdbus_cmd_monitor {
	__u64 id;
	__u64 flags;
};

/**
 * enum kdbus_ioctl_type - Ioctl API
 * @KDBUS_CMD_BUS_MAKE:		After opening the "control" device node, this
 * 				command creates a new bus with the specified
 * 				name. The bus is immediately shut down and
 * 				cleaned up when the opened "control" device node
 * 				is closed.
 * @KDBUS_CMD_NS_MAKE:		Similar to KDBUS_CMD_BUS_MAKE, but it creates a
 * 				new kdbus namespace.
 * @KDBUS_CMD_EP_MAKE:		Creates a new named special endpoint to talk to
 * 				the bus. Such endpoints usually carry a more
 * 				restrictive policy and grant restricted access
 * 				to specific applications.
 * @KDBUS_CMD_HELLO:		By opening the bus device node a connection is
 * 				created. After a HELLO the opened connection
 * 				becomes an active peer on the bus.
 * @KDBUS_CMD_MSG_SEND:		Send a message and pass data from userspace to
 * 				the kernel.
 * @KDBUS_CMD_MSG_RECV:		Receive a message from the kernel which is
 * 				placed in the receiver's pool.
 * @KDBUS_CMD_FREE:		Release the allocated memory in the receiver's
 * 				pool.
 * @KDBUS_CMD_NAME_ACQUIRE:	Request a well-known bus name to associate with
 * 				the connection. Well-known names are used to
 * 				address a peer on the bus.
 * @KDBUS_CMD_NAME_RELEASE:	Release a well-known name the connection
 * 				currently owns.
 * @KDBUS_CMD_NAME_LIST:	Retrieve the list of all currently registered
 * 				well-known and unique names.
 * @KDBUS_CMD_CONN_INFO:	Retrieve credentials and properties of the
 * 				initial creator of the connection. The data was
 * 				stored at registration time and does not
 * 				neccessarily represent the connected process or
 * 				the actual state of the process.
 * @KDBUS_CMD_MATCH_ADD:	Install a match which broadcast messages should
 * 				be delivered to the connection.
 * @KDBUS_CMD_MATCH_REMOVE:	Remove a current match for broadcast messages.
 * @KDBUS_CMD_MONITOR:		Monitor the bus and receive all transmitted
 * 				messages. Privileges are required for this
 * 				operation.
 * @KDBUS_CMD_EP_POLICY_SET:	Set the policy of an endpoint. It is used to
 * 				restrict the access for endpoints created with
 * 				KDBUS_CMD_EP_MAKE.
 * @KDBUS_CMD_MEMFD_NEW:	Return a new file descriptor which provides an
 * 				anonymous shared memory file and which can be
 * 				used to pass around larger chunks of data.
 * 				Kdbus memfd files can be sealed, which allows
 * 				the receiver to trust the data it has received.
 * 				Kdbus memfd files expose only very limited
 * 				operations, they can be mmap()ed, seek()ed,
 * 				(p)read(v)() and (p)write(v)(); most other
 * 				common file operations are not implemented.
 * 				Special caution needs to be taken with
 * 				read(v)()/write(v)() on a shared file; the
 * 				underlying file position is always shared
 * 				between all users of the file and race against
 * 				each other, pread(v)()/pwrite(v)() avoid these
 * 				issues.
 * @KDBUS_CMD_MEMFD_SIZE_GET:	Return the size of the underlying file, which
 * 				changes with write().
 * @KDBUS_CMD_MEMFD_SIZE_SET:	Truncate the underlying file to the specified
 * 				size.
 * @KDBUS_CMD_MEMFD_SEAL_GET:	Return the state of the file sealing.
 * @KDBUS_CMD_MEMFD_SEAL_SET:	Seal or break a seal of the file. Only files
 * 				which are not shared with other processes and
 * 				which are currently not mapped can be sealed.
 * 				The current process needs to be the one and
 * 				single owner of the file, the sealing cannot
 * 				be changed as long as the file is shared.
 */
enum kdbus_ioctl_type {
	KDBUS_CMD_BUS_MAKE =		_IOW (KDBUS_IOC_MAGIC, 0x00, struct kdbus_cmd_bus_make),
	KDBUS_CMD_NS_MAKE =		_IOR (KDBUS_IOC_MAGIC, 0x10, struct kdbus_cmd_ns_make),

	KDBUS_CMD_EP_MAKE =		_IOW (KDBUS_IOC_MAGIC, 0x20, struct kdbus_cmd_ep_make),
	KDBUS_CMD_HELLO =		_IOWR(KDBUS_IOC_MAGIC, 0x30, struct kdbus_cmd_hello),

	KDBUS_CMD_MSG_SEND =		_IOW (KDBUS_IOC_MAGIC, 0x40, struct kdbus_msg),
	KDBUS_CMD_MSG_RECV =		_IOR (KDBUS_IOC_MAGIC, 0x41, __u64 *),
	KDBUS_CMD_FREE =		_IOW (KDBUS_IOC_MAGIC, 0x42, __u64 *),

	KDBUS_CMD_NAME_ACQUIRE =	_IOWR(KDBUS_IOC_MAGIC, 0x50, struct kdbus_cmd_name),
	KDBUS_CMD_NAME_RELEASE =	_IOW (KDBUS_IOC_MAGIC, 0x51, struct kdbus_cmd_name),
	KDBUS_CMD_NAME_LIST =		_IOWR(KDBUS_IOC_MAGIC, 0x52, struct kdbus_cmd_name_list),

	KDBUS_CMD_CONN_INFO =		_IOWR(KDBUS_IOC_MAGIC, 0x60, struct kdbus_cmd_conn_info),

	KDBUS_CMD_MATCH_ADD =		_IOW (KDBUS_IOC_MAGIC, 0x70, struct kdbus_cmd_match),
	KDBUS_CMD_MATCH_REMOVE =	_IOW (KDBUS_IOC_MAGIC, 0x71, struct kdbus_cmd_match),
	KDBUS_CMD_MONITOR =		_IOW (KDBUS_IOC_MAGIC, 0x72, struct kdbus_cmd_monitor),

	KDBUS_CMD_EP_POLICY_SET =	_IOW (KDBUS_IOC_MAGIC, 0x80, struct kdbus_cmd_policy),

	KDBUS_CMD_MEMFD_NEW =		_IOR (KDBUS_IOC_MAGIC, 0x90, int *),
	KDBUS_CMD_MEMFD_SIZE_GET =	_IOR (KDBUS_IOC_MAGIC, 0x91, __u64 *),
	KDBUS_CMD_MEMFD_SIZE_SET =	_IOW (KDBUS_IOC_MAGIC, 0x92, __u64 *),
	KDBUS_CMD_MEMFD_SEAL_GET =	_IOR (KDBUS_IOC_MAGIC, 0x93, int *),
	KDBUS_CMD_MEMFD_SEAL_SET =	_IO  (KDBUS_IOC_MAGIC, 0x94),
};

/*
 * errno - api error codes
 * @E2BIG:		A message contains too many records or items.
 * @EADDRNOTAVAIL:	A message flagged not to activate a service, addressed
 * 			a service which is not currently running.
 * @EAGAIN:		No messages are queued at the moment.
 * @EBADF:		File descriptors passed with the message are not valid.
 * @EBADFD:		A bus connection is in a corrupted state.
 * @EBADMSG:		Passed data contains a combination of conflicting or
 * 			inconsistent types.
 * @EBUSY:		A well-known bus name is already taken by another
 * 			connection.
 * @ECOMM:		A peer does not accept the file descriptors addressed
 * 			to it.
 * @EDESTADDRREQ:	The well-known bus name is required but missing.
 * @EDOM:		The size of data does not match the expectations. Used
 * 			for the size of the bloom filter bit field.
 * @EEXIST:		A requested namespace, bus or endpoint with the same
 * 			name already exists.  A specific data type, which is
 * 			only expected once, is provided multiple times.
 * @EFAULT:		The supplied memory could not be accessed, or the data
 * 			is not properly aligned.
 * @EINVAL:		The provided data does not match its type or other
 * 			expectations, like a string which is not NUL terminated,
 * 			or a string length that points behind the first
 * 			\0-byte in the string.
 * @EMEDIUMTYPE:	A file descriptor which is not a kdbus memfd was
 * 			refused to send as KDBUS_MSG_PAYLOAD_MEMFD.
 * @EMFILE:		Too many file descriptors have been supplied with a
 * 			message.
 * @EMSGSIZE:		The supplied data is larger than the allowed maximum
 * 			size.
 * @ENAMETOOLONG:	The requested name is larger than the allowed maximum
 * 			size.
 * @ENOBUFS:		There is no space left for the submitted data to fit
 * 			into the receiver's pool.
 * @ENOMEM:		Out of memory.
 * @ENOSYS:		The requested functionality is not available.
 * @ENOTCONN:		The addressed peer is not an active connection.
 * @ENOTSUPP:		The feature negotiation failed, a not supported feature
 * 			was requested.
 * @ENOTTY:		An unknown ioctl command was received.
 * @ENOTUNIQ:		A specific data type was addressed to a broadcast
 * 			address, but only direct addresses support this kind of
 * 			data.
 * @ENXIO:		A unique address does not exist.
 * @EPERM:		The policy prevented an operation. The requested
 * 			resource is owned by another entity.
 * @ESHUTDOWN:		The connection is currently shutting down, no further
 * 			operations are possible.
 * @ESRCH:		A requested well-known bus name is not found.
 * @ETXTBSY:		A kdbus memfd file cannot be sealed or the seal removed,
 * 			because it is shared with other processes or still
 * 			mmap()ed.
 * @EXFULL:		The size limits in the pool are reached, no data of
 * 			the size tried to submit can be queued.
 */
#endif
