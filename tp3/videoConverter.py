import numpy as np
from scipy.misc import imresize
import cv2
import struct
import time

import sys

JPEG_QUALITY = 65
# python videoConverter.py sourcefile beginposmsec durationmsec outputheight

filename = sys.argv[1]
secpos = int(sys.argv[2])
countframes = int(sys.argv[3]) * 30 / 1000
height = int(sys.argv[4])

cap = cv2.VideoCapture(filename)
if not cap.isOpened():
    print("Erreur lecture")
    exit()

dest = open(filename[:-4] + ".ulv", 'wb')
dest.write(bytes("SETR", "ascii"))


cap.set(cv2.CAP_PROP_POS_MSEC, secpos)

count = 0
at = time.time()
while cap.isOpened() and count < countframes:
    ret, frame = cap.read()
    if not ret:
        print("Erreur grab")
        break

    if frame.shape[0] != height:
        frame = imresize(frame, height/frame.shape[0])

    if count == 0:
        dest.write(struct.pack("<I", frame.shape[1]))
        dest.write(struct.pack("<I", frame.shape[0]))
        dest.write(struct.pack("<I", frame.shape[2]))
        dest.write(struct.pack("<I", 30))


    ok, encimg = cv2.imencode('.jpg', frame, [int(cv2.IMWRITE_JPEG_QUALITY), JPEG_QUALITY])
    dest.write(struct.pack("<I", encimg.size))
    dest.write(encimg.tobytes())
    count += 1
print("Conversion de {} frames effectuee en {} secondes".format(count, time.time()-at))

dest.write(struct.pack("<I", 0))
dest.close()
cap.release()
cv2.destroyAllWindows()