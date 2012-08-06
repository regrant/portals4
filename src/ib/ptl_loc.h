#ifndef PTL_LOC_H
#define PTL_LOC_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define _XOPEN_SOURCE 600
#define _DARWIN_C_SOURCE		/* For Mac OS X */

#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <sched.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <assert.h>
#include <poll.h>
#include <sys/time.h>
#include <search.h>
#include <sys/ioctl.h>
#ifdef HAVE_ENDIAN_H
# include <endian.h>
#endif

#include "tree.h"

#if WITH_TRANSPORT_IB
#include <rdma/rdma_cma.h>
#include <infiniband/verbs.h>
#endif

#include "portals4.h"

#include "ptl_byteorder.h"
#include "ptl_log.h"
#include "ptl_list.h"
#include "ptl_lockfree.h"
#include "ptl_sync.h"
#include "ptl_ref.h"
#include "ptl_atomic.h"
#include "ptl_param.h"
#include "ptl_evloop.h"
#include "ptl_pool.h"
#include "ptl_queue.h"
#include "ptl_xpmem.h"
#include "ptl_gbl.h"
#include "ptl_obj.h"
#include "ptl_ppe.h"
#include "p4ppe.h"
#include "ptl_iface.h"
#include "ptl_pt.h"
#include "ptl_ni.h"
#include "ptl_data.h"
#include "ptl_conn.h"
#include "ptl_mr.h"
#include "ptl_md.h"
#include "ptl_le.h"
#include "ptl_me.h"
#include "ptl_ct.h"
#include "ptl_buf.h"
#include "ptl_eq.h"
#include "ptl_hdr.h"
#include "ptl_misc.h"

enum recv_state {
	STATE_RECV_SEND_COMP,
	STATE_RECV_RDMA_COMP,
	STATE_RECV_PACKET_RDMA,
	STATE_RECV_PACKET,
	STATE_RECV_DROP_BUF,
	STATE_RECV_REQ,
	STATE_RECV_INIT,
	STATE_RECV_REPOST,
	STATE_RECV_ERROR,
	STATE_RECV_DONE,
};

enum tgt_state {
	STATE_TGT_START,
	STATE_TGT_DROP,
	STATE_TGT_GET_MATCH,
	STATE_TGT_GET_LENGTH,
	STATE_TGT_WAIT_CONN,
	STATE_TGT_DATA,
	STATE_TGT_DATA_IN,
	STATE_TGT_START_COPY,
	STATE_TGT_RDMA,
	STATE_TGT_ATOMIC_DATA_IN,
	STATE_TGT_SWAP_DATA_IN,
	STATE_TGT_DATA_OUT,
	STATE_TGT_WAIT_RDMA_DESC,
	STATE_TGT_SHMEM_DESC,
	STATE_TGT_SEND_ACK,
	STATE_TGT_SEND_REPLY,
	STATE_TGT_COMM_EVENT,
	STATE_TGT_WAIT_APPEND,
	STATE_TGT_OVERFLOW_EVENT,
	STATE_TGT_CLEANUP,
	STATE_TGT_CLEANUP_2,
	STATE_TGT_ERROR,
	STATE_TGT_DONE,
};

enum init_state {
	STATE_INIT_START,
	STATE_INIT_PREP_REQ,
	STATE_INIT_WAIT_CONN,
	STATE_INIT_SEND_REQ,
	STATE_INIT_COPY_START,
	STATE_INIT_COPY_IN,
	STATE_INIT_COPY_OUT,
	STATE_INIT_COPY_DONE,
	STATE_INIT_WAIT_COMP,
	STATE_INIT_SEND_ERROR,
	STATE_INIT_EARLY_SEND_EVENT,
	STATE_INIT_WAIT_RECV,
	STATE_INIT_DATA_IN,
	STATE_INIT_LATE_SEND_EVENT,
	STATE_INIT_ACK_EVENT,
	STATE_INIT_REPLY_EVENT,
	STATE_INIT_CLEANUP,
	STATE_INIT_ERROR,
	STATE_INIT_DONE,
	STATE_INIT_LAST,
};

/* In current implementation a NID is just an IPv4 address in host order. */
static inline in_addr_t nid_to_addr(ptl_nid_t nid)
{
	return htonl(nid);
}

static inline ptl_nid_t addr_to_nid(struct sockaddr_in *sin)
{
	return ntohl(sin->sin_addr.s_addr);
}

/* A PID is a port in host order. */
static inline __be16 pid_to_port(ptl_pid_t pid)
{
	if (pid == PTL_PID_ANY) {
		return 0;
	} else {
		return htons(pid);
	}
}

static inline ptl_pid_t port_to_pid(__be16 port)
{
	return ntohs(port);
}

/* round up x to multiple of y. y MUST be a power of 2. */
#define ROUND_UP(x,y) ((x) + (y) - 1) & ~((y)-1);

int iov_copy_out(void *dst, ptl_iovec_t *iov, mr_t **mr_list, ptl_size_t num_iov,
		 ptl_size_t offset, ptl_size_t length);

int iov_copy_in(void *src, ptl_iovec_t *iov, mr_t **mr_list, ptl_size_t num_iov,
		ptl_size_t offset, ptl_size_t length);

int iov_atomic_in(atom_op_t op, int atom_size, void *src,
		  ptl_iovec_t *iov,  mr_t **mr_list, ptl_size_t num_iov,
		  ptl_size_t offset, ptl_size_t length);

int iov_count_elem(ptl_iovec_t *iov, ptl_size_t num_iov,
				   ptl_size_t offset, ptl_size_t length,
				   ptl_size_t *index_p, ptl_size_t *base_p);

int swap_data_in(ptl_op_t atom_op, ptl_datatype_t atom_type,
		 void *dest, void *source, void *operand);

int process_rdma_desc(buf_t *buf);

void *progress_thread(void *arg);

int process_init(buf_t *buf);

int process_tgt(buf_t *buf);

int check_match(buf_t *buf, const me_t *me);

int check_perm(buf_t *buf, const le_t *le);

#ifdef WITH_TRANSPORT_IB
int PtlNIInit_IB(iface_t *iface, ni_t *ni);
void cleanup_ib(ni_t *ni);
int init_iface_ib(iface_t *iface);
void initiate_disconnect_all(ni_t *ni);
void disconnect_conn_locked(conn_t *conn);
void progress_thread_ib(ni_t *ni);
#else
static inline int PtlNIInit_IB(iface_t *iface, ni_t *ni) { return PTL_OK; }
static inline void cleanup_ib(ni_t *ni) {  }
static inline int init_iface_ib(iface_t *iface) { return PTL_OK; }
static inline void initiate_disconnect_all(ni_t *ni) { }
static inline void progress_thread_ib(ni_t *ni) { }
#endif

#ifdef WITH_TRANSPORT_SHMEM
extern int PtlNIInit_shmem(ni_t *ni);
void cleanup_shmem(ni_t *ni);
int setup_shmem(ni_t *ni);
void shmem_enqueue(ni_t *ni, buf_t *buf, ptl_pid_t dest);
buf_t *shmem_dequeue(ni_t *ni);
#else
static inline int PtlNIInit_shmem(ni_t *ni) { return PTL_OK; }
static inline void cleanup_shmem(ni_t *ni) { }
static inline int setup_shmem(ni_t *ni) { return PTL_OK; }
#endif

#if (WITH_TRANSPORT_SHMEM && USE_KNEM)
int knem_init(ni_t *ni);
void knem_fini(ni_t *ni);
uint64_t knem_register(ni_t *ni, void *data, ptl_size_t len, int prot);
void knem_unregister(ni_t *ni, uint64_t cookie);
size_t knem_copy_from(ni_t * ni, void *dst,
					  uint64_t cookie, uint64_t off, size_t len);
size_t knem_copy_to(ni_t * ni, uint64_t cookie,
					uint64_t off, void *src, size_t len);
size_t knem_copy(ni_t * ni,
				 uint64_t scookie, uint64_t soffset,
				 uint64_t dcookie, uint64_t doffset,
				 size_t length);
#else
static inline int knem_init(ni_t *ni) { return PTL_OK; }
static inline void knem_fini(ni_t *ni) { }
static inline uint64_t knem_register(ni_t *ni, void *data, ptl_size_t len, int prot) { return 1; }
static inline void knem_unregister(ni_t *ni, uint64_t cookie) { }
#endif

#if WITH_TRANSPORT_SHMEM || IS_PPE
void PtlSetMap_mem(ni_t *ni, ptl_size_t map_size,
				   const ptl_process_t *mapping);
void process_recv_mem(ni_t *ni, buf_t *buf);
int do_mem_transfer(buf_t *buf);
ptl_size_t copy_mem_to_mem(ni_t *ni, data_dir_t dir, struct mem_iovec *remote_iovec,
						   void *local_addr, mr_t *local_mr, ptl_size_t len);
#else
static inline void PtlSetMap_mem(ni_t *ni, ptl_size_t map_size,
								 const ptl_process_t *mapping) { }
#endif

#ifdef WITH_TRANSPORT_UDP
int PtlNIInit_UDP(iface_t *iface, ni_t *ni);
int init_iface_udp(iface_t *iface);
void PtlSetMap_udp(ni_t *ni, ptl_size_t map_size, const ptl_process_t *mapping);
#else
static inline int PtlNIInit_UDP(iface_t *iface, ni_t *ni) { return PTL_OK; }
static inline int init_iface_udp(iface_t *iface) { return PTL_OK; }
static inline void PtlSetMap_udp(ni_t *ni, ptl_size_t map_size,
								 const ptl_process_t *mapping) { }
static inline void progress_thread_udp(ni_t *ni) { }
#endif

#ifdef IS_PPE
int PtlNIInit_ppe(gbl_t *gbl, ni_t *ni);

/* Translate a client address to the PPE space given an mr to which the address belong to. */
static inline void *addr_to_ppe(void *addr, mr_t *mr)
{
	assert(addr >= mr->addr && addr < (mr->addr+mr->length));
	return mr->ppe_addr + (addr - mr->addr);
}

#else
static inline int PtlNIInit_ppe(gbl_t *gbl, ni_t *ni) { return PTL_OK; }
#define addr_to_ppe(addr,dontcare) (addr)
#endif


int _PtlInit(gbl_t *gbl);
void _PtlFini(gbl_t *gbl);
int _PtlNIInit(gbl_t *gbl, ptl_interface_t iface_id, unsigned int options,
			   ptl_pid_t  pid, const ptl_ni_limits_t *desired,
			   ptl_ni_limits_t *actual, ptl_handle_ni_t *ni_handle);
int _PtlNIFini(gbl_t *gbl, ptl_handle_ni_t ni_handle);


#endif /* PTL_LOC_H */
