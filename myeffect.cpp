#include "emscripten/bind.h"
#include <math.h>

using namespace emscripten;

const unsigned kRenderQuantumFrames = 128;
const unsigned kBytesPerChannel = kRenderQuantumFrames * sizeof(float);

class Distortion {
 public:
  Distortion() {}

  void setDistortion (float newAmt) { distortion = newAmt; }
  float getDistortion() const { return distortion; }

  void setCutoff (float newAmt) { cutoff = newAmt; }
  float getCutoff() const { return cutoff; }

  void setBypass (bool newBypass) { bypass = newBypass; }
  bool getBypass() const { return bypass; }

  float vcf (float x, float Fc, float res)
  {
    Fc /= (48000.f / 2.f);
    float f = Fc * 1.16f;
    float fb = res * (1.0f - 0.15f * f * f);
    x -= out4 * fb;
    x *= 0.35013f * (f*f)*(f*f);
    out1 = x + 0.3f * in1 + (1 - f) * out1; // Pole 1
    in1 = x;
    out2 = out1 + 0.3f * in2 + (1 - f) * out2; // Pole 2
    in2 = out1;
    out3 = out2 + 0.3f * in3 + (1 - f) * out3; // Pole 3
    in3 = out2;
    out4 = out3 + 0.3f * in4 + (1 - f) * out4; // Pole 4
    in4 = out3;
    return out4 * 1.414427157f;
  }

  void Process(uintptr_t input_ptr, uintptr_t output_ptr, unsigned channel_count)
  {
    float* input_buffer = reinterpret_cast<float*>(input_ptr);
    float* output_buffer = reinterpret_cast<float*>(output_ptr);

    // Bypasses the data. By design, the channel count will always be the same
    // for |input_buffer| and |output_buffer|.
    for (unsigned channel = 0; channel < channel_count; ++channel)
    {
      float* destination = output_buffer + channel * kRenderQuantumFrames;
      float* source = input_buffer + channel * kRenderQuantumFrames;
      
      for (int i = 0; i < kRenderQuantumFrames; i++)
      {
        float x = source[i];
        x = vcf (x, cutoff, 0.707f);

        x *= (0.988592f * pow(1.035395f, distortion));		//x = x*a*b^distortion
	float alpha = 1.0f;

	float beta = pow(alpha, (5 / 2));
        
        if (bypass)
          destination[i] = source[i];
        else
	  destination[i] = fmin((alpha * x / pow((1 + beta*pow((abs(x)), 2.505f)), (1 / 2.494f))), 0.9999f);
      }

      //memcpy(destination, source, kBytesPerChannel);
    }
  }

  private:
    float distortion = 100.0f;
    float cutoff = 4000.0f;
    bool bypass = false;

    float out1 = 0.0f;
    float out2 = 0.0f;
    float out3 = 0.0f;
    float out4 = 0.0f;
    float in1 = 0.0f;
    float in2 = 0.0f;
    float in3 = 0.0f;
    float in4 = 0.0f;
};

EMSCRIPTEN_BINDINGS(CLASS_Distortion) {
  class_<Distortion>("Distortion")
      .constructor()
      .function("process",
                &Distortion::Process,
                allow_raw_pointers())
      .property("distortion",
                &Distortion::getDistortion,
                &Distortion::setDistortion)
      .property("cutoff",
	        &Distortion::getCutoff,
	        &Distortion::setCutoff)
      .property("bypass",
                &Distortion::getBypass,
                &Distortion::setBypass);
}

