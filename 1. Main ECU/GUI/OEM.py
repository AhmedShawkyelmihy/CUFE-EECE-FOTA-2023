from PyQt5.QtWidgets import QApplication, QMainWindow, QWidget,QMessageBox, QVBoxLayout, QLabel, QLineEdit, QPushButton, QTableWidget,QTabWidget,QGridLayout,QTableWidgetItem
from PyQt5.QtGui import QColor, QPalette , QFont
import sys
import socket
import sqlite3
import threading
from PyQt5.QtCore import QMetaType



class CompanyApp(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Company Application")
        self.setGeometry(50, 50, 1000, 700)


         # Set the window background color to black
        palette = self.palette()
        palette.setColor(QPalette.Window, QColor(25, 35, 45))
        self.setPalette(palette)

        self.tab_widget = QTabWidget(self)
        self.tab_widget.setGeometry(50, 50, 900, 600)

         # Set the tab_widget color to blue
        self.tab_widget.setStyleSheet("background-color: rgb(25, 35, 45);")

        #login page
        self.tab_login = QWidget()
        self.tab_widget.addTab(self.tab_login, "login")
        self.label_username = QLabel("Username:", self)
        self.label_username.setGeometry(80, 30, 80, 20)
        self.label_username.setStyleSheet("color: rgb(73, 143, 204);")
        self.line_edit_username = QLineEdit(self)
        self.line_edit_username.setFixedSize(400, 60)
        self.label_password = QLabel("Password:", self)
        self.label_password.setGeometry(80, 60, 80, 20)
        self.label_password.setStyleSheet("color: rgb(73, 143, 204);")
        self.line_edit_password = QLineEdit(self)
        self.line_edit_password.setFixedSize(400, 60)
        self.line_edit_password.setEchoMode(QLineEdit.Password)
        self.button_login = QPushButton("Login", self)
        self.button_login.setFixedSize(400, 60)
        self.button_login.setStyleSheet("background-color: rgb(73, 143, 204);")
        
        # Set the font size of the labels
        font = QFont()
        font.setPointSize(16)
        self.label_username.setFont(font)
        self.label_password.setFont(font)
        self.button_login.setFont(font)
        # Create a grid layout
        self.layout_login = QGridLayout()
        self.tab_login.setLayout(self.layout_login)
        self.layout_login.addWidget( self.label_username,1,1)
        self.layout_login.addWidget(self.line_edit_username,1,2)
        self.layout_login.addWidget(self.label_password,2,1)
        self.layout_login.addWidget(self.line_edit_password,2,2)
        self.layout_login.addWidget(self.button_login,3,2)


        #Emergency Update page
        self.tab_emergency = QWidget()
        self.tab_widget.addTab(self.tab_emergency, "Emergency Update")
        self.line_edit1 = QLineEdit(self)
        self.line_edit3 = QLineEdit(self)
        self.line_edit1.setStyleSheet("color: rgb(73, 143, 204);")
        self.line_edit3.setStyleSheet("color: rgb(73, 143, 204);")
        self.button1 = QPushButton("send emergence update", self)
        self.label_link = QLabel("put the link here:", self)
        self.label_link.setStyleSheet("color: rgb(73, 143, 204);")
        self.label_link.setFont(font)
        self.label_response = QLabel("response from user:", self)
        self.label_response.setStyleSheet("color: rgb(73, 143, 204);")
        self.label_response.setFont(font)
        # Set the size of the buttons
        self.button1.setFixedSize(850, 40)
        # Set the size and font of line edits
        font.setPointSize(12)
        self.line_edit1.setFixedSize(600, 40)
        self.line_edit1.setFont(font)
        self.line_edit3.setFixedSize(600, 40)
        self.line_edit3.setFont(font)
        # Set the button color to blue
        self.button1.setStyleSheet("background-color: rgb(73, 143, 204);")
        self.button1.setFont(font)
        # Create a grid layout
        self.layout_emergency = QGridLayout()
        self.tab_emergency.setLayout(self.layout_emergency)
        self.layout_emergency.addWidget(self.label_link,0,0)
        self.layout_emergency.addWidget(self.line_edit1,0,1)
        self.layout_emergency.addWidget(self.label_response,1,0)
        self.layout_emergency.addWidget(self.line_edit3,1,1)
        self.layout_emergency.addWidget(self.button1,2,0,1,0)
        
        #Accept Update page
        self.tab_accept = QWidget()
        self.tab_widget.addTab(self.tab_accept, "Accept Update")
        self.line_edit2 = QLineEdit(self)
        font.setPointSize(14)
        self.line_edit2.setFont(font)
        self.label_request = QLabel(" user response:", self)
        self.label_request.setStyleSheet("color: rgb(73, 143, 204);")
        self.label_request.setFont(font)
        # Set the size of line edits
        self.line_edit2.setFixedSize(850, 60)
        self.line_edit2.setStyleSheet("color: rgb(73, 143, 204);")
        self.layout_accept = QGridLayout()
        self.tab_accept.setLayout(self.layout_accept)
        self.layout_accept.addWidget(self.label_request,1,0)
        self.layout_accept.addWidget(self.line_edit2,1,0,2,0)
       
        
        
        #Diagnostics page
        #create table 
        self.table_widget = QTableWidget()
        self.table_widget.setColumnCount(2)
        self.table_widget.setHorizontalHeaderLabels(["ID", "Problem"])
        # Set the table to have an infinite number of rows
        self.table_widget.setRowCount(0)
        # Create the layout for the diagnostics page
        self.layout_diagnostics = QVBoxLayout()
        self.layout_diagnostics.addWidget(self.table_widget)

        # Create the diagnostics page
        self.tab_diagnostics = QWidget()
        self.tab_diagnostics.setLayout(self.layout_diagnostics)
        self.tab_widget.addTab(self.tab_diagnostics, "Diagnostics")

        #load data from database
        self.load_data_from_database() 
        self.Handel_Buttons()
        

    def Handel_Buttons(self):
        ## Handel All Buttons In Our App
        self.button_login.clicked.connect(self.login)
        self.button1.clicked.connect(self.client_program_Emergency)
        
        
    def login(self):
        username = self.line_edit_username.text()
        password = self.line_edit_password.text()

        # Connect to the SQLite database
        conn = sqlite3.connect('information.db')
        cursor = conn.cursor()

        # Execute a SELECT query to verify the username and password
        cursor.execute("SELECT * FROM company_informaton WHERE username=? AND password=?", (username, password))
        result = cursor.fetchone()

        if result:
            QMessageBox.information(self, "Login Successful", "Welcome, " + username + "!")
    
        else:
            QMessageBox.warning(self, "Login Failed", "Invalid username or password. Please try again.")

        # Close the database connection
        conn.close()

        # Clear the line edits after login attempt
        self.line_edit_username.clear()
        self.line_edit_password.clear()


     # for emergence case 
    def client_program_Emergency(self):
       # get the hostname
       hostname = socket.gethostname()  # as both code is running on same pc
       ip_address =socket.gethostbyname(hostname)# Raspberry Pi IP address
       port = 5005  # initiate port no above 1024
       try:
         # Create a socket object
         client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
         # Connect to the server
         client_socket.connect((ip_address, port))
         while True:
            # Prompt the user to enter the link or retrieve it from a source
            link = self.line_edit1.text()
        

            # Convert the link to bytes using UTF-8 encoding
            link_bytes = link.encode('utf-8')

            # Send the encoded link data to the server
            client_socket.sendall(link_bytes)

            # Receive the response from the server
            data = client_socket.recv(1024)
            if not data:
               break

            # Process the received data
            received_data = data.decode()
            print('Server response:', received_data)
            self.line_edit3.setText(received_data)
            client_socket.close()

       except Exception as e:
           print("Error occurred:", str(e))
           

    # for general case 
    def show_new_update(self):
        # Server configuration
       # get the hostname
       #hostname = socket.gethostname()    # as both code is running on same pc
       ip_address ='0.0.0.0' #socket.gethostbyname(hostname)# Raspberry Pi IP address
       port = 5900 # initiate port no above 1024
       # Create a socket object
       server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
       # Bind the socket to a specific address and port
       server_socket.bind((ip_address, port))
       # Listen for incoming connections
       server_socket.listen(10)
       try:
        while True:
         # Accept a client connection from Raspberry Pi
         conn,address = server_socket.accept()
         # Receive data from the Raspberry Pi
         data1 = conn.recv(1024)
         if not data1 :
             break
         received_data1 = data1.decode()
         self.line_edit2.setText(received_data1)
         
         # Get the link from a source (e.g., QLineEdit)
         link = 'https://drive.google.com/uc?export=download&id=11YKb9qwwa7lo7-YP1zH9fMASbG8YEH5e'

         # Convert the link to bytes using UTF-8 encoding
         link_bytes = link.encode('utf-8')

         # Send the encoded link data to the client
         conn.sendall(link_bytes)

         # Receive the response from the server
         data2 = conn.recv(1024)
         if not data2:
            break
         # Process the received data
         received_data2 = data2.decode()
         self.line_edit2.setText(received_data2)
         conn.close()
       except Exception as e:
            print('Error occurred:', str(e))
       finally:
        server_socket.close()
    

    # insert data in database 
    def insert_data(self,id,problem):
        # Connect to the SQLite database
        conn = sqlite3.connect('information.db')
        cursor = conn.cursor()

        # Execute an INSERT statement to insert the data into the table
        cursor.execute("INSERT INTO Diagnostic_update (ID,user_problem) VALUES (?,?)", (id,problem))

        # Commit the changes and close the database connection
        conn.commit()
        conn.close()

    
    #read data from database 
    def load_data_from_database(self):
        # Connect to the SQLite database
        conn = sqlite3.connect('information.db')
        cursor = conn.cursor()

        # Execute a SELECT query to fetch the data from the database
        cursor.execute("SELECT * FROM Diagnostic_update")

        # Fetch all the rows from the result set
        rows = cursor.fetchall()

        # Set the number of rows and columns in the QTableWidget
        self.table_widget.setRowCount(len(rows))
        self.table_widget.setColumnCount(len(rows[0]))  # Assuming all rows have the same number of columns

        # Iterate over the rows and insert the data into the QTableWidget
        for i, row in enumerate(rows):
            for j, value in enumerate(row):
                item = QTableWidgetItem(str(value))
                # Set the text color for the item
                color = QColor(73, 143, 204) # Red color
                item.setForeground(color)  # Set the text color
                # Set the item in the table widget
                self.table_widget.setItem(i, j, item)

        # Close the database connection
        conn.close()

        
    # for diagnostic case 
    def start_server(self):
        # Server configuration
       # get the hostname
      # hostname = socket.gethostname()    # as both code is running on same pc
       ip_address ='0.0.0.0' #socket.gethostbyname(hostname)# Raspberry Pi IP address
       port = 5003  # initiate port no above 1024
       # Create a socket object
       server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
       # Bind the socket to a specific address and port
       server_socket.bind((ip_address, port))
       # Listen for incoming connections
       server_socket.listen(2)

       while True:
         # Accept a client connection from Raspberry Pi
         client_socket,address = server_socket.accept()

         # Start a new thread to handle the client communication
         thread = threading.Thread(target=self.handle_client, args=(client_socket,))
         thread.start()


    def handle_client(self, client_socket):
       try:
         while True:
            # Receive data from the Raspberry Pi
            data = client_socket.recv(1024)
            if not data :
                break
            received_data = data.decode()
            self.insert_data(received_data[0:7],received_data[7:])
            self.load_data_from_database()
            client_socket.close()

       except Exception as e:
            print('Error occurred:', str(e))
       

    def connectToClient1(self):
      self.show_new_update()

    def connectToClient2(self):
      self.start_server()

if __name__ == "__main__":
   app = QApplication(sys.argv)
   company_app = CompanyApp()
   company_app.show()
   thread1 = threading.Thread(target=company_app.connectToClient1)
   thread1.start()
   thread2 = threading.Thread(target=company_app.connectToClient2)
   thread2.start() 
   sys.exit(app.exec_())