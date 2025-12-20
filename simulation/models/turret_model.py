"""
Turret Model for Renode Simulation
Simulates pan/tilt turret with servo motors
"""

class TurretModel:
    """
    Pan/Tilt turret simulation model.
    Controls two servo motors for horizontal (pan) and vertical (tilt) movement.
    """
    
    # Register offsets
    REG_CONTROL = 0x00        # Control register
    REG_PAN_SETPOINT = 0x04   # Pan angle setpoint (degrees * 100)
    REG_PAN_ACTUAL = 0x08     # Actual pan angle (degrees * 100)
    REG_TILT_SETPOINT = 0x0C  # Tilt angle setpoint (degrees * 100)
    REG_TILT_ACTUAL = 0x10    # Actual tilt angle (degrees * 100)
    REG_PAN_SPEED = 0x14      # Pan speed (degrees/sec)
    REG_TILT_SPEED = 0x18     # Tilt speed (degrees/sec)
    REG_STATUS = 0x1C         # Status register
    REG_CANFD_ID = 0x20       # CAN-FD node ID
    
    # Control bits
    CTRL_ENABLE = 0x01
    CTRL_HOME = 0x02
    CTRL_TRACK_MODE = 0x04
    
    # Status bits
    STATUS_MOVING = 0x01
    STATUS_AT_LIMIT = 0x02
    STATUS_HOMED = 0x04
    STATUS_FAULT = 0x08
    
    # Limits (degrees * 100)
    PAN_MIN = -18000   # -180 degrees
    PAN_MAX = 18000    # +180 degrees
    TILT_MIN = -4500   # -45 degrees
    TILT_MAX = 9000    # +90 degrees
    
    def __init__(self):
        self.control = 0
        self.pan_setpoint = 0
        self.pan_actual = 0
        self.tilt_setpoint = 0
        self.tilt_actual = 0
        self.pan_speed = 3000    # 30 deg/sec default
        self.tilt_speed = 2000   # 20 deg/sec default
        self.status = 0
        self.canfd_id = 0x200
        
    def _get_offset(self, offset):
        try:
            return offset.Offset
        except AttributeError:
            return offset

    def read(self, offset):
        actual_offset = self._get_offset(offset)
        if actual_offset == self.REG_CONTROL:
            return self.control
        elif actual_offset == self.REG_PAN_SETPOINT:
            return self.pan_setpoint & 0xFFFFFFFF
        elif actual_offset == self.REG_PAN_ACTUAL:
            return self.pan_actual & 0xFFFFFFFF
        elif actual_offset == self.REG_TILT_SETPOINT:
            return self.tilt_setpoint & 0xFFFFFFFF
        elif actual_offset == self.REG_TILT_ACTUAL:
            return self.tilt_actual & 0xFFFFFFFF
        elif actual_offset == self.REG_PAN_SPEED:
            return self.pan_speed
        elif actual_offset == self.REG_TILT_SPEED:
            return self.tilt_speed
        elif actual_offset == self.REG_STATUS:
            return self.status
        elif actual_offset == self.REG_CANFD_ID:
            return self.canfd_id
        return 0

    def write(self, offset, value=None):
        # Debug: print type and attributes of offset
        try:
            print("[turret_model.py] write() called with offset type:", type(offset), "attributes:", dir(offset))
        except Exception:
            pass
        # Support both (offset, value) and (request) signatures
        if value is None and hasattr(offset, 'Offset') and hasattr(offset, 'Value'):
            actual_offset = offset.Offset
            value = offset.Value
        else:
            actual_offset = self._get_offset(offset)
        if actual_offset == self.REG_CONTROL:
            self.control = value
            if value & self.CTRL_HOME:
                self._home()
        elif actual_offset == self.REG_PAN_SETPOINT:
            # Handle signed value
            if value > 0x7FFFFFFF:
                value = value - 0x100000000
            self.pan_setpoint = max(self.PAN_MIN, min(self.PAN_MAX, value))
        elif actual_offset == self.REG_TILT_SETPOINT:
            if value > 0x7FFFFFFF:
                value = value - 0x100000000
            self.tilt_setpoint = max(self.TILT_MIN, min(self.TILT_MAX, value))
        elif actual_offset == self.REG_PAN_SPEED:
            self.pan_speed = value
        elif actual_offset == self.REG_TILT_SPEED:
            self.tilt_speed = value
        elif actual_offset == self.REG_CANFD_ID:
            self.canfd_id = value
            
    def _home(self):
        """Home the turret to center position"""
        self.pan_setpoint = 0
        self.tilt_setpoint = 0
        self.pan_actual = 0
        self.tilt_actual = 0
        self.status = self.STATUS_HOMED  # Clear other status flags when homing
        
    def update(self):
        """Update turret position based on setpoints"""
        if not (self.control & self.CTRL_ENABLE):
            return
            
        # Update pan position
        if self.pan_actual < self.pan_setpoint:
            self.pan_actual = min(self.pan_setpoint, self.pan_actual + self.pan_speed // 100)
        elif self.pan_actual > self.pan_setpoint:
            self.pan_actual = max(self.pan_setpoint, self.pan_actual - self.pan_speed // 100)
            
        # Update tilt position
        if self.tilt_actual < self.tilt_setpoint:
            self.tilt_actual = min(self.tilt_setpoint, self.tilt_actual + self.tilt_speed // 100)
        elif self.tilt_actual > self.tilt_setpoint:
            self.tilt_actual = max(self.tilt_setpoint, self.tilt_actual - self.tilt_speed // 100)
            
        # Update status
        if self.pan_actual != self.pan_setpoint or self.tilt_actual != self.tilt_setpoint:
            self.status |= self.STATUS_MOVING
        else:
            self.status &= ~self.STATUS_MOVING
            
        # Check limits
        if (self.pan_actual == self.PAN_MIN or self.pan_actual == self.PAN_MAX or
            self.tilt_actual == self.TILT_MIN or self.tilt_actual == self.TILT_MAX):
            self.status |= self.STATUS_AT_LIMIT
        else:
            self.status &= ~self.STATUS_AT_LIMIT


# Renode peripheral interface
turret = TurretModel()

def read(offset):
    turret.update()
    return turret.read(offset)

def write(offset, value):
    turret.write(offset, value)
