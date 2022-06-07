#!/usr/bin/python3
#
# Simple script to help debug .gbow files by displaying information about it
import sys
import struct
import os
import xml.dom.minidom

def parseInfo(filePath):
  file = open(filePath, "rb")
  fileSize = os.path.getsize(filePath)
  print("File size: {}".format(fileSize))

  magic = int.from_bytes(file.read(4), "little")
  if magic == int('67626f77', 16):
    print("Valid magic value")
  else:
    print("Magic is wrong: {}".format(hex(magic)))
    return

  versionMajor = int.from_bytes(file.read(4), "little")
  versionMinor = int.from_bytes(file.read(4), "little")
  print("File version {}.{}".format(versionMajor, versionMinor))

  if versionMajor == 0:
      audioBufferSize = int.from_bytes(file.read(4), "little")
      audioBufferSamplerRate = file.read(8) # assumed double size
      audioBufferSamplerRate = struct.unpack('d', audioBufferSamplerRate)[0]
      audioBufferNumberOfSamples = int.from_bytes(file.read(4), "little")
      audioBufferChannel = int.from_bytes(file.read(4), "little")
      print("Audio Buffer info:")
      print("\tsize: {}".format(audioBufferSize))
      print("\tsampler rate: {}".format(audioBufferSamplerRate))
      print("\tnumber of samples: {}".format(audioBufferNumberOfSamples))
      print("\tchannel: {}".format(audioBufferChannel))

      specImageSpectrogramSize = int.from_bytes(file.read(4), "little")
      specImageHpcpSize = int.from_bytes(file.read(4), "little")
      specImageDetectedSize = int.from_bytes(file.read(4), "little")
      print("Spectrogram image info:")
      print("\tSpectrogram size: {}".format(specImageSpectrogramSize))
      print("\tHPCP size: {}".format(specImageHpcpSize))
      print("\tDectected size: {}".format(specImageDetectedSize))

      # skip buffers to get to XML
      skip = file.read(32 * 4) # uint32_t reserved[32];
      skip = file.read(audioBufferSize)
      skip = file.read(specImageSpectrogramSize)
      skip = file.read(specImageHpcpSize)
      skip = file.read(specImageDetectedSize)

      # rest of file
      xmlData = "".join(map(chr, file.read()))
      # size of header includes padding, simple way to remove it
      # Also need to remove trailing byte from doing a .join()
      xmlData = xmlData[xmlData.index("<?xml"):-1]
      xmlDom = xml.dom.minidom.parseString(xmlData)
      print(xmlDom.toprettyxml())

  else:
      print("File version not recognized")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("How to use: \npython " + sys.argv[0] + " input.gbow")
        exit(1)
    parseInfo(sys.argv[1])