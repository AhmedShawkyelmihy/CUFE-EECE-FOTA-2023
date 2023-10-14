import mcp2515_Register as mcpReg
import spidev

# /dev/spidev(bus).(device)
# /dev/spidev0.0  /dev/spidev0.1

bus 	= 0
device 	= 0

spi = spidev.SpiDev()
spi.open(bus, device)

# mcp2515 max speed is 10MHz
spi.max_speed_hz = 10000
spi.mode 		 = 0


def reset() -> None:
    """
    Method name:    - reset
    Args:           - None.
    rtrVal:         - None.
    brief:          - This method is used to reset the controller mcp2515 to reinitialize it.
    """
    spi.writebytes2([mcpReg.reset_instruction])


def read_bytes(address: int, num_of_bytes: int) -> tuple:
    """
    Method name:    - read_bytes
    Args:           - [address]: This is start address of the register to be read.
                    - [num_of_bytes]: The number of bytes to be read.
    rtrVal:         - The bytes read.
    brief:          - This method is used to read a register from the mpc2515 
    					or multiple registers sequentially specefied by num_of_bytes.
    """
    packet 	= [i*0 for i in range(2 + num_of_bytes)]
    packet[0] 	= mcpReg.read_instrucion 
    packet[1] 	= address

    packet	= spi.xfer3(packet)
    return packet[2:]

def read_rx_buffer(rx_buffer_number: int) -> tuple:
    """
    Method name:    - read_rx_buffer
    Args:           - [rx_buffer_number]: This is index of the receiption buffer
    					There are only 2 buffer index 0 and 1.
    rtrVal:         - The bytes read.
    brief:          - This method is used to read the receiption buffer
    """
    packet = [i*0 for i in range(13)]
    packet.insert(0, mcpReg.read_rx_buffer_instrucion | (rx_buffer_number<<2))
    packet		= spi.xfer3(packet)

    return packet[1:]

def write_bytes(address: int, _bytes: list) -> None:
    """
    Method name:    - write_bytes
    Args:           - [address]: This is start address of the register to write the bytes.
                    - [_bytes]: The needed bytes to be written.
    rtrVal:         - None.
    brief:          - This method is used to write a byte to a specefic register of the mpc2515 
    					or multiple registers sequentially if _bytes is list.
    """
    packet	= [i*0 for i in range(2)]
    packet[0] 	= mcpReg.write_instruction 
    packet[1] 	= address

    for _byte in _bytes:
	    packet.append(_byte)

    packet	= spi.xfer3(packet)

	
def write_tx_buffer(tx_mailbox_number: int, packet: list) -> None:
    """
    Method name:    - write_tx_buffer
    Args:           - [tx_mailbox_number]: This is the mailbox index.
    					There are 3 mailbox index of 0, 1, 2.
                    - [packet]: The needed bytes to be written.
    rtrVal:         - None.
    brief:          - This method is used to write a byte to the mailbox.
    """
    packet.insert(0, mcpReg.load_tx_buffer_instruction | (tx_mailbox_number<<1))
    spi.writebytes2(packet)


def request_to_send(tx_mailbox_number: int) -> None:
    """
    Method name:    - request_to_send
    Args:           - [tx_mailbox_number]: This is the mailbox index.
    					There are 3 mailbox index of 0, 1, 2.
    rtrVal:         - None.
    brief:          - This method is used to initiate the a transaction on the can bus.
    """
    packet = [mcpReg.request_to_send_instruction | (1<<tx_mailbox_number)]
    spi.writebytes2(packet)

def bit_modify(address: int, mask: int, data: int) -> None:
    """
    Method name:    - bit_modify
    Args:           - [address]: This is start address of the register to write the bytes.
		      [mask]   : This contains the bits index to modify.
		      [data]   : This is the actual data to be written to the register.
    rtrVal:         - None.
    brief:          - This method is used to modify specific bits in a register.
    """
    packet = [bit_modify_instruction, address, mask, data]
    spi.writebytes2(packet)


def read_status() -> int:
    """
    Method name:    - read_status
    Args:           - None.
    rtrVal:         - one byte: TX2IF TXREQ TX1IF TXREQ TX0IF TXREQ RX1IF RX0IF.
    brief:          - This method is used to read the status of the mailbox and receiption buffer.
    """
    packet = [mcpReg.read_status_instruction, 0x0]
    packet = spi.xfer3(packet)
    return packet[1]

def rx_status() -> int:
    """
    Method name:    - rx_status
    Args:           - None.	
    rtrVal:         - one byte: refer to datasheet for details page number 70.			
    brief:          - This method is used to read the status of the receiption.
    """
    packet = [mcpReg.rx_status_instrucion, 0x0]
    packet = spi.xfer3(packet)
    return packet[1]
