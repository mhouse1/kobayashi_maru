"""
Motor Model for Renode Simulation
Simulates a brushless DC motor with encoder feedback for 4WD robot
"""

class MotorModel:
    """
    Motor simulation model for heavy duty 4WD robot wheels.
    Supports CAN-FD communication for speed and position control.
    """
    
    # Register offsets
    REG_CONTROL = 0x00      # Control register (start/stop, direction)
    REG_SPEED_SET = 0x04    # Speed setpoint (RPM)
    REG_SPEED_ACT = 0x08    # Actual speed (RPM)
    REG_POSITION = 0x0C     # Encoder position (ticks)
    REG_CURRENT = 0x10      # Motor current (mA)
    REG_STATUS = 0x14       # Status register
    REG_CANFD_ID = 0x18     # CAN-FD node ID
    
    # Control bits
    CTRL_ENABLE = 0x01
    CTRL_DIRECTION = 0x02
    CTRL_BRAKE = 0x04
    
    # Status bits
    STATUS_RUNNING = 0x01
    STATUS_FAULT = 0x02
    STATUS_OVER_CURRENT = 0x04
    STATUS_OVER_TEMP = 0x08
    
    def __init__(self):
        self.control = 0
        self.speed_setpoint = 0
        self.actual_speed = 0
        self.position = 0
        self.current = 0
        self.status = 0
        self.canfd_id = 0x100
        
    def _get_offset(self, offset):
        try:
            return offset.Offset
        except AttributeError:
            return offset

    def read(self, offset):
        actual_offset = self._get_offset(offset)
        if actual_offset == self.REG_CONTROL:
            return self.control
        elif actual_offset == self.REG_SPEED_SET:
            return self.speed_setpoint
        elif actual_offset == self.REG_SPEED_ACT:
            return self.actual_speed
        elif actual_offset == self.REG_POSITION:
            return self.position
        elif actual_offset == self.REG_CURRENT:
            return self.current
        elif actual_offset == self.REG_STATUS:
            return self.status
        elif actual_offset == self.REG_CANFD_ID:
            return self.canfd_id
        return 0

    def write(self, offset, value=None):
        # Debug: print type and attributes of offset
        try:
            print("[motor_model.py] write() called with offset type:", type(offset), "attributes:", dir(offset))
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
            self._update_motor_state()
        elif actual_offset == self.REG_SPEED_SET:
            self.speed_setpoint = value
            self._update_motor_state()
        elif actual_offset == self.REG_CANFD_ID:
            self.canfd_id = value
            
    def _update_motor_state(self):
        """Simulate motor response to control inputs"""
        if self.control & self.CTRL_ENABLE:
            # Motor enabled - ramp to setpoint
            self.status |= self.STATUS_RUNNING
            # Simulate motor reaching speed with some delay
            if self.actual_speed < self.speed_setpoint:
                self.actual_speed = min(self.speed_setpoint, self.actual_speed + 100)
            elif self.actual_speed > self.speed_setpoint:
                self.actual_speed = max(self.speed_setpoint, self.actual_speed - 100)
            # Update position based on speed
            self.position += self.actual_speed // 60  # ticks per update
            # Simulate current draw
            self.current = abs(self.actual_speed) * 10  # mA
        else:
            # Motor disabled
            self.status &= ~self.STATUS_RUNNING
            if self.control & self.CTRL_BRAKE:
                self.actual_speed = 0
            else:
                # Coast down
                if self.actual_speed > 0:
                    self.actual_speed -= 50
                elif self.actual_speed < 0:
                    self.actual_speed += 50
            self.current = 0


# Renode peripheral interface
motor = MotorModel()

def read(offset):
    return motor.read(offset)

def write(offset, value):
    motor.write(offset, value)
