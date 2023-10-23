########################################################################
## IMPORTS THE EXTENSION LIBRARY
########################################################################

from PyQt5 import QtGui, QtWidgets, QtCore
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from PyQt5.uic import loadUi
import os
from os import path
import sys
import datetime


import datetime
import platform
import urllib.request

#data base
import sqlite3

import can
import Host_spi
from decrypt import decrypt
from time import sleep

########################################################################
# IMPORT GUI FILE
from GUI import *
########################################################################
import subprocess
########################################################################
# IMPORT Custom widgets
from Custom_Widgets.Widgets import *
########################################################################
#socket programming python
import socket
import threading

## MAIN WINDOW CLASS
#######################################################################
check = True
version_variable = "1.0.0"
Date = " "
Time = " "
ID = '1312000'
alert_buffer = ''
class Thread(QObject):
    update_progress = pyqtSignal(int)
    update_finished = pyqtSignal()
    update_unenable = pyqtSignal()
    show_message_box = pyqtSignal(str, str)
    Emergency_update = pyqtSignal(str)


    # in case of Emergency (company start update without ask the user )
    def server_program_Emergency(self):
        global Date
        global Time
        global version_variable # globalvariable to ghange in the current version
        # srart connection between client and server 
        # get the hostname
        #hostname = socket.gethostname()# Raspberry Pi IP address
        ip_address = '0.0.0.0'#socket.gethostbyname(hostname)# Raspberry Pi IP address
        port = 5005  # initiate port no above 1024
        try:
          # Create a socket object
          server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM) #socket.AF_INET for IPv4 addresses and socket.SOCK_STREAM for TCP protocol
          # Bind the socket to a specific host and port
          server_socket.bind((ip_address, port))
          # Listen for incoming connections
          server_socket.listen(10)
          while True:
               # Accept a client connection
               conn, address = server_socket.accept()
               self.Emergency_update.emit('Emergency Update send from company please wait until finish.....')
               self.update_unenable.emit()
               # Receive data from the client
               data = conn.recv(1024)
               if not data:
                   break
               # Process the received data
               received_data = data.decode()
               #download text file
               file_name ="version.txt"
               urllib.request.urlretrieve(received_data,file_name)
               #read from tet file and put it in list to handle information in it
               myfile= open("version.txt" ,"r")
               list = myfile.readlines()
               self.update_progress.emit(0)
               for version in list :
                   if (version[0] > version_variable[0]) or (version[2] > version_variable[2]) or (version[4] > version_variable[4]) :
                         version_variable[5] = version[:5]
               #Date and time
               DateTime = datetime.datetime.now()
               Date = "{}/{}/{}".format(DateTime.day, DateTime.month, DateTime.year)
               Time = "{:02d}:{:02d}:{:02d}".format(DateTime.hour, DateTime.minute, DateTime.second)
               urllib.request.urlretrieve(version[6:],'Application.bin.enc',self.Handle_Progress)
               
               if (os.path.exists('Application.bin.enc') == True):
                    Host_spi.BL_erase_sectors()
                    sleep(1)
                    Host_spi.BL_sent_bytes_v2('Application.bin')
                    sleep(1)
                    Host_spi.BL_jump_to_user_app()
                    self.show_message_box.emit('Update', 'Update from company is finished now')
                    
               else:
                  self.show_message_box.emit('Update', 'File does not exist')

               self.update_finished.emit()
               self.insert_data(Date,Time,version_variable)
               self.update_progress.emit(0)

               #Send a response back to the client
               response = ' Car_ID : '+ID+' has successfully updated chip 0x419 with version '+ version_variable
               data2=response.encode()
               conn.sendall(data2)

          # Close the client socket and server socket
          conn.close()
          server_socket.close()

        except Exception as e:
            print("An error occurred:", str(e))
            #self.show_message_box.emit('Error', "Error occurred: " + str(e))
        
    
     #prograss bar
    def Handle_Progress(self, block_num, block_size, total_size):
        percent = int(block_num * block_size * 100 / total_size)
        self.update_progress.emit(percent)

    
    # insert data in database 
    def insert_data(self,date,time,update_version):
        # Connect to the SQLite database
        conn = sqlite3.connect('information.db')
        cursor = conn.cursor()

        # Execute an INSERT statement to insert the data into the table
        cursor.execute("INSERT INTO update_history (Date,Time,update_version) VALUES (?,?,?)", (date,time,update_version))

        # Commit the changes and close the database connection
        conn.commit()
        conn.close()

    
    
class MainWindow(QMainWindow):
    def __init__(self, parent=None):
        QMainWindow.__init__(self)
        self.ui = Ui_MainWindow()
        loadUi("GUI.ui",self)
        self.ui.setupUi(self)
        ########################################################################
        # APPLY JSON STYLESHEET
        loadJsonStyle(self, self.ui)
        #######################################################################
        ########################################################################
        self.ui.progressBar.setValue(0)
        self.ui.lineEdit_8.setText("      New Update Available")
        self.load_data_from_database()
        self.Handel_Buttons()
        

        #handle calender
        self.ui.calendarWidget.selectionChanged.connect(self.calendarDateChanged)
       
        #handle lcd clock timer
        self.timer = QtCore.QTimer()
        self.timer.timeout.connect(self.updateLCD)
        self.timer.start(1000)

        self.thread = Thread()
        self.thread.update_progress.connect(self.update_progress_bar)
        self.thread.update_finished.connect(self.update_finished)
        self.thread.show_message_box.connect(self.show_message_box)
        self.thread.update_unenable.connect(self.update_unenable)
        self.thread.Emergency_update.connect(self.Emergency_update)
        

    def start_server_emergency(self):
        thread = threading.Thread(target=self.thread.server_program_Emergency)
        thread.start()

    def update_progress_bar(self, percent):
        self.ui.progressBar.setValue(percent)

    def update_finished(self):
        self.ui.update_button.setEnabled(True)
        #enable diagnostic button
        self.ui.pushButton_10.setEnabled(True)
        self.ui.lineEdit_8.clear()

    def update_unenable(self):
         self.ui.update_button.setEnabled(False)
         #unenable diagnostic button
         self.ui.pushButton_10.setEnabled(False)
         
    def show_message_box(self, title, message):
        QMessageBox.information(self, title, message)
    
    def Emergency_update(self,message):
        self.ui.lineEdit_8.setText(message)
         

    def updateLCD(self):
       self.currentTime = QtCore.QTime.currentTime()
       self.strCurrentTime = self.currentTime.toString('hh:mm')
       self.ui.lcdNumber.display(self.strCurrentTime)

    def calendarDateChanged(self):
        print("The calendar date was changed.")

    def Handel_Buttons(self):
        ## Handel All Buttons In Our App

        # EXPAND center menu WIDGET SIZE

        self.ui.settingBtn.clicked.connect(lambda: self.ui.centerMenuContanier.expandMenu())
        self.ui.informationBtn.clicked.connect(lambda: self.ui.centerMenuContanier.expandMenu())
        self.ui.helpBtn.clicked.connect(lambda: self.ui.centerMenuContanier.expandMenu())

        # close center menu WIDGET SIZE
        self.ui.closeCenterMenuBtn.clicked.connect(lambda: self.ui.centerMenuContanier.collapseMenu())

        # EXPAND right menu WIDGET SIZE

        self.ui.signupBtn.clicked.connect(lambda: self.ui.rightMenuContanier.expandMenu())
        self.ui.loginBtn.clicked.connect(lambda: self.ui.rightMenuContanier.expandMenu())

        # close right menu WIDGET SIZE
        self.ui.closeRightMenuBtn.clicked.connect(lambda: self.ui.rightMenuContanier.collapseMenu())

        # close notification WIDGET SIZE
        self.ui.closeNotificationBtn.clicked.connect(lambda: self.ui.NotificationContanier.collapseMenu())

        # login buttons 
        self.ui.pushButton_4.clicked.connect(lambda: self.login_Def())
        self.ui.pushButton_9.clicked.connect(lambda: self.ui.rightpages.setCurrentIndex(1))
        self.ui.line_edit_password.setEchoMode(QtWidgets.QLineEdit.Password)

        #register buttons
        self.ui.pushButton_2.clicked.connect(lambda: self.register_Def())
        self.ui.pushButton_3.clicked.connect(lambda: self.ui.rightpages.setCurrentIndex(0))
        self.ui.lineEdit_2.setEchoMode(QtWidgets.QLineEdit.Password)
        self.ui.lineEdit_3.setEchoMode(QtWidgets.QLineEdit.Password)

        #0pen tabs
        self.ui.pushButton_6.clicked.connect(self.History_Def) 
        self.ui.pushButton.clicked.connect(self.Update_Def)

        #diagnostics button
        self.ui.pushButton_10.clicked.connect(self.client_program_send_Diagnostic)

        # calculator connection
        self.ui.persentButton.clicked.connect(self.persent)
        self.ui.multiplyButton.clicked.connect(self.multiply)
        self.ui.divButton.clicked.connect(self.divide)
        self.ui.deleteButton.clicked.connect(self.delete)
        self.ui.sevenButton.clicked.connect(self.seven)
        self.ui.eightButton.clicked.connect(self.eight)
        self.ui.nineButton.clicked.connect(self.nine)
        self.ui.deleteButton.clicked.connect(self.minus)
        self.ui.fourButton.clicked.connect(self.four)
        self.ui.fiveButton.clicked.connect(self.five)
        self.ui.sixButton.clicked.connect(self.six)
        self.ui.plusButton.clicked.connect(self.plus)
        self.ui.threeButton.clicked.connect(self.three)
        self.ui.twoButton.clicked.connect(self.two)
        self.ui.oneButton.clicked.connect(self.one)
        self.ui.clearButton.clicked.connect(self.clear)
        self.ui.plusminusButton.clicked.connect(self.plusminus)
        self.ui.zeroButton.clicked.connect(self.zero)
        self.ui.dotButton.clicked.connect(self.dot)
        self.ui.equalButton.clicked.connect(self.equal)
  
    # login function
    def login_Def(self):
        username = self.ui.line_edit_username.text()
        password = self.ui.line_edit_password.text()

        # Connect to the SQLite database
        conn = sqlite3.connect('information.db')
        cursor = conn.cursor()

        # Execute a SELECT query to verify the username and password
        cursor.execute("SELECT * FROM username_password WHERE username=? AND password=?", (username, password))
        result = cursor.fetchone()
        if result:
            QMessageBox.information(self, "Login Successful", "Welcome, " + username + "!")
            # Proceed to the main application or perform other actions
        else:
            QMessageBox.warning(self, "Login Failed", "Invalid username or password. Please try again.")

        # Close the database connection
        conn.close()

        # Clear the line edits after login attempt
        self.ui.line_edit_username.clear()
        self.ui.line_edit_password.clear()
        

    # register function
    def register_Def(self):

        username = self.ui.lineEdit.text()
        password = self.ui.lineEdit_2.text()
        confirm_password = self.ui.lineEdit_3.text()

        if not username or not password or not confirm_password:
            QMessageBox.warning(self, "Sign Up Failed", "Please fill in all the fields.")
            return

        if password != confirm_password:
            QMessageBox.warning(self, "Sign Up Failed", "Passwords do not match.")
            return

        # Connect to the SQLite database
        conn = sqlite3.connect('information.db')
        cursor = conn.cursor()

        # Execute an INSERT query to store the user information
        try:
            cursor.execute("INSERT INTO username_password (username, password) VALUES (?, ?)", (username, password))
            conn.commit()
            QMessageBox.information(self, "Sign Up Successful", "User created successfully.")
        except sqlite3.Error as e:
            QMessageBox.warning(self, "Sign Up Failed", "Error occurred during sign-up: " + str(e))

        # Close the database connection
        conn.close()

        # Clear the line edits after sign-up
        self.ui.lineEdit.clear()
        self.ui.lineEdit_2.clear()
        self.ui.lineEdit_3.clear()

    
    #updates tab
    def Update_Def(self): 
      self.ui.tabWidget.setCurrentIndex(0)

   
    #history tab
    def History_Def(self): 
      self.ui.tabWidget.setCurrentIndex(1)

    
    # calculator functions
    def persent(self):
        global check
        self.ui.lineEdit_6.setText(self.ui.lineEdit_6.text() + "%")
        check = True

    def divide(self):
        global check
        self.ui.lineEdit_6.setText(self.ui.lineEdit_6.text() + "/")
        check = True

    def multiply(self):
        global check
        self.ui.lineEdit_6.setText(self.ui.lineEdit_6.text() + "x")
        check = True

    def delete(self):
        self.ui.lineEdit_6.setText(self.ui.lineEdit_6.text()[0:-1])

    def seven(self):
        global check
        if check:
            self.ui.lineEdit_6.setText(self.ui.lineEdit_6.text() + "7")
        if not check:
            self.ui.lineEdit_6.setText("7")


    def eight(self):
        global check
        if check:
           self.ui.lineEdit_6.setText(self.ui.lineEdit_6.text() + "8")
        if not check:
           self.ui.lineEdit_6.setText("8")


    def nine(self):
        global check
        if check:
            self.ui.lineEdit_6.setText(self.ui.lineEdit_6.text() + "9")
        if not check:
            self.ui.lineEdit_6.setText("9")


    def minus(self):
        global check
        self.ui.lineEdit_6.setText(self.ui.lineEdit_6.text() + "-")
        check = True

    def four(self):
        global check
        if check:
            self.ui.lineEdit_6.setText(self.ui.lineEdit_6.text() + "4")
        if not check:
            self.ui.lineEdit_6.setText("4")


    def five(self):
        global check
        if check:
            self.ui.lineEdit_6.setText(self.ui.lineEdit_6.text() + "5")
        if not check:
            self.ui.lineEdit_6.setText("5")


    def six(self):
        global check
        if check:
            self.ui.lineEdit_6.setText(self.ui.lineEdit_6.text() + "6")
        if not check:
            self.ui.lineEdit_6.setText("6")


    def plus(self):
        global check
        self.ui.lineEdit_6.setText(self.ui.lineEdit_6.text() + "+")
        check= True

    def three(self):
        global check
        if check:
            self.ui.lineEdit_6.setText(self.ui.lineEdit_6.text() + "3")
        if not check:
            self.ui.lineEdit_6.setText("3")


    def two(self):
        global check
        if check:
            self.ui.lineEdit_6.setText(self.ui.lineEdit_6.text() + "2")
        if not check:
            self.ui.lineEdit_6.setText("2")
    def one(self):
        global check
        if check:
            self.ui.lineEdit_6.setText(self.ui.lineEdit_6.text() + "1")
        if not check:
            self.ui.lineEdit_6.setText("1")


    def clear(self):
        self.ui.lineEdit_6.setText("")

    def plusminus(self):
        global check
        if check:
            self.ui.lineEdit_6.setText(self.ui.lineEdit_6.text() + "-")
        if not check:
            self.ui.lineEdit_6.setText("-")

    def zero(self):
        global check
        if check:
            self.ui.lineEdit_6.setText(self.ui.lineEdit_6.text() + "0")
        if not check:
            self.ui.lineEdit_6.setText("0")

    def dot(self):
        global check
        if check:
            self.ui.lineEdit_6.setText(self.ui.lineEdit_6.text() + ".")
        if not check:
            self.ui.lineEdit_6.setText(".")
        check = True
    def equal(self):
        global check
        try:
          self.ui.lineEdit_6.setText(str(eval(self.ui.lineEdit_6.text())))
        except ZeroDivisionError:
            self.ui.lineEdit_6.setText("cannot divide by zero ")
        except SyntaxError:
            self.ui.lineEdit_6.setText("Invalid Operation")
        except:
            print('Something strange has happened here... Sorry!')

        check = False

    # insert data in database 
    def insert_data(self,date,time,update_version):
        # Connect to the SQLite database
        conn = sqlite3.connect('information.db')
        cursor = conn.cursor()

        # Execute an INSERT statement to insert the data into the table
        cursor.execute("INSERT INTO update_history (Date,Time,update_version) VALUES (?,?,?)", (date,time,update_version))

        # Commit the changes and close the database connection
        conn.commit()
        conn.close()

    # table history 
    #read data from database 
    def load_data_from_database(self):
        global version_variable
        max_version = None 
        # Connect to the SQLite database
        conn = sqlite3.connect('information.db')
        cursor = conn.cursor()

        # Execute a SELECT query to fetch the data from the database
        cursor.execute("SELECT * FROM update_history")

        # Fetch all the rows from the result set
        rows = cursor.fetchall()

        # Set the number of rows and columns in the QTableWidget
        self.ui.tableWidget.setRowCount(len(rows))
        self.ui.tableWidget.setColumnCount(len(rows[0]))  # Assuming all rows have the same number of columns

        # Iterate over the rows and insert the data into the QTableWidget
        for i, row in enumerate(rows):
            for j, value in enumerate(row):
                item = QTableWidgetItem(str(value))
                self.ui.tableWidget.setItem(i, j, item)
                #Check if the value is greater than the current maximum version
            if max_version is None or row[-1] > max_version:
                    max_version = row[-1]

        # Close the database connection
        conn.close()

        # Update the version_variable with the maximum version
        if max_version is not None:
              version_variable = max_version

    def Handle_Progress(self,blocknum,blocksize,totalsize):
        read = blocknum*blocksize
        if totalsize > 0:
            progress = read*100/totalsize 
            percent = int(progress)  # Convert float progress to integer
            self.ui.progressBar.setValue(percent)
            QApplication.processEvents()
        else:
          # Set a minimum value for the progress bar when file size is unknown or zero
          min_value = 5  # Adjust this value as needed
          self.ui.progressBar.setValue(min_value)
          QApplication.processEvents()
    
    
    #update button function
    def client_program_show_new_update(self):
     global Date
     global Time
     global version_variable # globalvariable to ghange in the current vrsion
     self.load_data_from_database()
     # get the hostname
     #hostname = socket.gethostname()  # as both code is running on same pc
     ip_address ='' #socket.gethostbyname(hostname)# Raspberry Pi IP address
     port = 5900 # initiate port no above 1024
     # Create a socket object
     client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
     try:
       # Connect to the server
       client_socket.connect((ip_address, port))
       while True:
          # Send a request to the PC
          request = "user checks for new update now"
          data = request.encode()
          client_socket.sendall(data)
          # Receive the link from the PC
          link = client_socket.recv(1024)
          if not link:
            break
          # Process the received data
          received_data = link.decode()
          #download text file
          file_name ="version.txt"
          urllib.request.urlretrieve(received_data ,file_name)
          #read from tet file and putit in list to handle informationin it
          myfile= open("version.txt" ,"r")
          list = myfile.readlines()
          for version in list :
                if (version[0] > version_variable[0]) or (version[2] > version_variable[2]) or (version[4] > version_variable[4]) :
                   version_variable[5] = version[:5]
                   # Display a message box to confirm the update
                   msg = QMessageBox()
                   msg.setStandardButtons(QMessageBox.Yes | QMessageBox.No)
                   msg.setIcon(QMessageBox.Question)
                   msg.setText("are you sure that you want to download new version")
                   msg.setWindowTitle('update')
                   user_choice = msg.exec_()
                   # if yes start update application
                   if user_choice == QMessageBox.Yes:
                       self.ui.lineEdit_8.setText("         Update In Progress.....   ")
                       self.ui.update_button.setEnabled(False)
                       # Get the current date and time
                       DateTime = datetime.datetime.now()
                       Date = "{}/{}/{}".format(DateTime.day, DateTime.month, DateTime.year)
                       Time = "{:02d}:{:02d}:{:02d}".format(DateTime.hour, DateTime.minute, DateTime.second)
                       urllib.request.urlretrieve(version[6:],'Application.bin.enc',self.Handle_Progress)
                       if (os.path.exists('Application.bin.enc') == True):

                           decrypt('Application.bin.enc')
                          Host_spi.BL_erase_sectors()
                          sleep(1)
                          Host_spi.BL_sent_bytes_v2('Application.bin')
                          sleep(1)
                          Host_spi.BL_jump_to_user_app()
                          QMessageBox.information( self , 'Update' , 'Update is finished')

                       else:
                         QMessageBox.information( self , 'Update' , 'file does not exsits')

                       self.insert_data(Date,Time,version_variable)
                       self.ui.progressBar.setValue(0)
                       #Send a response back to the client
                       response = ' Car_ID : '+ID+' has successfully updated chip 0x419 with version '+ version_variable
                       data2=response.encode()
                       client_socket.sendall(data2)
                       self.load_data_from_database()
                       self.ui.update_button.setEnabled(True)
                       self.ui.lineEdit_8.clear()
                       break
                   else: 
                      self.ui.lineEdit_8.setText("          Update  is cancelled      ")
                      QMessageBox.information( self , 'Update' , 'you cancellled update')
                      break
                else:
                    QMessageBox.information( self , 'no update' , 'NO Avilable Update Now ')
                    break
                
       client_socket.close()

     except Exception as e:
        print("An error occurred:", str(e))
    
         
    def client_program_show_Diagnostic(self):
     global ID
     # get the hostname
     hostname = socket.gethostname()  # as both code is running on same pc
     ip_address = socket.gethostbyname(hostname)# Raspberry Pi IP address
     port = 5003  # initiate port no above 1024
     # Create a socket object
     client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
     try:
       # Connect to the server
       client_socket.connect((ip_address, port))
       while True:
          # Send problem to OEM
          alert = ''
          STID, DLC, alert_buffer = can.receive(0)
          print(STID, DLC, alert_buffer)
          if(alert_buffer[0] == 0x50):
            alert = ID + 'is facing a high radiator temperature'
            data = alert.encode()
            client_socket.sendall(data)
          client_socket.close()
     except Exception as e:
        print("An error occurred:", str(e))

    def client_program_send_Diagnostic(self):
     global ID
     # get the hostname
     hostname = socket.gethostname()  # as both code is running on same pc
     ip_address = socket.gethostbyname(hostname)# Raspberry Pi IP address
     port = 5003  # initiate port no above 1024
     # Create a socket object
     client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
     try:
       # Connect to the server
       client_socket.connect((ip_address, port))
       while True:
          # Send problem to OEM
          request = ID + self.ui.lineEdit_7.text()
          data = request.encode()
          client_socket.sendall(data)
          client_socket.close()
     except Exception as e:
        print("An error occurred:", str(e))

    def send_Dignostic(self):
          self.client_program_show_Diagnostic()

    def update_task(self) :
         self.ui.update_button.clicked.connect(self.client_program_show_new_update)

    
########################################################################
## EXECUTE APP
########################################################################
if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    # Create a thread for the update task
    update_thread = threading.Thread(target=window.update_task)
    # Start the thread
    update_thread.start()
    # Create a thread for the show_Dignostic
    Diagnostic_thread = threading.Thread(target=window.send_Dignostic)
    # Start the thread
    Diagnostic_thread.start()
    window.start_server_emergency()
    sys.exit(app.exec_())
########################################################################
## END===>
#####################################################################
