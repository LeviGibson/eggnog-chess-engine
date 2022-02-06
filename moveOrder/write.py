import numpy as np
import struct

data = np.load("data.npz")['arr_0'].flatten()
outfile = open("moveOrderData.bin", 'wb')

for i in data:
    outfile.write(struct.pack('<f', i))
