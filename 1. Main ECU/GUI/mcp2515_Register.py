# ===============================================
# ========= Transmission Registers ==============
# ===============================================

# TRANSMIT BUFFER n CONTROL REGISTER
TXB0CTRL	= 0x30
TXB1CTRL	= 0x40
TXB2CTRL	= 0x50

# (TXnRTS)' PIN CONTROL AND STATUS REGISTER
TXRTSCTRL	= 0x0D

# TRANSMIT BUFFER n STANDARD IDENTIFIER REGISTER HIGH
TXB0SIDH	= 0x31
TXB1SIDH	= 0x41
TXB2SIDH	= 0x51

# TRANSMIT BUFFER n STANDARD IDENTIFIER REGISTER LOW
TXB0SIDL	= 0x32
TXB1SIDL	= 0x42
TXB2SIDL	= 0x52

# TRANSMIT BUFFER n EXTENDED IDENTIFIER 8 REGISTER HIGH
TXB0EID8	= 0x33
TXB1EID8	= 0x43
TXB2EID8	= 0x53

# TRANSMIT BUFFER n EXTENDED IDENTIFIER 0 REGISTER LOW
TXB0EID0	= 0x34
TXB1EID0	= 0x44
TXB2EID0	= 0x54

# TRANSMIT BUFFER n DATA LENGTH CODE REGISTER
TXb0DLC		= 0x35
TXb1DLC		= 0x45
TXb2DLC		= 0x55

# TRANSMIT BUFFER 0 DATA BYTE m REGISTER
TXB0D0 		= 0x36
TXB0D1 		= 0x37
TXB0D2 		= 0x38
TXB0D3 		= 0x39
TXB0D4 		= 0x3A
TXB0D5 		= 0x3B
TXB0D6 		= 0x3C
TXB0D7 		= 0x3D

# TRANSMIT BUFFER 1 DATA BYTE m REGISTER
TXB1D0 		= 0x46
TXB1D1 		= 0x47
TXB1D2 		= 0x48
TXB1D3 		= 0x49
TXB1D4 		= 0x4A
TXB1D5 		= 0x4B
TXB1D6 		= 0x4C
TXB1D7 		= 0x4D

# TRANSMIT BUFFER 2 DATA BYTE m REGISTER
TXB2D0 		= 0x56
TXB2D1 		= 0x57
TXB2D2 		= 0x58
TXB2D3 		= 0x59
TXB2D4 		= 0x5A
TXB2D5 		= 0x5B
TXB2D6 		= 0x5C
TXB2D7 		= 0x5D


# ===============================================
# ========== Receiption Registers ===============
# ===============================================

# RECEIVE BUFFER n CONTROL REGISTER
RXB0CTRL	= 0x60
RXB1CTRL	= 0x70

# (RXnBF)' PIN CONTROL AND STATUS REGISTER
BFPCTRL		= 0x0C

# RECEIVE BUFFER n STANDARD IDENTIFIER REGISTER HIGH
RXB0SIDH	= 0x61
RXB1SIDH	= 0x71

# RECEIVE BUFFER n STANDARD IDENTIFIER REGISTER LOW
RXB0SIDL	= 0x62
RXB1SIDL	= 0x72

# RXBnEID8: RECEIVE BUFFER n EXTENDED IDENTIFIER REGISTER HIGH
RXB0EID8	= 0x63
RXB1EID8	= 0x73

# RXBnEID0: RECEIVE BUFFER n EXTENDED IDENTIFIER REGISTER LOW
RXB0EID0	= 0x64
RXB1EID0	= 0x74

# RECEIVE BUFFER n DATA LENGTH CODE REGISTER
RXB0DLC		= 0x65
RXB1DLC		= 0x75

# RECEIVE BUFFER 0 DATA BYTE m REGISTER
RXB0D0 		= 0x66
RXB0D1 		= 0x67
RXB0D2 		= 0x68
RXB0D3 		= 0x69
RXB0D4 		= 0x6A
RXB0D5 		= 0x6B
RXB0D6 		= 0x6C
RXB0D7 		= 0x6D

# RECEIVE BUFFER 1 DATA BYTE m REGISTER
RXB1D0 		= 0x76
RXB1D1 		= 0x77
RXB1D2 		= 0x78
RXB1D3 		= 0x79
RXB1D4 		= 0x7A
RXB1D5 		= 0x7B
RXB1D6 		= 0x7C
RXB1D7 		= 0x7D

# ===============================================
# ============ Filter Registers =================
# ===============================================

# FILTER n STANDARD IDENTIFIER REGISTER HIGH
RXF0SIDH	= 0x00 
RXF1SIDH	= 0x04 
RXF2SIDH	= 0x08 
RXF3SIDH	= 0x10 
RXF4SIDH	= 0x14 
RXF5SIDH	= 0x18

# FILTER n STANDARD IDENTIFIER REGISTER LOW
RXF0SIDL	= 0x01 
RXF1SIDL	= 0x05 
RXF2SIDL	= 0x09 
RXF3SIDL	= 0x11 
RXF4SIDL	= 0x15 
RXF5SIDL	= 0x19

# FILTER n EXTENDED IDENTIFIER REGISTER HIGH
RXF0EID8	= 0x02 
RXF1EID8	= 0x06 
RXF2EID8	= 0x0A 
RXF3EID8	= 0x12 
RXF4EID8	= 0x16 
RXF5EID8	= 0x1A

# FILTER n EXTENDED IDENTIFIER REGISTER LOW
RXF0EID0	= 0x03 
RXF1EID0	= 0x07 
RXF2EID0	= 0x0B 
RXF3EID0	= 0x13 
RXF4EID0	= 0x17 
RXF5EID0	= 0x1B

# MASK n STANDARD IDENTIFIER REGISTER HIGH
RXM0SIDH	= 0x20
RXM1SIDH	= 0x24

# MASK n STANDARD IDENTIFIER REGISTER LOW
RXM0SIDL	= 0x21
RXM1SIDL	= 0x25

# MASK n EXTENDED IDENTIFIER REGISTER HIGH
RCM0EID8	= 0x22
RCM1EID8	= 0x26

# MASK n EXTENDED IDENTIFIER REGISTER LOW
RCM0EID0	= 0x23
RCM1EID0	= 0x27

# ===============================================
# ========== Configuration Registers ============
# ===============================================

# CONFIGURATION REGISTER 1
CNF1		= 0x2A

# CONFIGURATION REGISTER 2
CNF2		= 0x29

# CONFIGURATION REGISTER 3
CNF3		= 0x28

# ===============================================
# ============== Error Registers ================
# ===============================================

# TRANSMIT ERROR COUNTER REGISTER
TEC			= 0x1C

# RECEIVE ERROR COUNTER REGISTER
REC 		= 0x1D

# ERROR FLAG REGISTER
EFLG		= 0x2D

# ===============================================
# ============ Interrupt Registers ==============
# ===============================================

# CAN INTERRUPT ENABLE REGISTER
CANINTE		= 0x2B

# CAN INTERRUPT FLAG REGISTER
CANINTF		= 0x2C

# ===============================================
# ========= Operation Mode Registers ============
# ===============================================

# CAN CONTROL REGISTER
CANCTRL		= 0x0F

# CAN STATUS REGISTER
CANSTAT		= 0x0E

# ===============================================
# =============== Instructions ==================
# ===============================================

reset_instruction 			= 0xC0
read_instrucion				= 0x03
read_rx_buffer_instrucion	= 0x90
write_instruction			= 0x02
load_tx_buffer_instruction	= 0x40
request_to_send_instruction	= 0x80
read_status_instruction		= 0xA0
rx_status_instrucion		= 0xB0
bit_modify_instruction		= 0x05