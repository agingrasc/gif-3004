import sys
import time
import alsaaudio
import numpy as np
import scipy.io.wavfile as wave
import scipy.signal as signal
import threading
from matplotlib import pyplot as plt


fs, data = wave.read(sys.argv[1])
print(data.shape)
data = data[:, 0] # np.newaxis]
print(data.shape)
dataBuf = data.tobytes()

SAMPLE_RATE = fs
PERIOD_SAMPLE_PLAY = 8
PERIOD_SAMPLE_REC = 8
TRESHOLD_DETECT = 10

_t_rewind = False
_t_recordAgain = False
_t_playDone = False
_t_recDone = False
_t_beginTimePlay = None
_t_beginTimeRecord = None
_t_bufferRecord = np.zeros(len(data), dtype='int16')
_t_barrier = threading.Barrier(2)

def playAudio():
    global _t_rewind, _t_beginTimePlay, _t_playDone
    print(">>>", fs, data.dtype)

    out = alsaaudio.PCM(alsaaudio.PCM_PLAYBACK, alsaaudio.PCM_NONBLOCK, device="default")
    out.setchannels(1)
    out.setrate(SAMPLE_RATE)
    out.setformat(alsaaudio.PCM_FORMAT_S16_LE)
    out.setperiodsize(PERIOD_SAMPLE_PLAY)

    while True:
        posWrite = 0
        _t_rewind = False
        _t_barrier.wait()
        _t_beginTimePlay = time.perf_counter()
        while (posWrite + PERIOD_SAMPLE_PLAY)*2 <= len(dataBuf):
            length = out.write(dataBuf[posWrite*2 : (posWrite + PERIOD_SAMPLE_PLAY)*2])
            if length == 0:
                time.sleep(0.001)
            else:
                posWrite += PERIOD_SAMPLE_PLAY

        #out.pause(True)
        _t_playDone = True
        while not _t_rewind:
            time.sleep(0.001)
        #out.pause(False)


def recordAudio():
    global _t_recordAgain, _t_beginTimeRecord, _t_bufferRecord, _t_recDone

    inp = alsaaudio.PCM(alsaaudio.PCM_CAPTURE, alsaaudio.PCM_NONBLOCK, device="default")
    inp.setchannels(1)
    inp.setrate(SAMPLE_RATE)
    inp.setformat(alsaaudio.PCM_FORMAT_S16_LE)
    inp.setperiodsize(PERIOD_SAMPLE_REC)

    while True:
        posRec = 0
        _t_recordAgain = False
        length = -1
        while length != 0:
            # Vider le buffer (pas de fonction dans l'API de alsaaudio pour le faire??)
            length, data = inp.read()
        _t_barrier.wait()
        _t_beginTimeRecord = time.perf_counter()

        while True:
            length, data = inp.read()
            if posRec + length > len(_t_bufferRecord):
                #inp.pause(True)
                break
            #print(length, len(data), buf.shape)
            if length > 0:
                #print("REC", PERIOD_SAMPLE_REC, length, np.frombuffer(data, dtype='int16', count=-1).shape)
                _t_bufferRecord[posRec:posRec+length] = np.frombuffer(data, dtype='int16', count=-1)
                posRec += length
            elif length < 0:
                print("ERREUR", length)
            else:
                time.sleep(0.001)

        
        _t_recDone = True
        while not _t_recordAgain:
            time.sleep(0.001)
        #inp.pause(False)



threadPlay = threading.Thread(target=playAudio)
threadRec = threading.Thread(target=recordAudio)
threadRec.start()
threadPlay.start()

plt.ion()
fig = plt.figure()
graphPlayed = plt.subplot(211)
graphPlayed.plot(data)
plt.title("Signal original")
graphRec = plt.subplot(212)

while True:
    while not _t_recDone:
        time.sleep(0.01)

    diffStart = _t_beginTimePlay - _t_beginTimeRecord

    testP = data # np.hstack((data, np.zeros((40000,))))
    testR = _t_bufferRecord #np.hstack((np.zeros((45000,)), _t_bufferRecord[:-45000]))
    corr = signal.correlate(testR.astype('float32') / 2**15, testP.astype('float32') / 2**15, mode="full", method="fft")
    idxMax = np.argmax(corr) - (len(testP)-1)
    # signal.fftconvolve(testP.astype('float32') / 2**15, testR[::-1].astype('float32') / 2**15)
    if corr[idxMax+(len(testP)-1)] > TRESHOLD_DETECT:
        latence = idxMax / SAMPLE_RATE - diffStart
        incertitude = ((PERIOD_SAMPLE_REC+PERIOD_SAMPLE_PLAY) / SAMPLE_RATE + np.abs(diffStart))
        titre = "Signal détecté (latence = {:.2f} ± {:.2f} ms)".format(latence*1000, 1000*incertitude)
    else:
        titre = "Aucun signal détecté (pic de corrélation : {:.5f})!".format(corr[idxMax+(len(testP)-1)])

    print(idxMax, corr[idxMax+(len(testP)-1)], diffStart)

    plt.cla()
    graphRec.cla()
    graphRec.plot(testR, color='orange')
    plt.title(titre)
    fig.canvas.draw()
    plt.draw()
    plt.pause(0.01)

    while not _t_playDone:
        time.sleep(0.01)

    _t_bufferRecord.fill(0)

    _t_playDone = False
    _t_recDone = False
    _t_recordAgain = True
    _t_rewind = True


#print("DONE RECORDING")
#wave.write("testFeedback.wav", SAMPLE_RATE, buf)
t.join()




