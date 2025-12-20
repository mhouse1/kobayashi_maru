print("=== LOADING motor_model.py ===")
"""
Motor Model for Renode Simulation
Simulates a brushless DC motor with encoder feedback for 4WD robot

"""
import os
print("[motor_model.py] Loaded from: {}".format(os.path.abspath(__file__)))

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
    
    def __init__(self, base_address=0x50001000):
        self.base_address = base_address
        self.control = 0
        self.speed_setpoint = 0
        self.actual_speed = 0
        self.position = 0
        self.current = 0
        self.status = 0
        self.canfd_id = 0x100

    def read(self, request):
        offset = request.address - self.base_address
        if offset == self.REG_CONTROL:
            return self.control
        elif offset == self.REG_SPEED_SET:
            return self.speed_setpoint
        elif offset == self.REG_SPEED_ACT:
            return self.actual_speed
        elif offset == self.REG_POSITION:
            return self.position
        elif offset == self.REG_CURRENT:
            return self.current
        elif offset == self.REG_STATUS:
            return self.status
        elif offset == self.REG_CANFD_ID:
            return self.canfd_id
        return 0

    def write(self, request):
        offset = request.address - self.base_address
        value = request.value
        try:
            print("[motor_model.py] write() called with offset:", offset, "value:", value)
        except Exception:
            pass
        if offset == self.REG_CONTROL:
            self.control = value
            self._update_motor_state()
        elif offset == self.REG_SPEED_SET:
            self.speed_setpoint = value
            self._update_motor_state()
        elif offset == self.REG_CANFD_ID:
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
# Support multiple instances mapped to different base addresses
# Use a registry keyed by peripheral base so each PythonPeripheral
# instance gets its own MotorModel with the correct base_address.
_models = {}

def _get_base_from_request(request):
    # Peripherals in the REPL use size 0x100, align down to that boundary
    return request.address & ~0xFF

def _get_model_for_request(request):
    base = _get_base_from_request(request)
    if base not in _models:
        _models[base] = MotorModel(base_address=base)
    return _models[base]

def read(request):
    model = _get_model_for_request(request)
    return model.read(request)

def write(request):
    model = _get_model_for_request(request)
    model.write(request)