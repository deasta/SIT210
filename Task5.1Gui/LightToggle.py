import tkinter as tk
# *** SIT210 Raspberry Pi Led GUI ***
#Controls hree connected LEDs using radio buttons. Only allows for a single light to be turned on at once.
# LED Pins
LED_PINS    = {"Bathroom": 17, "Living Room": 27, "Closet": 22}
LED_COLOURS = {"Bathroom": "blue", "Living Room": "green", "Closet": "yellow"}

try:
    import RPi.GPIO as GPIO
    GPIO.setmode(GPIO.BCM)
    for pin in LED_PINS.values():
        GPIO.setup(pin, GPIO.OUT, initial=GPIO.LOW)
    HAS_GPIO = True
except (ImportError, RuntimeError):
    HAS_GPIO = False

def set_pin(pin, state):
    # sets passed pin to HIGH, else LOW.
    # Selects one pin as high only
    if HAS_GPIO:
        GPIO.output(pin, GPIO.HIGH if state else GPIO.LOW)

def on_select(name):
    for n, pin in LED_PINS.items():
        set_pin(pin, n == name)

def all_off():
    selected.set("")
    for pin in LED_PINS.values():
        set_pin(pin, False)

root = tk.Tk()
root.title("LED Controller")
root.resizable(False, False)

selected = tk.StringVar(value="")
# Sellects Pin
for name in LED_PINS:
    tk.Radiobutton(root, text=name, variable=selected, value=name,
                   width=16, indicatoron=False, height=2,
                   background=LED_COLOURS[name],
                   command=lambda n=name: on_select(n)).pack(padx=20, pady=8)

tk.Button(root, text="All Off", width=16, height=2,
          command=all_off).pack(padx=20, pady=8)

def on_close():
    if HAS_GPIO:
        for pin in LED_PINS.values():
            GPIO.output(pin, GPIO.LOW)
        GPIO.cleanup()
    root.destroy()

root.protocol("WM_DELETE_WINDOW", on_close)
root.mainloop()
