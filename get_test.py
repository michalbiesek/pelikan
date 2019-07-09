#!/usr/bin/env python
# _*_ coding:utf-8 _*_
import pymemcache

mc = pymemcache.Client(('localhost', 12321))
for c in range(0, 100):
	ret = mc.get('foo'+str(c))
	print("zapytanie " + str(c) + "  " +str(ret))
