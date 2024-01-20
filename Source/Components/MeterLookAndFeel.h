#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ff_meters/ff_meters.h"
#include "Utils/Colour.h"

class MeterLookAndFeel : public foleys::LevelMeterLookAndFeel {
 public:
  MeterLookAndFeel() {}

 private:
  // Disables meter tick marks
  juce::Rectangle<float> getMeterTickmarksBounds(juce::Rectangle<float>, foleys::LevelMeter::MeterFlags) const override {
    return juce::Rectangle<float>();
  }

  // Disables meter clip light
  juce::Rectangle<float> getMeterClipIndicatorBounds(juce::Rectangle<float>, foleys::LevelMeter::MeterFlags) const override {
    return juce::Rectangle<float>();
  }

  // Disable meter max number
  juce::Rectangle<float> getMeterMaxNumberBounds(juce::Rectangle<float>, foleys::LevelMeter::MeterFlags) const override {
    return juce::Rectangle<float>();
  }

  // Just use the regular bounds, no margins
  juce::Rectangle<float> getMeterBarBounds(juce::Rectangle<float> bounds, foleys::LevelMeter::MeterFlags) const override {
    return bounds;
  }
  
  void drawMeterBar (juce::Graphics& g,
                     foleys::LevelMeter::MeterFlags meterType,
                     juce::Rectangle<float> bounds,
                     float rms, float peak) override
  {
    const auto infinity = meterType & foleys::LevelMeter::Reduction ? -30.0f :  -100.0f;
    const auto rmsDb  = juce::Decibels::gainToDecibels (rms,  infinity);
    const auto peakDb = juce::Decibels::gainToDecibels (peak, infinity);
    
    const juce::Rectangle<float> floored (ceilf (bounds.getX()) + 1.0f, ceilf (bounds.getY()) + 1.0f,
                                          floorf (bounds.getRight()) - (ceilf (bounds.getX() + 2.0f)),
                                          floorf (bounds.getBottom()) - (ceilf (bounds.getY()) + 2.0f));
    
    if (meterType & foleys::LevelMeter::Vintage) {
      // TODO
    }
    else if (meterType & foleys::LevelMeter::Reduction)
    {
      const float limitDb = juce::Decibels::gainToDecibels (rms, infinity);
      g.setColour (findColour (foleys::LevelMeter::lmMeterReductionColour));
      if (meterType & foleys::LevelMeter::Horizontal)
        g.fillRect (floored.withLeft (floored.getX() + limitDb * floored.getWidth() / infinity));
      else
        g.fillRect (floored.withBottom (floored.getY() + limitDb * floored.getHeight() / infinity));
    }
    else
    {
      if (meterType & foleys::LevelMeter::Horizontal)
      {
        jassert(false); // If you get here, I only added the rainbow meter vertical implementation. You'll have to do the same for horizontal if you want to use it
//        if (horizontalGradient.getNumColours() < 2)
//        {
//          horizontalGradient = juce::ColourGradient (findColour (foleys::LevelMeter::lmMeterGradientLowColour),
//                                                     floored.getX(), floored.getY(),
//                                                     findColour (foleys::LevelMeter::lmMeterGradientMaxColour),
//                                                     floored.getRight(), floored.getY(), false);
//          horizontalGradient.addColour (0.5, findColour (foleys::LevelMeter::lmMeterGradientLowColour));
//          horizontalGradient.addColour (0.75, findColour (foleys::LevelMeter::lmMeterGradientMidColour));
//        }
//        g.setGradientFill (horizontalGradient);
//        g.fillRect (floored.withRight (floored.getRight() - rmsDb * floored.getWidth() / infinity));
//        
//        if (peakDb > -49.0)
//        {
//          g.setColour (findColour ((peakDb > -0.3f) ? foleys::LevelMeter::lmMeterMaxOverColour :
//                                   ((peakDb > -5.0) ? foleys::LevelMeter::lmMeterMaxWarnColour :
//                                    foleys::LevelMeter::lmMeterMaxNormalColour)));
//          g.drawVerticalLine (juce::roundToInt (floored.getRight() - juce::jmax (peakDb * floored.getWidth() / infinity, 0.0f)),
//                              floored.getY(), floored.getBottom());
//        }
      }
      else
      {
        // vertical
        if (verticalGradient.getNumColours() < 2)
        {
          verticalGradient = juce::ColourGradient(Utils::getRainbow12Colour(11),
                                                   floored.getX(), floored.getBottom(),
                                                  Utils::getRainbow12Colour(0),
                                                   floored.getX(), floored.getY(), false);
          for (int i = 1; i < 12; ++i) {
            verticalGradient.addColour((12 - i) / 12.0, Utils::getRainbow12Colour(i));
          }
        }
        g.setGradientFill (verticalGradient);
        g.fillRect (floored.withTop (floored.getY() + rmsDb * floored.getHeight() / infinity));
        
        if (peakDb > -49.0f) {
          g.setColour (findColour ((peakDb > -0.3f) ? foleys::LevelMeter::lmMeterMaxOverColour :
                                   ((peakDb > -5.0f) ? foleys::LevelMeter::lmMeterMaxWarnColour :
                                    foleys::LevelMeter::lmMeterMaxNormalColour)));
          g.drawHorizontalLine (juce::roundToInt (floored.getY() + juce::jmax (peakDb * floored.getHeight() / infinity, 0.0f)),
                                floored.getX(), floored.getRight());
        }
      }
    }
  }
  
  juce::ColourGradient verticalGradient;

};
