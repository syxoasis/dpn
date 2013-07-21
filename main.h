// The maximum number of nodes in each bucket
#define NODES_PER_BUCKET 24

// Maximum transmission unit supported
#define MTU 1400

// Print debug output to stdout
#define debug 1

// Macros for host/network order
#define ntohll(x) (((uint64_t)(ntohl((uint32_t)((x << 32) >> 32))) << 32) | ntohl(((uint32_t)(x >> 32))))                                        
#define htonll(x) ntohll(x)