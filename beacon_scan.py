from bluetooth.ble import BeaconService
import sqlite3
import receiveCode
import sendCode

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

service = BeaconService("hci0")
devices = service.scan(2)

for address, data in list(devices.items()):
    b = Beacon(data, address)
    if (b._major==1):
        cursor.execute("SELECT * FROM students WHERE uuid = '" + b._uuid + "' LIMIT 1")
        print(cursor.fetchone())
        print(b)
        sendCode.send(b._uuid)
    else:
        if (receiveCode.process_code(b._uuid,b._minor)):
            print "The student is in class"
        else:
            print "The student's code is false, boooooo"

        
print("Done.")
