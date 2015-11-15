import sqlite3
import sys

db_filename='AuthDB.db'

def process_code(uuid, code):
    with sqlite3.connect(db_filename) as conn:
        cursor = conn.cursor()

        query = "select count (*) from students where code='"+code+"' and uuid='"+uuid+"'"
        cursor.execute(query)
        count = cursor.fetchone()[0]
    
        if (count==1):
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

        