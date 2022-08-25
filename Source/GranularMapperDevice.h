/*
  ==============================================================================

    GranularMapperDevice.h
    Created: 24 Aug 2022 9:03:47pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <vector>
#include "mapper/mapper.h"
#include <memory>

class GranularMapperDevice {
 public:
  GranularMapperDevice() {
    mGraph = mpr_graph_new(MPR_OBJ);
    mDevice = mpr_dev_new("granularSynth", mGraph);
  }
  ~GranularMapperDevice() {
    mpr_graph_free(mGraph);
    mpr_dev_free(mDevice);
    mGrainStreams.clear();
  }

  typedef struct GrainStream {
    GrainStream(mpr_dev dev) {
      float min = 0.0f;
      float max = 0.0f;
      int numInst = 8;
      gain = mpr_sig_new(dev, MPR_DIR_IN, "gain", 1, MPR_FLT, nullptr, &(min = 0.0f), &(max = 1.0f), &numInst, nullptr, 0);
      rateHz = mpr_sig_new(dev, MPR_DIR_IN, "rateHz", 1, MPR_FLT, nullptr, &(min = 0.25f), &(max = 20.0f), &numInst, nullptr, 0);
      durationMs = mpr_sig_new(dev, MPR_DIR_IN, "durationMs", 1, MPR_FLT, nullptr, &(min = 60.0f), &(max = 1000.0f), &numInst, nullptr, 0);
      grainShape = mpr_sig_new(dev, MPR_DIR_IN, "grainShape", 1, MPR_FLT, nullptr, &(min = 0.0f), &(max = 1.0f), &numInst, nullptr, 0);
      grainTilt = mpr_sig_new(dev, MPR_DIR_IN, "grainTilt", 1, MPR_FLT, nullptr, &(min = 0.0f), &(max = 1.0f), &numInst, nullptr, 0);
      pbRate = mpr_sig_new(dev, MPR_DIR_IN, "pbRate", 1, MPR_FLT, nullptr, &(min = 0.25f), &(max = 4.0f), &numInst, nullptr, 0);
      posRatio =
          mpr_sig_new(dev, MPR_DIR_IN, "posRatio", 1, MPR_FLT, nullptr, &(min = 0.0f), &(max = 1.0f), &numInst, nullptr, 0);
      posSpray =
          mpr_sig_new(dev, MPR_DIR_IN, "posSpray", 1, MPR_FLT, nullptr, &(min = 0.0f), &(max = 1.0f), &numInst, nullptr, 0);
      pitchSpray = mpr_sig_new(dev, MPR_DIR_IN, "pitchSpray", 1, MPR_FLT, nullptr, &(min = 0.0f), &(max = 1.0f), &numInst, nullptr, 0);
    }
    ~GrainStream() {
      mpr_sig_free(gain);
      mpr_sig_free(rateHz);
      mpr_sig_free(durationMs);
      mpr_sig_free(grainShape);
      mpr_sig_free(grainTilt);
      mpr_sig_free(pbRate);
      mpr_sig_free(posRatio);
      mpr_sig_free(posSpray);
      mpr_sig_free(pitchSpray);
    }

    mpr_sig gain;
    mpr_sig rateHz;
    mpr_sig durationMs;
    mpr_sig grainShape;
    mpr_sig grainTilt;
    mpr_sig pbRate;
    mpr_sig posRatio;
    mpr_sig posSpray;
    mpr_sig pitchSpray;
  } GrainStream;

 private:
  mpr_graph mGraph = nullptr;
  mpr_dev mDevice = nullptr;
  std::vector<GrainStream> mGrainStreams;
};
