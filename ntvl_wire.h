/* (c) 2012 Mario Ricardo Rodriguez
 *
 * Contributions by:
 *    Richard Andrews
 *    Luca Deri
 *    Lukasz Taczuk
 */

#if !defined( NTVL_WIRE_H_ )
#define NTVL_WIRE_H_

#include <stdlib.h>

#if defined(WIN32)
#include "win32/ntvl_win32.h"

#if defined(__MINGW32__)
#include <stdint.h>
#endif /* #ifdef __MINGW32__ */

#else /* #if defined(WIN32) */
#include <stdint.h>
#include <netinet/in.h>
#include <sys/socket.h> /* AF_INET and AF_INET6 */
#endif /* #if defined(WIN32) */

#define NTVL_PKT_VERSION                 2
#define NTVL_DEFAULT_TTL                 2       /* can be forwarded twice at most */
#define NTVL_COMMUNITY_SIZE              16
#define NTVL_MAC_SIZE                    6
#define NTVL_COOKIE_SIZE                 4
#define NTVL_PKT_BUF_SIZE                2048
#define NTVL_SOCKBUF_SIZE                64      /* string representation of INET or INET6 sockets */

typedef uint8_t ntvl_community_t[NTVL_COMMUNITY_SIZE];
typedef uint8_t ntvl_mac_t[NTVL_MAC_SIZE];
typedef uint8_t ntvl_cookie_t[NTVL_COOKIE_SIZE];

typedef char    ntvl_sock_str_t[NTVL_SOCKBUF_SIZE];       /* tracing string buffer */

enum ntvl_pc {
    ntvl_ping=0,                 /* Not used */
    ntvl_register=1,             /* Register edge to edge */
    ntvl_deregister=2,           /* Deregister this edge */
    ntvl_packet=3,               /* PACKET data content */
    ntvl_register_ack=4,         /* ACK of a registration from edge to edge */
    ntvl_register_super=5,       /* Register edge to supernode */
    ntvl_register_super_ack=6,   /* ACK from supernode to edge */
    ntvl_register_super_nak=7,   /* NAK from supernode to edge - registration refused */
    ntvl_federation=8            /* Not used by edge */
};

typedef enum ntvl_pc ntvl_pc_t;

#define NTVL_FLAGS_OPTIONS               0x0080
#define NTVL_FLAGS_SOCKET                0x0040
#define NTVL_FLAGS_FROM_SUPERNODE        0x0020

/* The bits in flag that are the packet type */
#define NTVL_FLAGS_TYPE_MASK             0x001f  /* 0 - 31 */
#define NTVL_FLAGS_BITS_MASK             0xffe0

#define IPV4_SIZE                       4
#define IPV6_SIZE                       16


#define NTVL_AUTH_TOKEN_SIZE             32      /* bytes */


#define NTVL_EUNKNOWN                    -1
#define NTVL_ENOTIMPL                    -2
#define NTVL_EINVAL                      -3
#define NTVL_ENOSPACE                    -4


typedef uint16_t ntvl_flags_t;
typedef uint16_t ntvl_transform_t;       /* Encryption, compression type. */
typedef uint32_t ntvl_sa_t;              /* security association number */

struct ntvl_sock {
    uint8_t     family;         /* AF_INET or AF_INET6; or 0 if invalid */
    uint16_t    port;           /* host order */
    union 
    {
    uint8_t     v6[IPV6_SIZE];  /* byte sequence */
    uint8_t     v4[IPV4_SIZE];  /* byte sequence */
    } addr;
};

typedef struct ntvl_sock ntvl_sock_t;

struct ntvl_auth {
    uint16_t    scheme;                         /* What kind of auth */
    uint16_t    toksize;                        /* Size of auth token */
    uint8_t     token[NTVL_AUTH_TOKEN_SIZE];     /* Auth data interpreted based on scheme */
};

typedef struct ntvl_auth ntvl_auth_t;


struct ntvl_common {
    /* int                 version; */
    uint8_t             ttl;
    ntvl_pc_t            pc;
    ntvl_flags_t         flags;
    ntvl_community_t     community;
};

typedef struct ntvl_common ntvl_common_t;

struct ntvl_REGISTER {
    ntvl_cookie_t        cookie;         /* Link REGISTER and REGISTER_ACK */
    ntvl_mac_t           srcMac;         /* MAC of registering party */
    ntvl_mac_t           dstMac;         /* MAC of target edge */
    ntvl_sock_t          sock;           /* REVISIT: unused? */
};

typedef struct ntvl_REGISTER ntvl_REGISTER_t;

struct ntvl_REGISTER_ACK {
    ntvl_cookie_t        cookie;         /* Return cookie from REGISTER */
    ntvl_mac_t           srcMac;         /* MAC of acknowledging party (supernode or edge) */
    ntvl_mac_t           dstMac;         /* Reflected MAC of registering edge from REGISTER */
    ntvl_sock_t          sock;           /* Supernode's view of edge socket (IP Addr, port) */
};

typedef struct ntvl_REGISTER_ACK ntvl_REGISTER_ACK_t;

struct ntvl_PACKET {
    ntvl_mac_t           srcMac;
    ntvl_mac_t           dstMac;
    ntvl_sock_t          sock;
    ntvl_transform_t     transform;
};

typedef struct ntvl_PACKET ntvl_PACKET_t;


/* Linked with ntvl_register_super in ntvl_pc_t. Only from edge to supernode. */
struct ntvl_REGISTER_SUPER {
    ntvl_cookie_t        cookie;         /* Link REGISTER_SUPER and REGISTER_SUPER_ACK */
    ntvl_mac_t           edgeMac;        /* MAC to register with edge sending socket */
    ntvl_auth_t          auth;           /* Authentication scheme and tokens */
};

typedef struct ntvl_REGISTER_SUPER ntvl_REGISTER_SUPER_t;


/* Linked with ntvl_register_super_ack in ntvl_pc_t. Only from supernode to edge. */
struct ntvl_REGISTER_SUPER_ACK {
    ntvl_cookie_t        cookie;         /* Return cookie from REGISTER_SUPER */
    ntvl_mac_t           edgeMac;        /* MAC registered to edge sending socket */
    uint16_t            lifetime;       /* How long the registration will live */
    ntvl_sock_t          sock;           /* Sending sockets associated with edgeMac */

    /* The packet format provides additional supernode definitions here. 
     * uint8_t count, then for each count there is one
     * ntvl_sock_t.
     */
    uint8_t             num_sn;         /* Number of supernodes that were send
                                         * even if we cannot store them all. If
                                         * non-zero then sn_bak is valid. */
    ntvl_sock_t          sn_bak;         /* Socket of the first backup supernode */

};

typedef struct ntvl_REGISTER_SUPER_ACK ntvl_REGISTER_SUPER_ACK_t;


/* Linked with ntvl_register_super_ack in ntvl_pc_t. Only from supernode to edge. */
struct ntvl_REGISTER_SUPER_NAK {
    ntvl_cookie_t        cookie;         /* Return cookie from REGISTER_SUPER */
};

typedef struct ntvl_REGISTER_SUPER_NAK ntvl_REGISTER_SUPER_NAK_t;



struct ntvl_buf {
    uint8_t *   data;
    size_t      size;
};

typedef struct ntvl_buf ntvl_buf_t;

int encode_uint8( uint8_t * base, size_t * idx, const uint8_t v );
int decode_uint8( uint8_t * out, const uint8_t * base, size_t * rem, size_t * idx );
int encode_uint16( uint8_t * base, size_t * idx, const uint16_t v );
int decode_uint16( uint16_t * out, const uint8_t * base, size_t * rem, size_t * idx );
int encode_uint32( uint8_t * base, size_t * idx, const uint32_t v );
int decode_uint32( uint32_t * out, const uint8_t * base, size_t * rem, size_t * idx );
int encode_buf( uint8_t * base, size_t * idx, const void * p, size_t s);
int decode_buf( uint8_t * out, size_t bufsize, const uint8_t * base, size_t * rem, size_t * idx );
int encode_mac( uint8_t * base, size_t * idx, const ntvl_mac_t m );
int decode_mac( uint8_t * out, const uint8_t * base, size_t * rem, size_t * idx ); /* of size NTVL_MAC_SIZE. This clearer than passing a ntvl_mac_t */
int encode_common( uint8_t * base, size_t * idx, const ntvl_common_t * common );
int decode_common( ntvl_common_t * out, const uint8_t * base, size_t * rem, size_t * idx );
int encode_sock( uint8_t * base, size_t * idx, const ntvl_sock_t * sock );
int decode_sock( ntvl_sock_t * sock, const uint8_t * base, size_t * rem, size_t * idx );
int encode_REGISTER( uint8_t * base, size_t * idx, const ntvl_common_t * common, const ntvl_REGISTER_t * reg );
int decode_REGISTER( ntvl_REGISTER_t * pkt, const ntvl_common_t * cmn, const uint8_t * base, size_t * rem, size_t * idx ); /* info on how to interpret it */
int encode_REGISTER_SUPER( uint8_t * base, size_t * idx, const ntvl_common_t * common, const ntvl_REGISTER_SUPER_t * reg );
int decode_REGISTER_SUPER( ntvl_REGISTER_SUPER_t * pkt, const ntvl_common_t * cmn, const uint8_t * base, size_t * rem, size_t * idx );
int encode_REGISTER_ACK( uint8_t * base, size_t * idx, const ntvl_common_t * common, const ntvl_REGISTER_ACK_t * reg );
int decode_REGISTER_ACK( ntvl_REGISTER_ACK_t * pkt, const ntvl_common_t * cmn, const uint8_t * base, size_t * rem, size_t * idx );
int encode_REGISTER_SUPER_ACK( uint8_t * base, size_t * idx, const ntvl_common_t * cmn, const ntvl_REGISTER_SUPER_ACK_t * reg );
int decode_REGISTER_SUPER_ACK( ntvl_REGISTER_SUPER_ACK_t * reg, const ntvl_common_t * cmn, const uint8_t * base, size_t * rem, size_t * idx );
int fill_sockaddr( struct sockaddr * addr, size_t addrlen, const ntvl_sock_t * sock );
int encode_PACKET( uint8_t * base, size_t * idx, const ntvl_common_t * common, const ntvl_PACKET_t * pkt );
int decode_PACKET( ntvl_PACKET_t * pkt, const ntvl_common_t * cmn, const uint8_t * base, size_t * rem, size_t * idx ); /* info on how to interpret it */


#endif /* #if !defined( NTVL_WIRE_H_ ) */
