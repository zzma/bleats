from bluetooth.ble import BeaconService
import sys


service = BeaconService()

if (len(sys.argv)<2):
    print "Input code please"
else:
    code = sys.argv[1]
    service.start_advertising("11111111-2222-3333-4444-555555555555",5,code,1,200)
    time.sleep(15)
    service.stop_advertsing()
    print ("Done".)