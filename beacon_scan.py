from bluetooth.ble import BeaconService, DiscoveryService
import sqlite3
import time
import smtplib
from random import randint
import sys

class Beacon(object):

    def __init__(self, data, address):
        self._uuid = data[0]
        self._major = data[1]
        self._minor = data[2]
        self._power = data[3]
        self._rssi = data[4]
        self._address = address

    def __str__(self):
        ret = "Beacon: address:{ADDR} uuid:{UUID} major:{MAJOR}"\
                " minor:{MINOR} txpower:{POWER} rssi:{RSSI}"\
                .format(ADDR=self._address, UUID=self._uuid, MAJOR=self._major,
                        MINOR=self._minor, POWER=self._power, RSSI=self._rssi)
        return ret



def process_code(uuid, code,cursorAuth,time=0):
    strCode = str(code)
    #with sqlite3.connect('AuthDB.db') as connAuth:
    cursorAuth = connAuth.cursor()
    query = "select count (*) from students where code='"+strCode+"' and uuid='"+uuid+"'"
    cursorAuth.execute(query)
    count = cursorAuth.fetchone()[0]
    query = "update students set codeSent = '"+"0"+"' where uuid = '"+uuid+"'" 
    cursorAuth.execute(query)
    if (count==1):
        query = "select timeout from students where code='"+strCode+"' and uuid='"+uuid+"'"
        cursorAuth.execute(query)
        timeout = cursorAuth.fetchone()[0]
        query = "select attempts from students where code='"+strCode+"' and uuid='"+uuid+"'"
        cursorAuth.execute(query)
        attempts = cursorAuth.fetchone()[0]
        if (time-int(timeout)>60):
           return False,attempts
        query = "update students set attempts = '"+"0"+"' where uuid = '"+uuid+"'"
        cursorAuth.execute(query)
        query = "update students set timeout = '"+"0"+"'where uuid = '"+uuid+"'"
        cursorAuth.execute(query)
        return True,attempts
    else:
        return False,0


def send(uuid,cursorAuth,time=0):
        code = str(randint(11111,65535))
        #with sqlite3.connect('AuthDB.db') as connAuth:
        cursorAuth = connAuth.cursor()
        query = "update students set code='"+code+"' where uuid='"+uuid+"'"
        cursorAuth.execute(query)
        print (uuid)
        query = "select code from students where uuid = '"+uuid+"'"
        print (cursor.fetchone())
        query = "update students set timeout='"+str(time)+"' where uuid='"+uuid+"'"
        cursorAuth.execute(query)

        query = "select attempts from students where code='"+code+"' and uuid='"+uuid+"'"
        cursorAuth.execute(query)
        attempts = cursorAuth.fetchone()[0] + 1
        query = "update students set attempts = '"+str(attempts)+"' where uuid = '"+uuid+"'"
        cursorAuth.execute(query)
        query = "update students set codeSent = '"+"1"+"' where uuid = '"+uuid+"'"
        cursorAuth.execute(query)




        message = """From: UIUC Bleats <uiucbleats@gmail.com>
        To: Student <5158678777@pm.sprint.com>
        Subject: Your Code

        Your code is  
        """ + code
        server = smtplib.SMTP("smtp.gmail.com",587)
        server.starttls()
        server.login('uiucbleats','WirelessNetworks')
        server.sendmail('Your Professor','5158678777@pm.sprint.com',message)
        
        
        

#conn = sqlite3.connect('students.db')
#cursor = conn.cursor()
#connAuth = sqlite3.connect('AuthDB.db')
#cursorAuth = connAuth.cursor()


service = BeaconService("hci0")
#service = DiscoveryService("hci0")
while True:
    with sqlite3.connect('AuthDB.db') as connAuth:
        with sqlite3.connect('students.db') as conn:
            cursor = conn.cursor()
            cursorAuth = connAuth.cursor()
            print ("Start Scan")
            devices = service.scan(2)
            print ("Scanned")
            for address, data in list(devices.items()):
                b = Beacon(data, address)
                if (b._major==1):
                    cursorAuth.execute("SELECT * FROM students WHERE uuid = '" + b._uuid + "' LIMIT 1")
                    print (cursorAuth.fetchone())
                    cursorAuth.execute("select codeSent FROM students WHERE uuid = '"+b._uuid+"'")
                    codeSent = cursorAuth.fetchone()[0]

                    if (codeSent == 0):
                        print(b)
                        send(b._uuid,cursorAuth,time.time())
                elif (b._major==5):
                    correctCode, attempts =process_code(b._uuid,b._minor,cursorAuth,time.time()) 
                    if (correctCode):
                        print "The student is in class"
                        sys.exit()
                    else:
                        print "The student's code is false, boooooo"

            query = "select uuid,attempts,codeSent from students where attempts < 4"
            cursorAuth.execute(query)
            resendStudents = cursorAuth.fetchall()
            print resendStudents
            for student in resendStudents:
                uuid,attempts,codeSent = student[0],student[1],student[2]
                if (attempts < 3 and codeSent==0):
                    send(uuid,cursorAuth,time.time())
                elif (attempts > 3):
                    print "The student has not responded in 3 attempts or have 3 incorrect codes, they are not in class"
            time.sleep(5)

print("Done.")
