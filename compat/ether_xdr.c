/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include <rpc/rpc.h>

/* Some machines have incompatible definitions for h_addr. */
#undef h_addr
#include "ether.h"

bool_t
xdr_ethertimeval(xdrs, objp)
	XDR *xdrs;
	ethertimeval *objp;
{
	if (!xdr_u_int(xdrs, &objp->tv_seconds)) {
		return (FALSE);
	}
	if (!xdr_u_int(xdrs, &objp->tv_useconds)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_etherstat(xdrs, objp)
	XDR *xdrs;
	etherstat *objp;
{
	if (!xdr_ethertimeval(xdrs, &objp->e_time)) {
		return (FALSE);
	}
	if (!xdr_u_int(xdrs, &objp->e_bytes)) {
		return (FALSE);
	}
	if (!xdr_u_int(xdrs, &objp->e_packets)) {
		return (FALSE);
	}
	if (!xdr_u_int(xdrs, &objp->e_bcast)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->e_size, NBUCKETS, sizeof(u_int), xdr_u_int)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->e_proto, NPROTOS, sizeof(u_int), xdr_u_int)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_etherhmem_node(xdrs, objp)
	XDR *xdrs;
	etherhmem_node *objp;
{
	if (!xdr_int(xdrs, &objp->h_addr)) {
		return (FALSE);
	}
	if (!xdr_u_int(xdrs, &objp->h_cnt)) {
		return (FALSE);
	}
	if (!xdr_pointer(xdrs, (char **)&objp->h_nxt, sizeof(etherhmem_node), xdr_etherhmem_node)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_etherhmem(xdrs, objp)
	XDR *xdrs;
	etherhmem *objp;
{
	if (!xdr_pointer(xdrs, (char **)objp, sizeof(etherhmem_node), xdr_etherhmem_node)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_etheraddrs(xdrs, objp)
	XDR *xdrs;
	etheraddrs *objp;
{
	if (!xdr_ethertimeval(xdrs, &objp->e_time)) {
		return (FALSE);
	}
	if (!xdr_u_int(xdrs, &objp->e_bytes)) {
		return (FALSE);
	}
	if (!xdr_u_int(xdrs, &objp->e_packets)) {
		return (FALSE);
	}
	if (!xdr_u_int(xdrs, &objp->e_bcast)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->e_addrs, HASHSIZE, sizeof(etherhmem), xdr_etherhmem)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_addrmask(xdrs, objp)
	XDR *xdrs;
	addrmask *objp;
{
	if (!xdr_int(xdrs, &objp->a_addr)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->a_mask)) {
		return (FALSE);
	}
	return (TRUE);
}
