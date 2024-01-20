/*
  ==============================================================================

    HPCP.cpp
    Created: 11 Jan 2024 9:01:42pm
    Author:  brady

  ==============================================================================
*/

#include "HPCP.h"

void HPCP::clear() {
  mHPCP.clear();
}

Utils::SpecBuffer* HPCP::process(Utils::SpecBuffer* spec, double sampleRate) {
  if (!spec) return nullptr;
  mHPCP.clear();
  mSampleRate = sampleRate;

  for (size_t frame = 0; frame < spec->size(); ++frame) {
//    updateProgress(mStartProgress + (mDiffProgress * (static_cast<double>(frame) / static_cast<double>(spec->size()))));
    mHPCP.emplace_back(std::vector<float>(NUM_HPCP_BINS, 0.0f));

    std::vector<float>& specFrame = (*spec)[frame];

    // Find local peaks to compute HPCP with
    std::vector<Peak> peaks = getPeaks(MAX_SPEC_PEAKS, specFrame);

    float curMax = 0.0;
    for (size_t i = 0; i < peaks.size(); ++i) {
      float peakFreq = ((peaks[i].binNum / (specFrame.size() - 1)) * mSampleRate) / 2;
      if (peakFreq < MIN_FREQ || peakFreq > MAX_FREQ) continue;

      // Create sum for each pitch class
      for (int pc = 0; pc < NUM_HPCP_BINS; ++pc) {
        int pcIdx = (pc + PITCH_CLASS_OFFSET_BINS) % NUM_HPCP_BINS;
        float centerFreq = REF_FREQ * std::pow(2.0f, pc / (float)NUM_HPCP_BINS);

        // Add contribution from each harmonic
        for (size_t hIdx = 0; hIdx < mHarmonicWeights.size(); ++hIdx) {
          float freq = peakFreq * pow(2., -mHarmonicWeights[hIdx].semitone / 12.0);
          float harmonicWeight = mHarmonicWeights[hIdx].gain;
          float d = std::fmod(12.0f * std::log2(freq / centerFreq), 12.0f);
          if (std::abs(d) <= (0.5f * HPCP_WINDOW_LEN)) {
            float w = std::pow(std::cos((juce::MathConstants<float>::pi * d) / HPCP_WINDOW_LEN), 2.0f);
            mHPCP[frame][pcIdx] += (w * std::pow(peaks[i].gain, 2) * harmonicWeight * harmonicWeight);
            if (mHPCP[frame][pcIdx] > curMax) curMax = mHPCP[frame][pcIdx];
          }
        }
      }
    }

    // Normalize HPCP frame and clear low energy frames
    float totalEnergy = 0.0f;
    if (curMax > 0.0f) {
      for (int pc = 0; pc < NUM_HPCP_BINS; ++pc) {
        totalEnergy += mHPCP[frame][pc];
        mHPCP[frame][pc] /= curMax;
      }
    }
    if (totalEnergy / NUM_HPCP_BINS < MIN_AVG_FRAME_ENERGY) {
      std::fill(mHPCP[frame].begin(), mHPCP[frame].end(), 0.0f);
    }
  }
  return &mHPCP;
}

// From essentia:
// Builds a weighting table of harmonic contribution. Higher harmonics
// contribute less and the fundamental frequency has a full harmonic
// Strength of 1.0.
void HPCP::initHarmonicWeights() {
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
      if ((*it).semitone > semitone - HARMONIC_PRECISION && (*it).semitone < semitone + HARMONIC_PRECISION) break;
    }

    if (it == mHarmonicWeights.end()) {
      // no harmonic peak found for this frequency; add it
      mHarmonicWeights.emplace_back(HarmonicWeight(semitone, (1.0f / octweight)));
    } else {
      // else, add the weight
      (*it).gain += (1.0 / octweight);
    }
  }
}

std::vector<HPCP::Peak> HPCP::getPeaks(int numPeaks, std::vector<float>& frame) {
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
      peaks.emplace_back(Peak(i, frame[i]));
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
    if (j + 1 < size - 1 && frame[j + 1] < frame[j] && frame[j] > MAGNITUDE_THRESHOLD) {  // going down again
      float resultBin = 0.0;
      float resultVal = 0.0;

      if (j != i) {  // plateau peak between i and j
        resultBin = (i + j) * 0.5;
        resultVal = frame[i];
      } else {  // interpolate peak at i-1, i and i+1
        interpolatePeak(frame[j - 1], frame[j], frame[j + 1], j, resultVal, resultBin);
      }

      if (resultBin > size - 1) break;

      peaks.emplace_back(Peak(resultBin, resultVal));
    }

    // nothing found, start loop again
    i = j;

    if (i + 1 >= size - 1) {  // check the one just before the last position
      if (i == size - 2 && frame[i - 1] < frame[i] && frame[i + 1] < frame[i] && frame[i] > MAGNITUDE_THRESHOLD) {
        float resultBin = 0.0;
        float resultVal = 0.0;
        interpolatePeak(frame[i - 1], frame[i], frame[i + 1], j, resultVal, resultBin);
        peaks.emplace_back(Peak(resultBin, resultVal));
      }
      break;
    }
  }

  // check upper boundary here, so peaks are already sorted by position
  float pos = 1.0 / scale;
  if (size - 2 < pos && pos <= size - 1 && frame[size - 1] > frame[size - 2]) {
    if (frame[size - 1] > MAGNITUDE_THRESHOLD) {
      peaks.emplace_back(Peak((size - 1), frame[size - 1]));
    }
  }

  // we only want this many peaks
  int nWantedPeaks = juce::jmin(numPeaks, (int)peaks.size());
  std::sort(peaks.begin(), peaks.end(), [](Peak self, Peak other) { return self.gain > other.gain; });
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
void HPCP::interpolatePeak(const float leftVal, const float middleVal, const float rightVal, int currentBin,
                                    float& resultVal, float& resultBin) const {
  float delta_x = 0.5 * ((leftVal - rightVal) / (leftVal - 2 * middleVal + rightVal));
  resultBin = currentBin + delta_x;
  resultVal = middleVal - 0.25 * (leftVal - rightVal) * delta_x;
}
