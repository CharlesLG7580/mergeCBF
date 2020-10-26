# mergeCBF
mergeCountingBloomFilter
This What is this?
=============

This is an implemation of mergeCountingBloomFilter written in C++.

ccbf.c: Implementation
mergecbf.h: API and documentation
main.cc: 
murmur.cc

What is mergeCBF?
==========================

A mergeCBF is a compact structure which efficiently supports the
following operations :

- Insert(item)
- Delete(item)
- Query(item) 
- Merge(ccbf1,ccbf2 )

Like Counting Bloom filters, Merge Counting Bloom filter support the item 
insert, delete, querye. Advantages of mergeCBF over Bloom filters include:

- Efficient query
- Lower false positive
- The ability to merge filters

Advantages of Counting Bloom filters over mergeCBF include:

- Significantly lower implementation complexity
- Efficient insert and delete

Example usage
=============

  ComposableCountingrBloomFilter bloom1(100000, 0.15);
  ComposableCountingrBloomFilter bloom2(100000, 0.05)
  for (int i = 0; i < length; i++){
		bloom1.queryItem(i);
	}
  bloom1.update_orbarr();
  bloom1.deleteItem(rand()%100000);
  bloom1.update_orbarr();
  bloom1.queryItem(rand()%100000);
  bloom1.mergeBloom(bloom2)
 

HashFunction
=================================



References
==========
