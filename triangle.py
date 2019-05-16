#!/usr/bin/env python

MAX = 8

T_N = [0, 0, 1, 3, 6, 10, 15, 21, 28]

def compute_edge_mask(idx):
	e = 0
	for i in range(0, idx):
		bit = T_N[idx] + i
		e |= 1 << bit
	for i in range(idx+1, MAX):
		bit = T_N[i] + idx
		e |= 1 << bit
	return e

def compute_edge_mask_format(idx):
	return "{0:b}".format(compute_edge_mask(idx))

for idx in range(0, MAX):
	print('edge mask for %d: %s' % (idx, compute_edge_mask_format(idx)))

