 
import numpy as np
import cv2
import struct
import time

import sys

# python videoReader.py sourcefile

filename = sys.argv[1]

f = open(filename, 'rb')
header = f.read(4)
assert header.decode('ASCII') == "SETR"

width = struct.unpack("<I", f.read(4))[0]
height = struct.unpack("<I", f.read(4))[0]
channels = struct.unpack("<I", f.read(4))[0]
fps = struct.unpack("<I", f.read(4))[0]

while True:
    timeB = time.time()
    s = struct.unpack("<I", f.read(4))[0]
    if s == 0:
        print("End of video")
    data = np.frombuffer(f.read(s), np.uint8, s)
    im = cv2.imdecode(data, 1)

    print("!!!", im.shape)
    
    cv2.imshow('frame',im)
    if cv2.waitKey(int((1/fps - (time.time() - timeB))*1000)) & 0xFF == ord('q'):
        break

f.close()
cv2.destroyAllWindows()