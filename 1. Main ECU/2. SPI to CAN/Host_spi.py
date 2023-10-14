import os
import struct
import can
import mcp2515_driver as mcp
from time import sleep
from time import perf_counter


# =======================================================================
# ========================== BL Commands ================================
# =======================================================================

CBL_GET_VER_CMD             = 0x10
CBL_GET_HELP_CMD            = 0x11
CBL_GET_CID_CMD             = 0x12
CBL_GET_RDP_STATUS_CMD      = 0x13
CBL_GO_TO_ADDR_CMD          = 0x14
CBL_FLASH_ERASE_CMD         = 0x15
CBL_MEM_WRITE_CMD           = 0x16

CBL_ED_W_PROTECT_CMD        = 0x17
CBL_OTP_READ_CMD            = 0x20
CBL_CHANGE_ROP_Level_CMD    = 0x21
CBL_JUMP_TO_USER_APP_CMD    = 0x22
CBL_JUMP_TO_BOOTLOADER_CMD  = 0x23


INVALID_SECTOR_NUMBER        = 0x00
VALID_SECTOR_NUMBER          = 0x01
UNSUCCESSFUL_ERASE           = 0x02
SUCCESSFUL_ERASE             = 0x03

FLASH_PAYLOAD_WRITE_FAILED   = 0x00
FLASH_PAYLOAD_WRITE_PASSED   = 0x01


verbose_mode = 1
memory_write_active = 0

t_sleep = 0.02

# =======================================================================
# ================================ Utils ================================
# =======================================================================

def word_to_byte(word: int, index: int) -> int:
    """
    Method name:    - word_to_byte
    Args:           - [word]: This contains the word value.
                    - [index]: The byte number wanted to be returned starting from index 0.
    rtrVal:         - The indexed byte in the given word.
    brief:          - This method is used to get a certain byte in the word.
    """
    return (word >> (8 * index)) & 0xFF

class FileHandle:
    """
    A class to deal with binary files
    """
    def __init__(self, file_name: str):
        self.file_name = file_name
        self.file_object = open(self.file_name, 'rb')

    def cal_size(self):
        """
        Method name:    - cal_size
        Args:           - None.
        rtrVal:         - the calculated size of the file in bytes.
        breif:          - This method is used to calculate the size of a certain file. 
        """
        return os.path.getsize(self.file_name)

    def read_byte(self):
        """
        Method name:    - read_byte
        Args:           - None.
        rtrVal:         - One byte from the file.
        breif:          - This method is used to read only one byte from the file. 
        """
        return self.file_object.read(1)

    def write_byte(self, _byte):
        """
        Method name:    - write_byte
        Args:           - None.
        rtrVal:         - None.
        breif:          - This method is used to write only one byte to the file. 
        """
        self.file_object.write(_byte)

    def __del__(self):
        self.file_object.close()


def write_data_to_can(packet):

    index = can.get_empty_mailbox()
    if(index != None):
        can.transmit(index, 0x3FF, len(packet), packet)

def can_receive():
    received_message = can.pending_message()
    while received_message == 0:
        received_message = can.pending_message()

    if received_message == 1:
        STID, DLC, payload = can.receive(0)
    
    elif received_message == 2:
        STID, DLC, payload = can.receive(1)
    
    else:
        STID, DLC, payload = can.receive(0)
        STID, DLC, payload = can.receive(1)

    print("STID:", STID, " DLC: ", DLC, "payload: ", payload)
    # input("press any key to continue...")

    return payload

# =======================================================================
# =========================== BL Response ===============================
# =======================================================================



def Process_CBL_GET_VER_CMD():
    # Read From can
    Serial_Data = can_receive()
    _value = bytearray(Serial_Data)
    print("\n   Bootloader Vendor ID : ", _value[0])
    print("   Bootloader Version   : ", _value[1], ".", _value[2], ".", _value[3])

def Process_CBL_GET_CID_CMD():
    # Read From can
    Serial_Data = can_receive()
    CID = (Serial_Data[1] << 8) | Serial_Data[0]
    print("\n   Chip Identification Number : ", hex(CID))

def Process_CBL_GET_RDP_STATUS_CMD():
    # Read From can
    Serial_Data = can_receive()
    _value = bytearray(Serial_Data)
    if(_value[0] == 0xEE):
        print("\n   Error While Reading FLASH Protection level !!")
    elif(_value[0] == 0xAA):
        print("\n   FLASH Protection : LEVEL 0")
    elif(_value[0] == 0x55):
        print("\n   FLASH Protection : LEVEL 1")
    elif(_value[0] == 0xCC):
        print("\n   FLASH Protection : LEVEL 2")

def Process_CBL_GO_TO_ADDR_CMD():
    # Read From can
    Serial_Data = can_receive()
    _value = bytearray(Serial_Data)
    if(_value[0] == 1):
        print("\n   Address Status is Valid")
    else:
        print("\n   Address Status is InValid")

def Process_CBL_FLASH_ERASE_CMD():
    BL_Erase_Status = 0
    # Read From can
    Serial_Data = can_receive()
    if(len(Serial_Data)):
        BL_Erase_Status = bytearray(Serial_Data)
        if(BL_Erase_Status[0] == INVALID_SECTOR_NUMBER):
            print("\n   Erase Status -> Invalid Sector Number ")
        elif (BL_Erase_Status[0] == UNSUCCESSFUL_ERASE):
            print("\n   Erase Status -> Unsuccessfule Erase ")
        elif (BL_Erase_Status[0] == SUCCESSFUL_ERASE):
            print("\n   Erase Status -> Successfule Erase ")
        else:
            print("\n   Erase Status -> Unknown Error")
    else:
        print("Timeout !!, Bootloader is not responding")

def Process_CBL_MEM_WRITE_CMD():
    global Memory_Write_All
    BL_Write_Status = 0
    # Read From can
    Serial_Data = can_receive()
    BL_Write_Status = bytearray(Serial_Data)
    if(BL_Write_Status[0] == FLASH_PAYLOAD_WRITE_FAILED):
        print("\n   Write Status -> Write Failed or Invalid Address ")
    elif (BL_Write_Status[0] == FLASH_PAYLOAD_WRITE_PASSED):
        print("\n   Write Status -> Write Successfule ")
        Memory_Write_All = Memory_Write_All and FLASH_PAYLOAD_WRITE_PASSED
    else:
        print("Timeout !!, Bootloader is not responding")

def Process_CBL_CHANGE_ROP_Level_CMD():
    BL_CHANGE_ROP_Level_Status = 0
    # Read From can
    Serial_Data = can_receive()
    if(len(Serial_Data)):
        BL_CHANGE_ROP_Level_Status = bytearray(Serial_Data)
        if(BL_CHANGE_ROP_Level_Status[0] == 0x01):
            print("\n   ROP Level Changed")
        elif (BL_CHANGE_ROP_Level_Status[0] == 0x00):
            print("\n   ROP Level Not Changed ")
        else:
            print("\n   ROP Level -> Unknown Error")

def Process_CBL_JUMP_TO_USER_APP_CMD():
    # Read From can
    Serial_Data = can_receive()
    if(len(Serial_Data)):
        BL_JUMP_Status = bytearray(Serial_Data)
        if(BL_CHANGE_ROP_Level_Status[0] == 0x01):
            print("\n   Jumped to user app.\n")
        elif (BL_CHANGE_ROP_Level_Status[0] == 0x00):
            print("\n   Not Jumped to user app.\n")

def Process_CBL_JUMP_TO_BOOTLOADER_CMD():
    # Read From can
    Serial_Data = can_receive()
    if(len(Serial_Data)):
        BL_JUMP_Status = bytearray(Serial_Data)
        if(BL_CHANGE_ROP_Level_Status[0] == 0x01):
            print("\n   Jumped to bootloader.\n")
        elif (BL_CHANGE_ROP_Level_Status[0] == 0x00):
            print("\n   Not Jumped bootloader.\n")


# =======================================================================
# =========================== BL Methods ================================
# =======================================================================
def BL_version():
    BL_Buffer = []

    print("Read BootLoader Version.")
    BL_Buffer.append(CBL_GET_VER_CMD)

    # Send Data To BL
    print("Data sent: ", BL_Buffer)
    write_data_to_can(BL_Buffer)

    # Read Data from BL
    Process_CBL_GET_VER_CMD()

def BL_supported_CMD():
    BL_Buffer = []

    print("Read the commands supported by the bootloader")
    BL_Buffer.append(CBL_GET_HELP_CMD)

    # Send Data To BL
    write_data_to_can(BL_Buffer)

    # Read Data from BL
    Process_CBL_GET_HELP_CMD()

def BL_MCU_ID_number():
    BL_Buffer = []

    print("Read the MCU chip identification number")
    BL_Buffer.append(CBL_GET_CID_CMD)

    # Send Data To BL
    write_data_to_can(BL_Buffer)

    # Read Data from BL
    Process_CBL_GET_CID_CMD()

def BL_flash_read_protection_level():
    BL_Buffer = []

    BL_Buffer.append(CBL_GET_RDP_STATUS_CMD)

    # Send Data To BL
    write_data_to_can(BL_Buffer)

    # Read Data from BL
    Process_CBL_GET_RDP_STATUS_CMD()

def BL_go_to_add():
    BL_Buffer = []

    print("Jump bootloader to specified address command")

    CBL_Jump_Address = input("\n   Please Enter the Address in Hex : ")
    CBL_Jump_Address = int(CBL_Jump_Address, 16)

    BL_Buffer.append(CBL_GO_TO_ADDR_CMD)

    # Set Address
    BL_Buffer.append(word_to_byte(CBL_Jump_Address, 0))
    BL_Buffer.append(word_to_byte(CBL_Jump_Address, 1))
    BL_Buffer.append(word_to_byte(CBL_Jump_Address, 2))
    BL_Buffer.append(word_to_byte(CBL_Jump_Address, 3))

    # Send Data To BL
    write_data_to_can(BL_Buffer)
    
    # Read Data from BL
    Process_CBL_GO_TO_ADDR_CMD()

def BL_erase_sectors():
    BL_Buffer = []

    print("Mass erase or sector erase of the user flash command")

    sector_number = 0
    number_of_sectors = 0

    BL_Buffer.append(CBL_FLASH_ERASE_CMD)

    sector_number = input("\n   Please enter start sector number(0-11)          : ")
    # get the number of sector to erase
    sector_number = int(sector_number, 16)
    if(sector_number != 0xFF):
        number_of_sectors = int(input("\n   Please enter number of sectors to erase (12 Max): "), 16)


    # Set the sector number
    BL_Buffer.append(sector_number)
    BL_Buffer.append(number_of_sectors)

    # Send Data To BL
    write_data_to_can(BL_Buffer)

    # Read Data from BL
    Process_CBL_FLASH_ERASE_CMD()
    
def BL_sent_bytes(binary_file):
    print("Write data into different memories of the MCU command")
    global memory_write_is_active
    global memory_write_all
    global filter_flag
    
    memory_write_all = 1

    # Opening the file
    f = FileHandle(binary_file)

    file_total_len = f.cal_size()
    print("   Preparing writing a binary file with length (", file_total_len, ") Bytes")
    
    BL_Buffer = []
    
    BL_Buffer.append(CBL_MEM_WRITE_CMD)
    
    base_memory_address = input("\n   Enter the start address : ")
    base_memory_address = int(base_memory_address, 16)
    
    BL_Buffer.append(word_to_byte(base_memory_address, 0))
    BL_Buffer.append(word_to_byte(base_memory_address, 1))
    BL_Buffer.append(word_to_byte(base_memory_address, 2))
    BL_Buffer.append(word_to_byte(base_memory_address, 3))
    
    BL_Buffer.append(word_to_byte(file_total_len, 0))
    BL_Buffer.append(word_to_byte(file_total_len, 1))
    
    write_data_to_can(BL_Buffer)
    
    sleep(t_sleep)
    
    number_of_completed_8frame = file_total_len // 8
    number_of_remaining_8bytes = file_total_len % 8
    
    number_of_bytes_sent = 0
    
    start = perf_counter()
    for i in range(number_of_completed_8frame):
        BL_Buffer = []
        
        for j in range(8):
            bin_file_byte_value = f.read_byte()
            bin_file_byte_value = bytearray(bin_file_byte_value)
            BL_Buffer.append(int(bin_file_byte_value[0]))
            
        write_data_to_can(BL_Buffer)
        
        number_of_bytes_sent += 8
        print("\n   Bytes sent to the bootloader :{0}".format(number_of_bytes_sent))
        
        if(i % 2 == 0):
            sleep(t_sleep)
            
        #if(number_of_bytes_sent%128 == 0):
            #can_receive()
         
    
    if(number_of_remaining_8bytes != 0):
        sleep(t_sleep)
        BL_Buffer = []
        for j in range(number_of_remaining_8bytes):
            bin_file_byte_value = f.read_byte()
            bin_file_byte_value = bytearray(bin_file_byte_value)
            BL_Buffer.append(int(bin_file_byte_value[0]))
            
        write_data_to_can(BL_Buffer)
    
        number_of_bytes_sent += number_of_remaining_8bytes
        print("\n   Bytes sent to the bootloader :{0}".format(number_of_bytes_sent))
        
    end = perf_counter()
    
    elapsed_time_ms = (end - start) #* 10**3
    
    memory_write_is_active = 0
    if(memory_write_all == 1):
        print("\n\n Payload Written Successfully in ", elapsed_time_ms, "s.\n\n")

def BL_read_protection_level():
    print("Change read protection level of user flash.")

    BL_Buffer = []

    # Command Code
    BL_Buffer.append(CBL_CHANGE_ROP_Level_CMD)

    # get protection level
    Protection_level = input("\n   Please Enter one of these Protection levels : 0,1,2 : ")
    Protection_level = int(Protection_level, 8)

    if(Protection_level == 2):
            print("\n   Protection level (2) not supported !!")

    elif (Protection_level == 0 or Protection_level == 1):
        print("\n   Changing the protection level to be : ", Protection_level)
        BL_Buffer.append(protection_mode)

        # write to can
        write_data_to_can(BL_Buffer)

        # read bootloader response
        Process_CBL_CHANGE_ROP_Level_CMD()

    else:
        print("\n   Protection level (", Protection_level, ") not supported !!")

    
def BL_jump_to_user_app():
    print("Jump to user application.")

    BL_Buffer = []

    # Command Code
    BL_Buffer.append(CBL_JUMP_TO_USER_APP_CMD)

    # write to can
    write_data_to_can(BL_Buffer)

    # read bootloader response
    Process_CBL_JUMP_TO_USER_APP_CMD()

def BL_jump_to_bootloader_app():
    print("Jump to bootloader.")

    BL_Buffer = []

    # Command Code
    BL_Buffer.append(CBL_JUMP_TO_BOOTLOADER_CMD)

    # write to can
    write_data_to_can(BL_Buffer)

    # read bootloader response
    Process_CBL_JUMP_TO_USER_APP_CMD()    







def Decode_CBL_Command(Command):

    if(Command == 1):
        BL_version()

    elif(Command == 2):
        BL_MCU_ID_number()

    elif(Command == 3):
        BL_flash_read_protection_level()

    elif(Command == 4):
        BL_go_to_add()

    elif(Command == 5):
        BL_erase_sectors()

    elif(Command == 6):
        #BL_send_bytes("Application.bin")
        BL_sent_bytes_v2("Application.bin")
        
    elif(Command == 7):
        BL_jump_to_user_app()

    elif(Command == 8):
        BL_jump_to__app()


def can_init():
    can.reset()
    can.init_CNF(0x40, 0xB5, 0x01) # 250kbps
    ph = mcp.read_bytes(0x28, 3)
    packet = [hex(elem) for elem in ph]
    print(packet)
    can.start(can.normal_mode)

can_init()

while True:
    print("\nSTM32F429 Custome BootLoader")
    print("==============================")
    print("Which command you need to send to the bootLoader :");
    print("   CBL_GET_VER_CMD              --> 1")
    print("   CBL_GET_CID_CMD              --> 2")
    print("   CBL_GET_RDP_STATUS_CMD       --> 3")
    print("   CBL_GO_TO_ADDR_CMD           --> 4")
    print("   CBL_FLASH_ERASE_CMD          --> 5")
    print("   CBL_MEM_WRITE_CMD            --> 6")
    print("   CBL_JUMP_TO_USER_APP_CMD     --> 7")
    print("   CBL_JUMP_TO_BOOTLOADER_CMD   --> 8")
        
    CBL_Command = input("\nEnter the command code : ")
    
    if(not CBL_Command.isdigit()):
        print("   Error !!, Please enter a valid command !! \n")
    else:
        Decode_CBL_Command(int(CBL_Command))
    
    input("\nPlease press any key to continue ...")
    
