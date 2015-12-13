import smtplib

sender = 'uiucbleats@gmail.com'
receiver = 'rwking2@illinois.edu'

message = """From: UIUC Bleats <uiucbleats@gmail.com>
To: Student <rwking2@illinois.edu>
Subject: Your Dual Authentication

This is a test Authentication Message Yo
"""
username = 'uiucbleats@gmail.com'
password = 'WirelessNetworks'

server = smtplib.SMTP('smtp.gmail.com',587)
server.ehlo()
server.starttls()
server.login(username,password)
server.sendmail(sender, receiver, message)
server.close()

