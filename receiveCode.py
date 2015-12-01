import sqlite3
import sys

db_filename='AuthDB.db'

def process_code(uuid, code,time=0):
    with sqlite3.connect(db_filename) as conn:
        cursor = conn.cursor()
	strCode = str(code)
        query = "select count (*) from students where code='"+strCode+"' and uuid='"+uuid+"'"
        cursor.execute(query)
        count = cursor.fetchone()[0]
        if (count==1):
            query = "select timeout from students where code='"+strCode+"' and uuid='"+uuid+"'"
            cursor.execute(query)
            timeout = cursor.fetchone()[0]
            if (time-int(timeout)>60):
                return False
            return True
        else:
            return False
            

if __name__=='__main__':
    if len(sys.argv)>2:
        uuid = sys.argv[1]
        code = sys.argv[2]
        if (process_code(uuid,code)):
            print "You are in class!"
        else:
            print "Don't f with me brah"
        
    else:
        print "Please provide your uuid and the code sent to you"

        
