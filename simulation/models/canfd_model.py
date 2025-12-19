"""
CAN-FD Model for Renode Simulation
Simulates CAN-FD bus communication for robot modules
"""

class CANFDMessage:
    """CAN-FD message structure"""
    def __init__(self, msg_id=0, data=None, dlc=0, brs=False, esi=False):
        self.id = msg_id
        self.data = data if data else bytearray(64)
        self.dlc = dlc
        self.brs = brs  # Bit Rate Switch
        self.esi = esi  # Error State Indicator


class CANFDModel:
    """
    CAN-FD bus simulation model.
    Handles communication between FRDM-MCXN947 and motor/sensor modules.
    """
    
    # Register offsets
    REG_CONTROL = 0x00      # Control register
    REG_STATUS = 0x04       # Status register
    REG_TX_ID = 0x08        # TX message ID
    REG_TX_DLC = 0x0C       # TX data length code
    REG_TX_DATA0 = 0x10     # TX data bytes 0-3
    REG_TX_DATA1 = 0x14     # TX data bytes 4-7
    REG_TX_DATA2 = 0x18     # TX data bytes 8-11
    REG_TX_DATA3 = 0x1C     # TX data bytes 12-15
    REG_RX_ID = 0x40        # RX message ID
    REG_RX_DLC = 0x44       # RX data length code
    REG_RX_DATA0 = 0x48     # RX data bytes 0-3
    REG_RX_DATA1 = 0x4C     # RX data bytes 4-7
    REG_RX_DATA2 = 0x50     # RX data bytes 8-11
    REG_RX_DATA3 = 0x54     # RX data bytes 12-15
    REG_FILTER_ID = 0x80    # Filter ID
    REG_FILTER_MASK = 0x84  # Filter mask
    REG_BAUDRATE = 0x88     # Baud rate setting
    REG_ERROR_CNT = 0x8C    # Error counter
    
    # Control bits
    CTRL_ENABLE = 0x01
    CTRL_TX_REQ = 0x02
    CTRL_LOOPBACK = 0x04
    CTRL_FD_MODE = 0x08
    CTRL_BRS = 0x10
    
    # Status bits
    STATUS_TX_DONE = 0x01
    STATUS_RX_READY = 0x02
    STATUS_ERROR = 0x04
    STATUS_BUS_OFF = 0x08
    STATUS_PASSIVE = 0x10
    
    # CAN-FD node IDs for robot modules
    NODE_ID_MOTOR_FL = 0x100
    NODE_ID_MOTOR_FR = 0x101
    NODE_ID_MOTOR_RL = 0x102
    NODE_ID_MOTOR_RR = 0x103
    NODE_ID_TURRET = 0x200
    NODE_ID_GPS = 0x300
    NODE_ID_IMU = 0x301
    NODE_ID_VISION = 0x400
    NODE_ID_MASTER = 0x001
    
    def __init__(self):
        self.control = 0
        self.status = 0
        self.tx_id = 0
        self.tx_dlc = 0
        self.tx_data = bytearray(64)
        self.rx_id = 0
        self.rx_dlc = 0
        self.rx_data = bytearray(64)
        self.filter_id = 0
        self.filter_mask = 0
        self.baudrate = 5000000  # 5 Mbps CAN-FD
        self.error_count = 0
        self.tx_queue = []
        self.rx_queue = []
        
    def _get_offset(self, offset):
        try:
            return offset.Offset
        except AttributeError:
            return offset

    def read(self, offset):
        actual_offset = self._get_offset(offset)
        if actual_offset == self.REG_CONTROL:
            return self.control
        elif actual_offset == self.REG_STATUS:
            return self.status
        elif actual_offset == self.REG_TX_ID:
            return self.tx_id
        elif actual_offset == self.REG_TX_DLC:
            return self.tx_dlc
        elif actual_offset == self.REG_TX_DATA0:
            return int.from_bytes(self.tx_data[0:4], 'little')
        elif actual_offset == self.REG_TX_DATA1:
            return int.from_bytes(self.tx_data[4:8], 'little')
        elif actual_offset == self.REG_TX_DATA2:
            return int.from_bytes(self.tx_data[8:12], 'little')
        elif actual_offset == self.REG_TX_DATA3:
            return int.from_bytes(self.tx_data[12:16], 'little')
        elif actual_offset == self.REG_RX_ID:
            return self.rx_id
        elif actual_offset == self.REG_RX_DLC:
            return self.rx_dlc
        elif actual_offset == self.REG_RX_DATA0:
            return int.from_bytes(self.rx_data[0:4], 'little')
        elif actual_offset == self.REG_RX_DATA1:
            return int.from_bytes(self.rx_data[4:8], 'little')
        elif actual_offset == self.REG_RX_DATA2:
            return int.from_bytes(self.rx_data[8:12], 'little')
        elif actual_offset == self.REG_RX_DATA3:
            return int.from_bytes(self.rx_data[12:16], 'little')
        elif actual_offset == self.REG_FILTER_ID:
            return self.filter_id
        elif actual_offset == self.REG_FILTER_MASK:
            return self.filter_mask
        elif actual_offset == self.REG_BAUDRATE:
            return self.baudrate
        elif actual_offset == self.REG_ERROR_CNT:
            return self.error_count
        return 0

    def write(self, offset, value):
        actual_offset = self._get_offset(offset)
        if actual_offset == self.REG_CONTROL:
            self.control = value
            if value & self.CTRL_TX_REQ:
                self._transmit()
        elif actual_offset == self.REG_TX_ID:
            self.tx_id = value
        elif actual_offset == self.REG_TX_DLC:
            self.tx_dlc = value
        elif actual_offset == self.REG_TX_DATA0:
            self.tx_data[0:4] = value.to_bytes(4, 'little')
        elif actual_offset == self.REG_TX_DATA1:
            self.tx_data[4:8] = value.to_bytes(4, 'little')
        elif actual_offset == self.REG_TX_DATA2:
            self.tx_data[8:12] = value.to_bytes(4, 'little')
        elif actual_offset == self.REG_TX_DATA3:
            self.tx_data[12:16] = value.to_bytes(4, 'little')
        elif actual_offset == self.REG_FILTER_ID:
            self.filter_id = value
        elif actual_offset == self.REG_FILTER_MASK:
            self.filter_mask = value
        elif actual_offset == self.REG_BAUDRATE:
            self.baudrate = value
            
    def _transmit(self):
        """Simulate CAN-FD message transmission"""
        if not (self.control & self.CTRL_ENABLE):
            return
            
        msg = CANFDMessage(
            msg_id=self.tx_id,
            data=bytearray(self.tx_data),
            dlc=self.tx_dlc,
            brs=(self.control & self.CTRL_BRS) != 0
        )
        
        # Loopback mode for testing
        if self.control & self.CTRL_LOOPBACK:
            self._receive(msg)
        else:
            self.tx_queue.append(msg)
            
        self.status |= self.STATUS_TX_DONE
        self.control &= ~self.CTRL_TX_REQ
        
    def _receive(self, msg):
        """Handle received CAN-FD message"""
        # Apply filter
        if (msg.id & self.filter_mask) != (self.filter_id & self.filter_mask):
            return
            
        self.rx_id = msg.id
        self.rx_dlc = msg.dlc
        self.rx_data = msg.data
        self.status |= self.STATUS_RX_READY


# Renode peripheral interface
canfd = CANFDModel()

def read(offset):
    return canfd.read(offset)

def write(offset, value):
    canfd.write(offset, value)
