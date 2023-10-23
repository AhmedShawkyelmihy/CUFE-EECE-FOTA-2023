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


INVALID_SECTOR_NUMBER        = 0x00
VALID_SECTOR_NUMBER          = 0x01
UNSUCCESSFUL_ERASE           = 0x02
SUCCESSFUL_ERASE             = 0x03

FLASH_PAYLOAD_WRITE_FAILED   = 0x00
FLASH_PAYLOAD_WRITE_PASSED   = 0x01


verbose_mode = 1
memory_write_active = 0

t_sleep = 0.045

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
    # can_receive()
    Process_CBL_GET_VER_CMD()

def BL_erase_sectors():
    BL_Buffer = []

    print("Mass erase or sector erase of the user flash command")

    sector_number = 0
    number_of_sectors = 0

    BL_Buffer.append(CBL_FLASH_ERASE_CMD)

    # get the number of sector to erase
    sector_number = 2 # int(sector_number, 16)
    
    number_of_sectors = 2
    # Set the sector number
    BL_Buffer.append(sector_number)
    BL_Buffer.append(number_of_sectors)

    # Send Data To BL
    write_data_to_can(BL_Buffer)




    
def BL_sent_bytes_v2(binary_file):
    print("Write data into different memories of the MCU command")
    global memory_write_is_active
    global memory_write_all
    global filter_flag
    
    memory_write_all = 1

    # Opening the file
    f = FileHandle(binary_file)

    file_total_len = f.cal_size()
    print("   Preparing writing a binary file with length (", file_total_len, ") Bytes")
    # input("press any key to continue...")
    
    BL_Buffer = []
    
    BL_Buffer.append(CBL_MEM_WRITE_CMD)
    
    # base_memory_address = input("\n   Enter the start address : ")
    base_memory_address = 0x8008000 # int(base_memory_address, 16)
    
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
        
    




    
def BL_jump_to_user_app():
    print("Jump to user application.")

    BL_Buffer = []

    # Command Code
    BL_Buffer.append(CBL_JUMP_TO_USER_APP_CMD)

    # write to can
    write_data_to_can(BL_Buffer)

    # read bootloader response
    # can_receive()


def can_init():
    can.reset()
    can.init_CNF(0x40, 0xB5, 0x01) # 250kbps
    can.start(can.normal_mode)

can_init()

