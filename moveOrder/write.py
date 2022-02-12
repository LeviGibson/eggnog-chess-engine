import numpy as np
import struct
import matplotlib.pyplot as plt

data = np.load("data.npz")['arr_0'].flatten()
outfile = open("moveOrderData.bin", 'wb')
data = np.clip(data, -10000, 10000)

for i in data:
    outfile.write(struct.pack('<h', round(i)))
