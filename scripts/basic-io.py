
import usb.core
import usb.util

VID = 0x0451
PID = 0xE001

EP_IN = 0x81
EP_OUT = 0x02

dev = usb.core.find(idVendor=VID, idProduct=PID)

if dev is None:
    raise RuntimeError("TI SilverLink not found")

# Claim interface 0
if dev.is_kernel_driver_active(0):
    dev.detach_kernel_driver(0)

dev.set_configuration()
usb.util.claim_interface(dev, 0)

def write(data: bytes):
    n = dev.write(EP_OUT, data, timeout=5000)
    if n != len(data):
        raise IOError(f"Short write: {n}/{len(data)}")

def read(max_bytes=64, timeout=5000) -> bytes:
    return bytes(dev.read(EP_IN, max_bytes, timeout=timeout))

# Replace this with an actual protocol packet.
# write(bytes.fromhex("..."))

try:
    while True:
        try:
            data = read(timeout=1000)
            print("RX:", data.hex(" "))
        except usb.core.USBTimeoutError:
            pass
finally:
    usb.util.release_interface(dev, 0)
    usb.util.dispose_resources(dev)
