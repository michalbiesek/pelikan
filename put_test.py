#!/usr/bin/env python
# _*_ coding:utf-8 _*_
import pymemcache

mc = pymemcache.Client(('localhost', 12321))
mc.set("foo", "bar")
ret = mc.get('foo')
print(ret)
