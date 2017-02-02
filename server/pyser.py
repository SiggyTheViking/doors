#!/usr/bin/env python3

import serial
import datetime
import paho.mqtt.publish as pub
import redis
import psycopg2 as pg

ser = serial.Serial('/dev/ttyACM0', 9600)
doors = [
    {'door':'front','open':None},
    {'door':'french','open':None},
    {'door':'kitchen','open':None},
    {'door':'music','open':None},
    {'door':'prayer','open':None}]

def set_door_state(bt,state):
    changes = []
    for i,d in enumerate(state):
        thisDoor = bool(bt & (1 << i))
        if (state[i]['open'] != thisDoor):
            state[i]['open'] = thisDoor
            changes.append(state[i])
    return changes

r = redis.StrictRedis(host='mylocalipaddr', port=6379, db=0)
conn = pg.connect(host='mylocalipaddr',port=5433,dbname='flintstone',user='fred',password='mystrongpassword')
cur = conn.cursor()
old = None
while 1:
    raw = ser.readline()
    this = int(raw.strip()[0])
    if (this != old):
               changes = set_door_state(this,doors)
               print(datetime.datetime.now(), ' ', changes)
               pub.single('doors',this)
               old = this
               for change in changes:
                   r.set('doors.'+change['door'],change['open'])
                   cur.execute("insert into discretehistory (point,eventtime,value) values(%s,%s,%s)",
                          ('doors.'+change['door'],datetime.datetime.now(),change['open']))     
                   conn.commit()
