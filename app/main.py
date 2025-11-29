import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque
import time

# -----------------------------
# CONFIGURATION
# -----------------------------
USB_PORT = '/dev/cu.usbmodem1301'
BAUDRATE = 115200
HISTORY_SECONDS = 300
GUI_WINDOW_SECONDS = 20
UPDATE_INTERVAL_MS = 50
MAX_POINTS = 10000

# DATA STORAGE
times = deque(maxlen=MAX_POINTS)
voltages = deque(maxlen=MAX_POINTS)
start_time = time.time()

# SETUP PLOT
plt.style.use('dark_background')
fig, ax = plt.subplots()
line, = ax.plot([], [], color='yellow', lw=1.8)

ax.set_xlabel("Time (s)")
ax.set_ylabel("Voltage (V)")
ax.set_title("Live Voltage Monitor (scroll + zoom)")

auto_scroll = True

def toggle_live(event):
    global auto_scroll
    if event.key == ' ':
        auto_scroll = not auto_scroll
        print(f"Live mode {'ON' if auto_scroll else 'OFF'}")

fig.canvas.mpl_connect('key_press_event', toggle_live)

# SERIAL CONNECTION HELPER
def try_connect():
    """Try to open the serial port, return None if unavailable."""
    try:
        ser = serial.Serial(USB_PORT, BAUDRATE, timeout=0.5)
        print(f"Connected to {USB_PORT}")
        return ser
    except serial.SerialException:
        print("Serial port not found. Retrying...")
        return None

ser = try_connect()

# UPDATE FUNCTION
def update(frame):
    global ser

    if ser is None or not ser.is_open:
        ser = try_connect()
        return line,

    # Read available lines
    try:
        while ser.in_waiting:
            line_str = ser.readline().decode('utf-8').strip()
            voltage = float(line_str)
            t = time.time() - start_time
            times.append(t)
            voltages.append(voltage)
    except (ValueError, serial.SerialException, OSError):
        # Port might have been disconnected mid-read
        print("Lost connection. Reconnecting...")
        try:
            ser.close()
        except:
            pass
        ser = None
        return line,

    if not times:
        return line,

    line.set_data(times, voltages)

    if auto_scroll:
        current_time = times[-1]
        ax.set_xlim(current_time - GUI_WINDOW_SECONDS, current_time)
        vmin, vmax = min(voltages), max(voltages)
        pad = (vmax - vmin) * 0.2 if vmax != vmin else 0.05
        ax.set_ylim(vmin - pad, vmax + pad)

    return line,

# ANIMATION
ani = animation.FuncAnimation(fig, update, interval=UPDATE_INTERVAL_MS, blit=False)
plt.show()
