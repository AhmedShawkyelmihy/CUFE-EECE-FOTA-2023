import mcp2515_Register as reg
import mcp2515_driver as mcp


def reset() -> None:
    """
    Method name:    - reset
    Args:           - None.
    rtrVal:         - None.
    brief:          - This method is used to reset can module.
    """
    mcp.reset()

normal_mode			= 0x0
sleep_mode			= 0x1
loopback_mode		= 0x2
listen_only_mode	= 0x3
configuration_mode	= 0x4  # default mode

def start(can_mode: int) -> None:
    """
    Method name:    - start
    Args:           - [can_mode]: This is the mode required to work by can module.
    							  This must be a value of 5 values 
    							  normal_mode, sleep_mode, loopback_mode, listen_only_mode
    							  configuration_mode -> default mode.
    rtrVal:         - None.
    brief:          - This method is used to start can module working.
    """
    mcp.write_bytes(reg.CANCTRL, [(can_mode<<5) | (1<<3)])
    packet = mcp.read_bytes(reg.CANSTAT, 1)[0]
    while (((packet>>5) & 0x07) != can_mode):
	    packet = mcp.read_byte(reg.CANSTAT)


def init(SJW: int = 1, BRP: int = 1, PRSEG: int = 2, PHSEG1: int = 3, PHSEG2: int = 2) -> None:
    """
    Method name:    - init
    Args:           - [SJW]:	Synchronization Jump Width, must be a value from 1 to 4.
		      [BRP]:	Baud Rate Prescaler, must be a value from 1 to 64.
		      [PRSEG]:	Propagation Segment Length, must be a value from 1 to 8.
		      [PHSEG1]:	Phase Segment 1 Length, must be a value from 1 to 8.
		      [PHSEG2]:	Phase Segment 2 Length, must be a value from 1 to 8.
    rtrVal:         - None.
    brief:          - This method is used to initalize the bit rate of the can module
    					The default bit rate is 500kbps.
    """
    # One Shot Mode.
    mcp.write_bytes(reg.CANCTRL, [0x80])

    # Configure Bit Rate.
    CNF1_value = ((SJW - 1) << 6) | (BRP - 1)
    CNF2_value = (1 << 7) | ((PHSEG1 - 1) << 3) | ((PRSEG - 1) << 0)
    CNF3_value = (PHSEG2 - 1)
    packet = [CNF3_value, CNF2_value, CNF1_value]
    mcp.write_bytes(reg.CNF3, packet)


def init_CNF(CNF1_value: int = 0, CNF2_value: int = 0x91, CNF3_value: int = 0x01) -> None:
    """
    Method name:    - init
    Args:           - [CNF1_value]:	The value of the register CNF1 to configure BRP and SJW.
    				  [CNF2_value]:	The value of the register CNF2 to configure PRSEG and PHSEG1.
    				  [CNF3_value]:	The value of the register CNF3 to configure PHSEG2.
    rtrVal:         - None.
    brief:          - This method is used to initalize the bit rate of the can module
    					The default bit rate is 500kbps.
    """
    # One Shot Mode.
    mcp.write_bytes(reg.CANCTRL, [0x88])

    # Configure Bit Rate.
    packet = [CNF3_value, CNF2_value, CNF1_value]
    mcp.write_bytes(reg.CNF3, packet)

def get_empty_mailbox() -> int:
    """
    Method name:    - get_empty_mailbox
    Args:           - None.
    rtrVal:         - The index of the empty mailbox starting from 0 to 2 or None if there is no mailbox empty.
    brief:          - This method is used to get the index of the empty mailbox.
    """
    _byte = mcp.read_status()
    TXREQ0 = (_byte>>2) & 0x1
    if TXREQ0 == 0:
	    return 0

    TXREQ1 = (_byte>>4) & 0x1
    if TXREQ1 == 0:
	    return 1

    TXREQ2 = (_byte>>6) & 0x1
    if TXREQ2 == 0:
	    return 2

    return None



def transmit(mailbox_index: int,  STID: int, DLC: int, payload: list) -> None:
    """
    Method name:    - transmit
    Args:           - [mailbox_index]: The index of the empty mailbox.
		      [DLC]          : Data Length Counter is the length of payload.
		      [STID]	     : Standard Identifier.
		      [payload]	     : the actual payload max 8 bytes.
    rtrVal:         - The index of the empty mailbox starting from 0 to 2 or None if there is no mailbox empty.
    brief:          - This method is used to get the index of the empty mailbox.
    """
    packet = []
    packet.append(STID>>3)
    packet.append((STID & 0x7) << 5)
    packet.append(0)
    packet.append(0)
    packet.append(DLC)
    for _byte in payload[:DLC]:
	    packet.append(_byte)

    mcp.write_tx_buffer(mailbox_index, packet)
    #print(packet)
    mcp.request_to_send(mailbox_index)

def receive(FIFO_index: int) -> tuple:
    """
    Method name:    - receive
    Args:           - [FIFO_index]: the index of the receiption buffer to be read.
    rtrVal:         - the details of the frame (STID, DLC, payload).
    brief:          - This method is used to read the receiption buffer and return the detail of the received frame.
    """
    packet = mcp.read_rx_buffer(FIFO_index)

    STID		= (packet[0] << 3) | (packet[1] >> 5)
    DLC			= (packet[4] & 0xF)
    payload		= packet[5:]

    return STID, DLC, payload[:DLC]

def pending_message() -> int:
    """
    Method name:    - pending_message
    Args:           - [None]
    rtrVal:         - Integer number: 0 indecates there is no pending messages.
    				      1 indecates there is a pending message in receive buffer 0.
				      2 indecates there is a pending message in receive buffer 1.
				      3 indecates there is a pending message in both reveive buffers.
    brief:          - Integer number indicates that is any pending message in any reception buffer or not.
    """
    byte_status = mcp.rx_status()
    return (byte_status>>6) & 0x3


def interrupt_enable(MERRE: int = 0,  WAKIE: int = 0, ERRIE: int = 0, TX2IE: int = 0, TX1IE: int = 0, TX0IE: int = 0, RX1IE: int = 0, RX0IE: int = 0) -> None:
    """
    Method name:    - interrupt_enable
    Args:           - [MERRE]: Message Error Interrupt Enable bit.
		      [WAKIE]: Wake-up Interrupt Enable bit.
		      [ERRIE]: Error Interrupt Enable bit (multiple sources in EFLG register).
		      [TX2IE]: Transmit Buffer 2 Empty Interrupt Enable bit.
		      [TX1IE]: Transmit Buffer 1 Empty Interrupt Enable bit.
		      [TX0IE]: Transmit Buffer 0 Empty Interrupt Enable bit.
		      [RX1IE]: Receive Buffer 1 Full Interrupt Enable bit.
		      [RX0IE]: Receive Buffer 0 Full Interrupt Enable bit.
    rtrVal:         - None.
    brief:          - Enables the different interrupts.
    """
    _data = (MERRE << 7) | (WAKIE << 6) | (ERRIE << 5) | (TX2IE << 4) | (TX1IE << 3) | (TX0IE << 2) | (RX1IE << 1) | (RX0IE << 0)
    mcp.bit_modify(reg.CANINTE, 0xFF, _data)

def clear_INT_Flags(MERRE: int = 1,  WAKIE: int = 1, ERRIE: int = 1, TX2IE: int = 1, TX1IE: int = 1, TX0IE: int = 1, RX1IE: int = 1, RX0IE: int = 1) -> None:
    """
    Method name:    - clear_INT_Flags
    Args:           - [MERRE]: Message Error Interrupt Enable bit.
		      [WAKIE]: Wake-up Interrupt Enable bit.
		      [ERRIE]: Error Interrupt Enable bit (multiple sources in EFLG register).
		      [TX2IE]: Transmit Buffer 2 Empty Interrupt Enable bit.
		      [TX1IE]: Transmit Buffer 1 Empty Interrupt Enable bit.
		      [TX0IE]: Transmit Buffer 0 Empty Interrupt Enable bit.
		      [RX1IE]: Receive Buffer 1 Full Interrupt Enable bit.
		      [RX0IE]: Receive Buffer 0 Full Interrupt Enable bit.
    rtrVal:         - None.
    brief:          - Clear Interrupt Flags.
    """
    _data = (MERRE << 7) | (WAKIE << 6) | (ERRIE << 5) | (TX2IE << 4) | (TX1IE << 3) | (TX0IE << 2) | (RX1IE << 1) | (RX0IE << 0)
    _mask = ~_data
    mcp.bit_modify(reg.CANINTE, _mask, _data)


NO_INT 		= 0
ERROR_INT 	= 1
WAKEUP_INT 	= 2
TXB0_INT 	= 3
TXB1_INT 	= 4
TXB2_INT 	= 5
RXB0_INT 	= 6
RXB1_INT 	= 7



def read_INT_code() -> int:
    """
    Method name:    - read_INT_code
    Args:           - None.
    rtrVal:         - Interrupt Code.
    brief:          - It read The code of the happened Interrupt.
    """
    packet = read_bytes(reg.CANSTAT, 1)
    return (packet[0] >> 1) & 0x7
