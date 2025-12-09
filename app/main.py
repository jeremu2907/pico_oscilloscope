import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque
import time

USB_PORT = '/dev/cu.usbmodem1101'
BAUDRATE = 115200
GUI_WINDOW_SECONDS = 20
UPDATE_INTERVAL_MS = 50
MAX_POINTS = 10000

times_ch0 = deque(maxlen=MAX_POINTS)
times_ch1 = deque(maxlen=MAX_POINTS)
volt_ch0 = deque(maxlen=MAX_POINTS)
volt_ch1 = deque(maxlen=MAX_POINTS)
start_time = time.time()

plt.style.use('dark_background')
fig, ax = plt.subplots()
channel_0_line, = ax.plot([], [], color='yellow', lw=1.8, label='channel 0')
channel_1_line, = ax.plot([], [], color='red', lw=1.8, label='channel 1')
ax.legend()

auto_scroll = True

def toggle_live(event):
    global auto_scroll
    if event.key == ' ':
        auto_scroll = not auto_scroll

fig.canvas.mpl_connect('key_press_event', toggle_live)

def try_connect():
    try:
        return serial.Serial(USB_PORT, BAUDRATE, timeout=0.5)
    except:
        return None

ser = try_connect()

def update(frame):
    global ser

    if ser is None or not ser.is_open:
        ser = try_connect()
        return channel_0_line, channel_1_line

    try:
        while ser.in_waiting:
            line = ser.readline().decode('utf-8').strip()
            parts = line.split()
            if len(parts) != 2:
                continue
            ch = parts[0]
            try:
                v = float(parts[1])
            except:
                continue
            t = time.time() - start_time
            if ch == 'ch_0':
                times_ch0.append(t)
                volt_ch0.append(v)
            elif ch == 'ch_1':
                times_ch1.append(t)
                volt_ch1.append(v)
    except:
        try:
            ser.close()
        except:
            pass
        ser = None
        return channel_0_line, channel_1_line

    channel_0_line.set_data(times_ch0, volt_ch0)
    channel_1_line.set_data(times_ch1, volt_ch1)

    if auto_scroll:
        last_t = 0
        if times_ch0:
            last_t = max(last_t, times_ch0[-1])
        if times_ch1:
            last_t = max(last_t, times_ch1[-1])
        ax.set_xlim(last_t - GUI_WINDOW_SECONDS, last_t)

        vals = []
        vals.extend(volt_ch0)
        vals.extend(volt_ch1)
        if vals:
            vmin, vmax = min(vals), max(vals)
            pad = (vmax - vmin) * 0.2 if vmax != vmin else 0.05
            ax.set_ylim(vmin - pad, vmax + pad)

    return channel_0_line, channel_1_line

ani = animation.FuncAnimation(fig, update, interval=UPDATE_INTERVAL_MS, blit=False, save_count=MAX_POINTS)
plt.show()
