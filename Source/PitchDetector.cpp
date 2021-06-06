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
  // estimatePitches();
  if (onPitchesUpdated != nullptr && !threadShouldExit()) {
    // onPitchesUpdated(mHPCP);
    onPitchesUpdated(mHPCP);
  }
}

void PitchDetector::estimatePitches() {
  mPitchesTest.clear();
  for (int frame = 0; frame < mHPCP.size(); ++frame) {
    mPitchesTest.push_back(std::vector<float>(NUM_PITCH_CLASSES, 0.0f));
    for (int i = 0; i < NUM_PITCH_CLASSES; ++i) {
      if (mHPCP[frame][i] == 1.0f) {
        mPitchesTest[frame][i] = 1.0f;
      }
    }
  }
}

void PitchDetector::computeHPCP() {
  float lastGain = 0.0f;
  bool isIncreasing = true;

  mHPCP.clear();

  
  float threshold = std::powf(10.0f, DB_THRESH / 20.0f);
  std::vector<std::vector<float>>& spec = mFft.getSpectrum();
  for (int frame = 0; frame < spec.size(); ++frame) {
    mHPCP.push_back(std::vector<float>(NUM_PITCH_CLASSES, 0.0f));
    
    // Find local peaks to compute HPCP with
    std::vector<Peak> &peaks = getPeaks(frame);

    if (threadShouldExit()) return;

    float curMax = 0.0;
    for (int i = 0; i < peaks.size(); ++i) {
      float peakFreq = (peaks[i].binPos * mSampleRate) / 2;
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

std::vector<PitchDetector::Peak> PitchDetector::getPeaks(int frame) {
  std::vector<float>& specFrame = mFft.getSpectrum()[frame];
  int size = specFrame.size();
  const float scale = 1.0 / (float)(size - 1);

  std::vector<Peak> peaks;
  peaks.reserve(size);

  // we want to round up to the next integer instead of simple truncation,
  // otherwise the peak frequency at i can be lower than _minPos
  int i = 0;

  // first check the boundaries:
  if (i + 1 < size && specFrame[i] > specFrame[i + 1]) {
    if (specFrame[i] > MAGNITUDE_THRESHOLD) {
      peaks.push_back(Peak(i * scale, specFrame[i]));
    }
  }

  while (true) {
    // going down
    while (i + 1 < size - 1 && specFrame[i] >= specFrame[i + 1]) {
      i++;
    }

    // now we're climbing
    while (i + 1 < size - 1 && specFrame[i] < specFrame[i + 1]) {
      i++;
    }

    // not anymore, go through the plateau
    int j = i;
    while (j + 1 < size - 1 && (specFrame[j] == specFrame[j + 1])) {
      j++;
    }

    // end of plateau, do we go up or down?
    if (j + 1 < size - 1 && specFrame[j + 1] < specFrame[j] &&
        specFrame[j] > MAGNITUDE_THRESHOLD) {  // going down again
      float resultBin = 0.0;
      float resultVal = 0.0;

      if (j != i) {  // plateau peak between i and j
        resultBin = (i + j) * 0.5;
        resultVal = specFrame[i];
      } else {  // interpolate peak at i-1, i and i+1
        interpolatePeak(specFrame[j - 1], specFrame[j], specFrame[j + 1], j,
                        resultVal, resultBin);
      }

      float resultPos = resultBin * scale;

      if (resultPos > 1.0) break;

      peaks.push_back(Peak(resultPos, resultVal));
    }

    // nothing found, start loop again
    i = j;

    if (i + 1 >= size - 1) {  // check the one just before the last position
      if (i == size - 2 && specFrame[i - 1] < specFrame[i] &&
          specFrame[i + 1] < specFrame[i] && specFrame[i] > MAGNITUDE_THRESHOLD) {
        float resultBin = 0.0;
        float resultVal = 0.0;
        interpolatePeak(specFrame[i - 1], specFrame[i], specFrame[i + 1], j,
                        resultVal, resultBin);
        peaks.push_back(Peak(resultBin * scale, resultVal));
      }
      break;
    }
  }

  // check upper boundary here, so peaks are already sorted by position
  float pos = 1.0 / scale;
  if (size - 2 < pos && pos <= size - 1 &&
      specFrame[size - 1] > specFrame[size - 2]) {
    if (specFrame[size - 1] > MAGNITUDE_THRESHOLD) {
      peaks.push_back(Peak((size - 1) * scale, specFrame[size - 1]));
    }
  }

  // we only want this many peaks
  size_t nWantedPeaks = std::min((size_t)MAX_PEAKS, peaks.size());
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