from bluetooth.ble import BeaconService
import sqlite3
import sendCode
import time
import smtplib

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

conn = sqlite3.connect('students.db')
cursor = conn.cursor()
connAuth = sqlite3.connect('AuthDB.db')
cursorAuth = connAuth.cursor()

service = BeaconService("hci0")

while True:
    devices = service.scan(2)
    for address, data in list(devices.items()):
        b = Beacon(data, address)
        if (b._major==1):
            cursor.execute("SELECT * FROM students WHERE uuid = '" + b._uuid + "' LIMIT 1")
            print(cursor.fetchone())
            print(b)
            send(b._uuid,time.time())
        else:
            correctCode, attempts =process_code(b._uuid,b._minor,time.time()) 
            if (correctCode):
                print "The student is in class"
            else:
                print "The student's code is false, boooooo"

    query = "select uuid,attempts from students where attempts < 4"
    cursorAuth.execute(query)
    resendStudents = cursor.fetchall()
    for student in resendStudents:
        uuid,attempts = student[0],student[1]
        if (attempts < 3):
            send(uuid,time.time())
        else:
            print "The student has not responded in 3 attempts, they are not in class"


def process_code(uuid, code,time=0):
    strCode = str(code)
    query = "select count (*) from students where code='"+strCode+"' and uuid='"+uuid+"'"
    cursorAuth.execute(query)
    count = cursorAuth.fetchone()[0]
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
        return True,attempts
    else:
        return False,attempts


def send(uuid,time=0):
        code = str(randint(11111,65535))
        

    
        query = "update students set code='"+code+"' where uuid='"+uuid+"'"
        cursorAuth.execute(query)
        query = "update students set timeout='"+str(time)+"' where uuid='"+uuid+"'"
        cursorAuth.execute(query)

        query = "select attempts from students where code='"+strCode+"' and uuid='"+uuid+"'"
        cursorAuth.execute(query)
        attempts = cursorAuth.fetchone()[0] + 1
        query = "update students set attempts = '"+attempts+"' where uuid = '"+uuid+"'"
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
        
print("Done.")
