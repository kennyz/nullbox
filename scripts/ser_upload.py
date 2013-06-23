import time
import serial
import smtplib
import xively
import datetime
import re
import os

TO = 'putyour@email.here'
GMAIL_USER = 'putyour@email.here'
GMAIL_PASS = 'putyourpasswordhere'

SUBJECT = 'Intrusion!!'
TEXT = 'Your PIR sensor detected movement'
  
ser = serial.Serial('/dev/ttyACM0', 19200)
XIVELY_APIKEY=os.getenv("XIVELY_APIKEY");

def send_email():
    print("Sending Email")
    smtpserver = smtplib.SMTP("smtp.gmail.com",587)
    smtpserver.ehlo()
    smtpserver.starttls()
    smtpserver.ehlo
    smtpserver.login(GMAIL_USER, GMAIL_PASS)
    header = 'To:' + TO + '\n' + 'From: ' + GMAIL_USER
    header = header + '\n' + 'Subject:' + SUBJECT + '\n'
    print header
    msg = header + '\n' + TEXT + ' \n\n'
    smtpserver.sendmail(GMAIL_USER, TO, msg)
    smtpserver.close()

api = xively.XivelyAPIClient(XIVELY_APIKEY);
print "API["+XIVELY_APIKEY+"]\n";
feed = api.feeds.get("1910880543")
datastream1 = feed.datastreams.get("Temperature")
datastream2 = feed.datastreams.get("Humidity")
sensor_list = {"TE":"Temperature","HU":"Humidity","SD":"Sound","LT":"Light","RD":"Red"};
datastream = {}
for sensor in sensor_list:
    datastream[sensor] = feed.datastreams.get(sensor_list[sensor]);

def update_file(filename,data):
    f = file(filename, 'w') # open for 'w'riting
    f.write(data) # write text to file
    f.close() # close the file

index = 0;    
ds = []
dss = {}
while True:
    message = ser.readline()
    #print(message)
    if(message.find("[TH]")>=0 ):
        #p = re.compile(r'\[TH\](.+),(.+)\r')
        strlist = message[4:].rstrip("\r\n").split(",");
        print strlist
        if(len(strlist)<2):
            continue;

        now_iso = datetime.datetime.now().isoformat()
        if(index%5==0):
            update_file("data_tem.txt",'{\n"timestamp":"'+now_iso+'","value":'+strlist[1]+'\n}')
            update_file("data_hum.txt",'{\n"timestamp":"'+now_iso+'","value":'+strlist[0]+'\n}')
            index = index +1
    #feed.datastreams = [];
    for sense in sensor_list:
        if(message.find("["+sense+"]")>=0 ):
            strval = message[4:].rstrip("\r\n")
            print sensor_list[sense]+":"+strval
            now = datetime.datetime.utcnow()
            #datastream[sense].current_value = strval
            #datastream[sense].at = now
            #datastream[sense].update();
            #ds.append(xively.Datastream(id=sense, current_value=strval, at=now));
            dss[sense] = xively.Datastream(id=sensor_list[sense], current_value=strval, at=now);
    if(len(dss)==5):
        for sense in dss:
            ds.append(dss[sense]);
        feed.datastreams = ds;
        try:
            feed.update();
            print "* "+str(len(ds))+" items uploaded";
        except:
            print 'exception on update\n'
        ds = []
        dss = {}
    if message[0] == 'M' :
        send_email()
    #time.sleep(1)


