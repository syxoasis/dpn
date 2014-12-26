#pragma once

int getBucketID(dpn_node check);
dpn_node getClosestAddressFromBuckets(dpn_node check, int stepse);
int addNodeToBuckets(dpn_node newnode);
int nodeIDComparator(const dpn_node* a, const dpn_node* b);
