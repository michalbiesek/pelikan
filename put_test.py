#!/usr/bin/env python
# _*_ coding:utf-8 _*_
import pymemcache
import string
import random
 
 
def string_generator(size):
	chars = string.ascii_uppercase + string.ascii_lowercase
	return ''.join(random.choice(chars) for _ in range(size))

mc = pymemcache.Client(('localhost', 12321))
for c in range(0, 100):
	mc.set("foo"+str(c), "bar"+string_generator(c))
	ret = mc.get('foo'+str(c))
	print("zapytanie " + str(c) + "  " +ret)
	
for c in range(0, 100, 2):
	mc.delete("foo"+str(c), noreply=False)

for c in range(0, 100):
	ret = mc.get('foo'+str(c))
	print("zapytanie " + str(c) + "  " +str(ret))
