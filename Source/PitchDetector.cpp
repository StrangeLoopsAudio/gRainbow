/*
  ==============================================================================

    PitchDetector.cpp
    Created: 24 Apr 2021 2:07:10pm
    Author:  brady

  ==============================================================================
*/

#define _USE_MATH_DEFINES

#include "PitchDetector.h"

#include <limits.h>

PitchDetector::PitchDetector()
    : mFft(FFT_SIZE, HOP_SIZE), juce::Thread("pitch detector thread") {
  initHarmonicWeights();
}

PitchDetector::~PitchDetector() { stopThread(2000); }

void PitchDetector::processBuffer(juce::AudioBuffer<float>* fileBuffer,
                                  double sampleRate) {
  stopThread(2000);
  mFileBuffer = fileBuffer;
  mSampleRate = sampleRate;
  startThread();
}

void PitchDetector::run() {
  if (mFileBuffer == nullptr) return;
  mFft.processBuffer(*mFileBuffer);
  if (threadShouldExit()) return;
  computeHPCP();
  if (threadShouldExit()) return;
  segmentPitches();
  if (threadShouldExit()) return;
  estimatePitches();
  if (onPitchesUpdated != nullptr && !threadShouldExit()) {
    //onPitchesUpdated(mHPCP);
    onPitchesUpdated(mPitchesTest);
  }
}

void PitchDetector::estimatePitches() {
  mPitchesTest.clear();
  /*for (int frame = 0; frame < mHPCP.size(); ++frame) {
    mPitchesTest.push_back(std::vector<float>(NUM_PITCH_CLASSES, 0.0f));
    for (int i = 0; i < NUM_PITCH_CLASSES; ++i) {
      if (mHPCP[frame][i] == 1.0f) {
        mPitchesTest[frame][i] = 1.0f;
      }
    }
  } */
  for (int frame = 0; frame < mHPCP.size(); ++frame) {
    mPitchesTest.push_back(std::vector<float>(mHPCP[frame].size(), 0.0f));
  }
  for (int i = 0; i < mPitches.size(); ++i) {
    auto pitch = mPitches[i];
    int frame = pitch.posRatio * (mHPCP.size() - 1);
    int bin = (int)(pitch.pitchClass * (NUM_PITCH_CLASSES / 12.0)) + 6;
    DBG("frame: " << frame << ", bin: " << bin << ", pc: " << pitch.pitchClass);
    for (int j = 0; j < pitch.duration; ++j) {
      mPitchesTest[frame + j][bin] = 1.0f;
    }
  }
}

void PitchDetector::computeHPCP() {
  mHPCP.clear();

  std::vector<std::vector<float>>& spec = mFft.getSpectrum();
  for (int frame = 0; frame < spec.size(); ++frame) {
    mHPCP.push_back(std::vector<float>(NUM_PITCH_CLASSES, 0.0f));

    std::vector<float>& specFrame = mFft.getSpectrum()[frame];

    // Find local peaks to compute HPCP with
    std::vector<Peak>& peaks = getPeaks(MAX_SPEC_PEAKS, specFrame);

    if (threadShouldExit()) return;

    float curMax = 0.0;
    for (int i = 0; i < peaks.size(); ++i) {
      float peakFreq = ((peaks[i].binNum / (specFrame.size() - 1)) * mSampleRate) / 2;
      if (peakFreq < MIN_FREQ || peakFreq > MAX_FREQ) continue;

      // Create sum for each pitch class
      for (int pc = 0; pc < NUM_PITCH_CLASSES; ++pc) {
        float centerFreq =
            REF_FREQ * std::pow(2.0f, (pc + 1) / (float)NUM_PITCH_CLASSES);

        // Add contribution from each harmonic
        for (int hIdx = 0; hIdx < mHarmonicWeights.size(); ++hIdx) {
          float freq =
              peakFreq * pow(2., -mHarmonicWeights[hIdx].semitone / 12.0);
          float harmonicWeight = mHarmonicWeights[hIdx].gain;
          float d = std::fmod(12.0f * std::log2(freq / centerFreq), 12.0f);
          if (std::abs(d) <= (0.5f * HPCP_WINDOW_LEN)) {
            float w = std::pow(std::cos((M_PI * d) / HPCP_WINDOW_LEN), 2.0f);
            mHPCP[frame][pc] += (w * std::pow(peaks[i].gain, 2) *
                                 harmonicWeight * harmonicWeight);
            if (mHPCP[frame][pc] > curMax) curMax = mHPCP[frame][pc];
          }
        }
      }
    }

    // Normalize HPCP frame
    if (curMax > 0.0f) {
      for (int pc = 0; pc < NUM_PITCH_CLASSES; ++pc) {
        mHPCP[frame][pc] /= curMax;
      }
    }

    if (threadShouldExit()) return;
  }
}

void PitchDetector::segmentPitches() {
  if (mHPCP.empty()) return;

  mPitches.clear();

  // Initialize parameters
  int maxIdleFrames = mSampleRate * (MAX_IDLE_TIME_MS / 1000.0) / HOP_SIZE;
  int minNoteFrames = mSampleRate * (MIN_NOTE_TIME_MS / 1000.0) / HOP_SIZE;
  DBG("max dev: " << MAX_DEVIATION_BINS << ", max idle: " << maxIdleFrames << ", minNote: " << minNoteFrames << ", totalFrames: " << mHPCP.size());

  // Initialize the first segments using the first valid peaks
  std::vector<PitchDetector::Peak> firstPeaks;
  int startFrame = 0;
  for (int frame = 0; frame < mHPCP.size(); ++frame) {
    std::vector<PitchDetector::Peak> peaks =
        getPeaks(NUM_ACTIVE_SEGMENTS, mHPCP[frame]);
    if (!peaks.empty()) {
      auto peak = peaks.begin();
      // Keep pushing peaks until starting segments are full
      while (firstPeaks.size() < NUM_ACTIVE_SEGMENTS && peak != peaks.end()) {
        firstPeaks.push_back(*peak);
        startFrame++;
      }
    }
  }

  // Get a better audio clip bro
  if (firstPeaks.size() != NUM_ACTIVE_SEGMENTS) return;

  for (int i = 0; i < NUM_ACTIVE_SEGMENTS; ++i) {
    mSegments[i].binNum = firstPeaks[i].binNum;
    mSegments[i].salience = firstPeaks[i].gain;
  }

  // Calculate note trajectories through the clip
  for (int frame = startFrame; frame < mHPCP.size(); ++frame) {
    // Get the new pitch candidates
    std::vector<PitchDetector::Peak> peaks =
        getPeaks(NUM_ACTIVE_SEGMENTS, mHPCP[frame]);

    // Look for continuation candidates in peaks
    for (int i = 0; i < mSegments.size(); ++i) {
      PitchSegment& curSegment = mSegments[i];
      int closestIdx = -1;
      for (int j = 0; j < peaks.size(); ++j) {
        float devBins = std::abs(curSegment.binNum - peaks[j].binNum);
        if (devBins <= MAX_DEVIATION_BINS) {
          // Replace candidate if:
          if (closestIdx == -1) {  // It is the first one
            closestIdx = j;
          } else if (devBins <
                         std::abs(curSegment.binNum -
                                  peaks[closestIdx]
                                      .binNum) ||  // It is closer to the target
                     (peaks[closestIdx].binNum == peaks[j].binNum &&
                      (peaks[closestIdx].gain <
                       peaks[j].gain))) {  // It's tied for distance
                                           // but has a higher gain
            closestIdx = j;
          }
        }
      }

      if (closestIdx == -1) {
        // Mark segment as waiting for continuance
        if (curSegment.idleFrame == -1) curSegment.idleFrame = frame;
      } else {
        // Continue segment
        curSegment.idleFrame = -1;
        curSegment.binNum = peaks[closestIdx].binNum;
        curSegment.salience =
            peaks[closestIdx].gain;  // TODO: salience function
        peaks[closestIdx].binNum =
            INVALID_BIN;  // Mark peak so it isn't reused for multiple segments
      }

      // Check for segment expiration
      if (curSegment.idleFrame != -1 &&
          (frame - curSegment.idleFrame) > maxIdleFrames) {
        
        if (frame - curSegment.startFrame > minNoteFrames) {
          // Push to completed segments
          PitchClass pc = getPitchClass(curSegment.binNum);
          mPitches.push_back(
              Pitch(pc, (float)curSegment.startFrame / mHPCP.size(),
                                   frame - curSegment.startFrame,
                                   curSegment.salience));
          DBG("new note: " << pc << ", from frame: " << curSegment.startFrame
                           << ", duration: " << frame - curSegment.startFrame);
        }
        // Replace segment with new peak
        for (int j = 0; j < peaks.size(); ++j) {
          if (peaks[j].binNum != INVALID_BIN) {
            curSegment.startFrame = frame;
            curSegment.idleFrame = -1;
            curSegment.binNum = peaks[j].binNum;
            curSegment.salience = peaks[j].gain;
            break;
          }
        }
      }
    }
  }
}

PitchDetector::PitchClass PitchDetector::getPitchClass(float binNum) {
  int binsPerClass = NUM_PITCH_CLASSES / 12;
  int pc = (binNum / binsPerClass) + PITCH_CLASS_OFFSET;
  return (PitchClass)(pc % 12);
}

PitchDetector::Peak PitchDetector::interpolatePeak(int frame, int bin) {
  // Use quadratic interpolation to find peak freq and amplitude
  std::vector<std::vector<float>>& spec = mFft.getSpectrum();
  if (bin == 0 || bin == spec[frame].size() - 1) {
    return Peak((bin * mSampleRate) / FFT_SIZE, spec[frame][bin]);
  }
  float a = 20 * std::log10(spec[frame][bin - 1]);
  float b = 20 * std::log10(spec[frame][bin]);
  float c = 20 * std::log10(spec[frame][bin + 1]);

  float p = 0.5f * (a - c) / (a - (2.0f * b) + c);
  float interpBin = bin + p;
  float freq = (interpBin * mSampleRate) / FFT_SIZE;
  float gainDB = b - (0.25 * (a - c) * p);
  return Peak(freq, juce::jlimit(0.0, 1.0, std::pow(10, gainDB / 20.0f)));
}

// From essentia:
// Builds a weighting table of harmonic contribution. Higher harmonics
// contribute less and the fundamental frequency has a full harmonic
// strength of 1.0.
void PitchDetector::initHarmonicWeights() {
  mHarmonicWeights.clear();

  // Populate _harmonicPeaks with the semitonal positions of each of the
  // harmonics.
  for (int i = 0; i <= NUM_HARMONIC_WEIGHTS; i++) {
    float semitone = 12.0 * log2(i + 1.0);
    float octweight = std::max(1.0, (semitone / 12.0) * 0.5);

    // Get the semitone within the range
    // (0-HARMONIC_PRECISION, 12.0-HARMONIC_PRECISION]
    while (semitone >= 12.0 - HARMONIC_PRECISION) {
      semitone -= 12.0;
    }

    // Check to see if the semitone has already been added to weights
    std::vector<HarmonicWeight>::iterator it;
    for (it = mHarmonicWeights.begin(); it != mHarmonicWeights.end(); it++) {
      if ((*it).semitone > semitone - HARMONIC_PRECISION &&
          (*it).semitone < semitone + HARMONIC_PRECISION)
        break;
    }

    if (it == mHarmonicWeights.end()) {
      // no harmonic peak found for this frequency; add it
      mHarmonicWeights.push_back(HarmonicWeight(semitone, (1.0 / octweight)));
    } else {
      // else, add the weight
      (*it).gain += (1.0 / octweight);
    }
  }
}

std::vector<PitchDetector::Peak> PitchDetector::getPeaks(
    int numPeaks, std::vector<float>& frame) {
  int size = frame.size();
  const float scale = 1.0 / (float)(size - 1);

  std::vector<Peak> peaks;
  peaks.reserve(size);

  // we want to round up to the next integer instead of simple truncation,
  // otherwise the peak frequency at i can be lower than _minPos
  int i = 0;

  // first check the boundaries:
  if (i + 1 < size && frame[i] > frame[i + 1]) {
    if (frame[i] > MAGNITUDE_THRESHOLD) {
      peaks.push_back(Peak(i, frame[i]));
    }
  }

  while (true) {
    // going down
    while (i + 1 < size - 1 && frame[i] >= frame[i + 1]) {
      i++;
    }

    // now we're climbing
    while (i + 1 < size - 1 && frame[i] < frame[i + 1]) {
      i++;
    }

    // not anymore, go through the plateau
    int j = i;
    while (j + 1 < size - 1 && (frame[j] == frame[j + 1])) {
      j++;
    }

    // end of plateau, do we go up or down?
    if (j + 1 < size - 1 && frame[j + 1] < frame[j] &&
        frame[j] > MAGNITUDE_THRESHOLD) {  // going down again
      float resultBin = 0.0;
      float resultVal = 0.0;

      if (j != i) {  // plateau peak between i and j
        resultBin = (i + j) * 0.5;
        resultVal = frame[i];
      } else {  // interpolate peak at i-1, i and i+1
        interpolatePeak(frame[j - 1], frame[j], frame[j + 1], j, resultVal,
                        resultBin);
      }

      if (resultBin > size - 1) break;

      peaks.push_back(Peak(resultBin, resultVal));
    }

    // nothing found, start loop again
    i = j;

    if (i + 1 >= size - 1) {  // check the one just before the last position
      if (i == size - 2 && frame[i - 1] < frame[i] && frame[i + 1] < frame[i] &&
          frame[i] > MAGNITUDE_THRESHOLD) {
        float resultBin = 0.0;
        float resultVal = 0.0;
        interpolatePeak(frame[i - 1], frame[i], frame[i + 1], j, resultVal,
                        resultBin);
        peaks.push_back(Peak(resultBin, resultVal));
      }
      break;
    }
  }

  // check upper boundary here, so peaks are already sorted by position
  float pos = 1.0 / scale;
  if (size - 2 < pos && pos <= size - 1 && frame[size - 1] > frame[size - 2]) {
    if (frame[size - 1] > MAGNITUDE_THRESHOLD) {
      peaks.push_back(Peak((size - 1), frame[size - 1]));
    }
  }

  // we only want this many peaks
  int nWantedPeaks = juce::jmin(numPeaks, (int)peaks.size());
  std::sort(peaks.begin(), peaks.end(),
            [](Peak self, Peak other) { return self.gain > other.gain; });
  return std::vector<Peak>(peaks.begin(), peaks.begin() + nWantedPeaks);
}

/**
 * http://ccrma.stanford.edu/~jos/parshl/Peak_Detection_Steps_3.html
 *
 * Estimating the "true" maximum peak (frequency and magnitude) of the detected
 * local maximum using a parabolic curve-fitting. The idea is that the main-lobe
 * of spectrum of most analysis windows on a dB scale looks like a parabola and
 * therefore the maximum of a parabola fitted through a local maximum bin and
 * it's two neighboring bins will give a good approximation of the actual
 * frequency and magnitude of a sinusoid in the input signal.
 *
 * The parabola f(x) = a(x-n)^2 + b(x-n) + c can be completely described using 3
 * points; f(n-1) = A1, f(n) = A2 and f(n+1) = A3, where A1 = 20log10(|X(n-1)|),
 * A2 = 20log10(|X(n)|), A3 = 20log10(|X(n+1)|).
 *
 * Solving these equation yields: a = 1/2*A1 - A2 + 1/2*A3, b = 1/2*A3 - 1/2*A1
 * and c = A2.
 *
 * As the 3 bins are known to be a maxima, solving d/dx f(x) = 0, yields the
 * fractional bin position x of the estimated peak. Substituting delta_x for
 * (x-n) in this equation yields the fractional offset in bins from n where the
 * peak's maximum is.
 *
 * Solving this equation yields: delta_x = 1/2 * (A1 - A3)/(A1 - 2*A2 + A3).
 *
 * Computing f(n+delta_x) will estimate the peak's magnitude (in dB's):
 * f(n+delta_x) = A2 - 1/4*(A1-A3)*delta_x.
 */
void PitchDetector::interpolatePeak(const float leftVal, const float middleVal,
                                    const float rightVal, int currentBin,
                                    float& resultVal, float& resultBin) const {
  float delta_x =
      0.5 * ((leftVal - rightVal) / (leftVal - 2 * middleVal + rightVal));
  resultBin = currentBin + delta_x;
  resultVal = middleVal - 0.25 * (leftVal - rightVal) * delta_x;
}