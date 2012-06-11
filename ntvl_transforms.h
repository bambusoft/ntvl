/* (c) 2009 Richard Andrews */

#if !defined(NTVL_TRANSFORMS_H_)
#define NTVL_TRANSFORMS_H_

#include "ntvl_keyfile.h"
#include "ntvl_wire.h"


#define NTVL_TRANSFORM_ID_INVAL          0       /* marks uninitialised data */
#define NTVL_TRANSFORM_ID_NULL           1
#define NTVL_TRANSFORM_ID_TWOFISH        2
#define NTVL_TRANSFORM_ID_AESCBC         3
#define NTVL_TRANSFORM_ID_LZO            4
#define NTVL_TRANSFORM_ID_TWOFISH_LZO    5
#define NTVL_TRANSFORM_ID_AESCBC_LZO     6
#define NTVL_TRANSFORM_ID_USER_START     64
#define NTVL_TRANSFORM_ID_MAX            65535


struct ntvl_trans_op;
typedef struct ntvl_trans_op ntvl_trans_op_t;

struct ntvl_tostat {
    uint8_t             can_tx;         /* Does this transop have a valid SA for encoding. */
    ntvl_cipherspec_t    tx_spec;        /* If can_tx, the spec used to encode. */
};

typedef struct ntvl_tostat ntvl_tostat_t;


typedef int             (*ntvl_transdeinit_f)( ntvl_trans_op_t * arg );
typedef int             (*ntvl_transaddspec_f)( ntvl_trans_op_t * arg, 
                                               const ntvl_cipherspec_t * cspec );
typedef ntvl_tostat_t    (*ntvl_transtick_f)( ntvl_trans_op_t * arg, 
                                            time_t now );

typedef int             (*ntvl_transform_f)( ntvl_trans_op_t * arg,
                                            uint8_t * outbuf,
                                            size_t out_len,
                                            const uint8_t * inbuf,
                                            size_t in_len );

/** Holds the info associated with a data transform plugin.
 *
 *  When a packet arrives the transform ID is extracted. This defines the code
 *  to use to decode the packet content. The transform code then decodes the
 *  packet and consults its internal key lookup.
 */
struct ntvl_trans_op {
    void *              priv;   /* opaque data. Key schedule goes here. */

    ntvl_transform_t     transform_id;   /* link header enum to a transform */
    size_t              tx_cnt;
    size_t              rx_cnt;

    ntvl_transdeinit_f   deinit; /* destructor function */
    ntvl_transaddspec_f  addspec; /* parse opaque data from a key schedule file. */
    ntvl_transtick_f     tick;   /* periodic maintenance */
    ntvl_transform_f     fwd;    /* encode a payload */
    ntvl_transform_f     rev;    /* decode a payload */
};

/* Setup a single twofish SA for single-key operation. */
int transop_twofish_setup( ntvl_trans_op_t * ttt, 
                           ntvl_sa_t sa_num,
                           uint8_t * encrypt_pwd, 
                           uint32_t encrypt_pwd_len );

/* Initialise an empty transop ready to receive cipherspec elements. */
int  transop_twofish_init( ntvl_trans_op_t * ttt );
int  transop_aes_init( ntvl_trans_op_t * ttt );
void transop_null_init( ntvl_trans_op_t * ttt );

#endif /* #if !defined(NTVL_TRANSFORMS_H_) */

