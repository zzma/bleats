import smtplib

server = smtplib.SMTP("smtp.gmail.com",587)
server.starttls()
server.login('uiucbleats','WirelessNetworks')
server.sendmail('Your Professor','5158678777@pm.sprint.com','TestSMS')
