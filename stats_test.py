#!/usr/bin/env python
# _*_ coding:utf-8 _*_
import pymemcache

mc = pymemcache.Client(('localhost', 9999))
mc.shutdown()