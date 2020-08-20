/*
  Copyright 2006-2016 David Robillard <d@drobilla.net>
  Copyright 2006 Steve Harris <steve@plugin.org.uk>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

/**
   LV2 headers are based on the URI of the specification they come from, so a
   consistent convention can be used even for unofficial extensions.  The URI
   of the core LV2 specification is <http://lv2plug.in/ns/lv2core>, by
   replacing `http:/` with `lv2` any header in the specification bundle can be
   included, in this case `lv2.h`.
*/

#if defined(LV2_PLUGIN)

extern "C" {

#include "lv2/core/lv2.h"

/** Include standard C headers */
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

/**
   The URI is the identifier for a plugin, and how the host associates this
   implementation in code with its description in data.  In this plugin it is
   only used once in the code, but defining the plugin URI at the top of the
   file is a good convention to follow.  If this URI does not match that used
   in the data files, the host will fail to load the plugin.
*/
#define AMP_URI "http://lv2plug.in/plugins/eg-amp"

static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
            double                    rate,
            const char*               bundle_path,
            const LV2_Feature* const* features);

static void
connect_port(LV2_Handle instance,
             uint32_t   port,
             void*      data);

static void
activate(LV2_Handle instance);

static void
run(LV2_Handle instance, uint32_t n_samples);

static void
deactivate(LV2_Handle instance);

static void
cleanup(LV2_Handle instance);

static const void*
extension_data(const char* uri);

LV2_SYMBOL_EXPORT
const LV2_Descriptor*
lv2_descriptor(uint32_t index);

}  // extern "C"

#include "mesh/Triangular2DMesh.hpp"
#include "dsp/Filter.hpp"
#include "dsp/FilterDesigner.hpp"


/**
   In code, ports are referred to by index.  An enumeration of port indices
   should be defined for readability.
*/
typedef enum {
   AMP_ATTENUATION = 0,
   AMP_INPUT_THRESHOLD,
	AMP_GAIN,
	AMP_INPUT,
	AMP_OUTPUT
} PortIndex;

/**
   Every plugin defines a private structure for the plugin instance.  All data
   associated with a plugin instance is stored here, and is available to
   every instance method.  In this simple plugin, only port buffers need to be
   stored, since there is no additional instance data.
*/
typedef struct {
   // Audio plugin structure
   char *meshmem_ptr;
   Triangular2DMesh *mesh_ptr;
   DSP::BiquadCoeffs crossover_lpf_c;
   DSP::BiquadCoeffs crossover_hpf_c;
   DSP::Biquad<1>::State *crossover_lpf_s;
   DSP::Biquad<1>::State *crossover_hpf_s;
   DSP::Biquad<1> *crossover_lpf;
   DSP::Biquad<1> *crossover_hpf;
   DSP::ARSmoother *ar_smoother;
	// Port buffers
   const float* attenuation;
   const float* input_threshold;
	const float* gain;
	const float* input;
	float*       output;
} Amp;

Triangular2DMesh::Properties mesh_properties {
   300.f,
   300.f,
   10.f,
};

/**
   The `instantiate()` function is called by the host to create a new plugin
   instance.  The host passes the plugin descriptor, sample rate, and bundle
   path for plugins that need to load additional resources (e.g. waveforms).
   The features parameter contains host-provided features defined in LV2
   extensions, but this simple plugin does not use any.

   This function is in the ``instantiation'' threading class, so no other
   methods on this instance will be called concurrently with it.
*/
static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
            double                    rate,
            const char*               bundle_path,
            const LV2_Feature* const* features)
{
	Amp* amp = (Amp*)calloc(1, sizeof(Amp));

   // Allocate and instantiate mesh
   amp->meshmem_ptr = new char[Triangular2DMesh::GetMemSize(mesh_properties)];
   amp->mesh_ptr = new Triangular2DMesh(mesh_properties, amp->meshmem_ptr);

   // Filter design/allocation
   const float fcut = 100.f;
   DSP::FilterDesigner::ResonantLowpass(&amp->crossover_lpf_c, rate, fcut, 0.5f, 1.f);
   amp->crossover_lpf_s = new DSP::Biquad<1>::State[1];
   amp->crossover_lpf = new DSP::Biquad<1>(1, &amp->crossover_lpf_c,
         amp->crossover_lpf_s);

   DSP::FilterDesigner::ResonantHighpass(&amp->crossover_hpf_c, rate, fcut, 0.5f, 1.f);
   amp->crossover_hpf_s = new DSP::Biquad<1>::State[1];
   amp->crossover_hpf = new DSP::Biquad<1>(1, &amp->crossover_hpf_c,
         amp->crossover_hpf_s);

   // Smoother design/allocation
   const float alpha_a = 0.01;
   const float alpha_r = 0.001;
   amp->ar_smoother = new DSP::ARSmoother(alpha_a, alpha_r);

	return (LV2_Handle)amp;
}

/**
   The `connect_port()` method is called by the host to connect a particular
   port to a buffer.  The plugin must store the data location, but data may not
   be accessed except in run().

   This method is in the ``audio'' threading class, and is called in the same
   context as run().
*/
static void
connect_port(LV2_Handle instance,
             uint32_t   port,
             void*      data)
{
	Amp* amp = (Amp*)instance;

	switch ((PortIndex)port) {
	case AMP_ATTENUATION:
		amp->attenuation = (const float*)data;
		break;
	case AMP_INPUT_THRESHOLD:
		amp->input_threshold = (const float*)data;
		break;
	case AMP_GAIN:
		amp->gain = (const float*)data;
		break;
	case AMP_INPUT:
		amp->input = (const float*)data;
		break;
	case AMP_OUTPUT:
		amp->output = (float*)data;
		break;
	}
}

/**
   The `activate()` method is called by the host to initialise and prepare the
   plugin instance for running.  The plugin must reset all internal state
   except for buffer locations set by `connect_port()`.  Since this plugin has
   no other internal state, this method does nothing.

   This method is in the ``instantiation'' threading class, so no other
   methods on this instance will be called concurrently with it.
*/
static void
activate(LV2_Handle instance)
{
	const Amp* amp = (const Amp*)instance;
   amp->mesh_ptr->Reset();
   amp->crossover_lpf->Reset();
   amp->crossover_hpf->Reset();
   amp->ar_smoother->Reset();
}

/** Define a macro for converting a gain in dB to a coefficient. */
#define DB_CO(g) ((g) > -90.0f ? powf(10.0f, (g) * 0.05f) : 0.0f)

/**
   The `run()` method is the main process function of the plugin.  It processes
   a block of audio in the audio context.  Since this plugin is
   `lv2:hardRTCapable`, `run()` must be real-time safe, so blocking (e.g. with
   a mutex) or memory allocation are not allowed.
*/
static void
run(LV2_Handle instance, uint32_t n_samples)
{
	const Amp* amp = (const Amp*)instance;

   // Parameter retrieval
	const float gain = *(amp->gain);
	const float attenuation = *(amp->attenuation);
	const float input_threshold = *(amp->input_threshold);
	const float* const input  = amp->input;
	float* const       output = amp->output;

   // Pass parameters
	const float coef = DB_CO(gain);
   amp->mesh_ptr->SetAttenuation(attenuation);
   const float thresh = DB_CO(input_threshold);

   // Per sample execution
	for (uint32_t pos = 0; pos < n_samples; pos++) {
      float x, y;
      x = input[pos];
      y = x;
      // Crossover
      amp->crossover_hpf->ProcessFrame(&y);
      amp->crossover_lpf->ProcessFrame(&x);
      // Signal conditioning
      x = std::abs(x);
      x = amp->ar_smoother->ProcessSample(x);
      x -= thresh;  // Subtract threshold and rectify
      if (x < 0) {
         x = 0;
      }
      // White noise!
      float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
      r *= 2.f;
      r -= 1.f;
      x = r * x;
      // Mesh execution
		x = coef * amp->mesh_ptr->ProcessSample(true, x);
      // Mix back
      //output[pos] = x + y;
      output[pos] = x;
	}
}

/**
   The `deactivate()` method is the counterpart to `activate()`, and is called by
   the host after running the plugin.  It indicates that the host will not call
   `run()` again until another call to `activate()` and is mainly useful for more
   advanced plugins with ``live'' characteristics such as those with auxiliary
   processing threads.  As with `activate()`, this plugin has no use for this
   information so this method does nothing.

   This method is in the ``instantiation'' threading class, so no other
   methods on this instance will be called concurrently with it.
*/
static void
deactivate(LV2_Handle instance)
{
}

/**
   Destroy a plugin instance (counterpart to `instantiate()`).

   This method is in the ``instantiation'' threading class, so no other
   methods on this instance will be called concurrently with it.
*/
static void
cleanup(LV2_Handle instance)
{
   const Amp* amp = (const Amp*)instance;
   delete amp->mesh_ptr;
   delete amp->meshmem_ptr;
   delete amp->crossover_hpf;
   delete amp->crossover_hpf_s;
   delete amp->crossover_lpf;
   delete amp->crossover_lpf_s;
   delete amp->ar_smoother;
	free(instance);
}

/**
   The `extension_data()` function returns any extension data supported by the
   plugin.  Note that this is not an instance method, but a function on the
   plugin descriptor.  It is usually used by plugins to implement additional
   interfaces.  This plugin does not have any extension data, so this function
   returns NULL.

   This method is in the ``discovery'' threading class, so no other functions
   or methods in this plugin library will be called concurrently with it.
*/
static const void*
extension_data(const char* uri)
{
	return NULL;
}

/**
   Every plugin must define an `LV2_Descriptor`.  It is best to define
   descriptors statically to avoid leaking memory and non-portable shared
   library constructors and destructors to clean up properly.
*/
static const LV2_Descriptor descriptor = {
	AMP_URI,
	instantiate,
	connect_port,
	activate,
	run,
	deactivate,
	cleanup,
	extension_data
};

/**
   The `lv2_descriptor()` function is the entry point to the plugin library.  The
   host will load the library and call this function repeatedly with increasing
   indices to find all the plugins defined in the library.  The index is not an
   indentifier, the URI of the returned descriptor is used to determine the
   identify of the plugin.

   This method is in the ``discovery'' threading class, so no other functions
   or methods in this plugin library will be called concurrently with it.
*/
LV2_SYMBOL_EXPORT
const LV2_Descriptor*
lv2_descriptor(uint32_t index)
{
	switch (index) {
	case 0:  return &descriptor;
	default: return NULL;
	}
}

#endif  // defined(LV2_PLUGIN)
