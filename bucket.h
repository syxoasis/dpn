int getBucketID(underlink_node check);
underlink_node getClosestAddressFromBuckets(underlink_node check, int steps, underlink_routermode routermode);
int addNodeToBuckets(underlink_node newnode);
int nodeIDComparator(const underlink_node* a, const underlink_node* b);