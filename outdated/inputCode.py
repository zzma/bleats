import subprocess
import sys



if __name__=='__main__':

    if (len(sys.argv)<2):
        print "Input code please"
    else:
        code = sys.argv[1]
        HOST = "Bob-Ubuntu14"
        COMMAND = "python cs439/bleats/receiveCode.py 11111111-2222-3333-4444-555555555555 " + code
        ssh = subprocess.Popen(["ssh","%s" % HOST, COMMAND], shell=False, stdout=subprocess.PIPE,stderr = subprocess.PIPE)
        result = ssh.stdout.readlines()
        if result ==[]:
            error = ssh.stderr.readlines()
            print >>sys.stderr, "ERROR: %s" % error
        else:
            print result
