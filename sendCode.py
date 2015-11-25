import smtplib
from random import randint
import sqlite3

def send(uuid):
        code = str(randint(11111,65535))

        db_filename = 'AuthDB.db'

        with sqlite3.connect(db_filename) as conn:
                cursor = conn.cursor()
                query = "update students set code='"+code+"' where uuid='"+uuid+"'"
                cursor.execute(query) 


        message = """From: UIUC Bleats <uiucbleats@gmail.com>
        To: Student <5158678777@pm.sprint.com>
        Subject: Your Code

        Your code is  
        """ + code
        server = smtplib.SMTP("smtp.gmail.com",587)
        server.starttls()
        server.login('uiucbleats','WirelessNetworks')
        server.sendmail('Your Professor','5158678777@pm.sprint.com',message)

if __name__=='__main__':
        uuid = '11111111-2222-3333-4444-555555555555'
        send(uuid)
