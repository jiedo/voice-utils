import alsaaudio

for pcm in  alsaaudio.pcms():
    print pcm

print

for c in  alsaaudio.cards():
    print c

recorder = alsaaudio.PCM(type=alsaaudio.PCM_CAPTURE, device="sysdefault:CARD=Microphone")
print recorder.dumpinfo()

player = alsaaudio.PCM(type=alsaaudio.PCM_PLAYBACK, device="sysdefault:CARD=PCH")
print player.dumpinfo()


# print "alsaaudio.PCM_PLAYBACK =", alsaaudio.PCM_PLAYBACK
# print "alsaaudio.PCM_CAPTURE =", alsaaudio.PCM_CAPTURE
# print "type:", pcm.pcmtype()
# print "mode:", pcm.pcmmode()
# print "card:", pcm.cardname()

#['__class__', '__delattr__', '__doc__', '__format__', '__getattribute__', '__hash__', '__init__', '__new__', '__reduce__', '__reduce_ex__', '__repr__', '__setattr__', '__sizeof__', '__str__', '__subclasshook__', 'cardname', 'close', 'dumpinfo', 'pause', 'pcmmode', 'pcmtype', 'polldescriptors', 'read', 'setchannels', 'setformat', 'setperiodsize', 'setrate', 'write']

while True:
    print ".",
    size, data = recorder.read()
    if size > 0:
        player.write(data)
