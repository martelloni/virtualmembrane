/*
 * Adapted from assignment2_drums to reuse I/O layout.
 *
 * Andrew McPherson, Becky Stewart and Victor Zappi
 * 2015-2020
 */


#include <Bela.h>
#include <cmath>

#include <libraries/Scope/Scope.h>

#include "DetectHit.hpp"
#include "Triangular2DMesh.hpp"
using meshcl = Triangular2DMesh;  // Because typing it every time was...

/* Global variables for current implementation */

/* Board layout (same as assignment 2) */
enum {
    kDigButton1 = 0,
    kDigButton2,
    kDigLED
};
enum {
    kAnalogPot = 0,
    kAnalogAccelX,
    kAnalogAccelY,
    kAnalogAccelZ,
};
static const unsigned int kNButtons = 2;
static const unsigned int kLEDCountLength = 100;  // Samples

/* Internal objects needed */
DetectHit hit;
char *meshmem;
meshcl *mesh;

// Oscilloscope for debugging
Scope gScope;

// setup() is called once before the audio rendering starts.
// Use it to perform any initialisation and allocation which is dependent
// on the period size or sample rate.
//
// userData holds an opaque pointer to a data structure that was passed
// in from the call to initAudio().
//
// Return true on success; returning false halts the program.

bool setup(BelaContext *context, void *userData)
{
    if (context->audioSampleRate != 44100.f) {
        // Warn if sample rate isn't supported - spatial sample rate
        // of mesh requires specific sampling rate. Multirate processing
        // is a marvellous thing but I need sleep.
        rt_printf("WARNING - Sample rate not set to 44.1 kHz!");
    }
    // Board-related inits: pots, GPIO, scope...
    gScope.setup(3, context->audioSampleRate);
    pinMode(context, 0, kDigButton1, INPUT);
    pinMode(context, 0, kDigButton2, INPUT);
    pinMode(context, 0, kDigLED, OUTPUT);

    // Internal setup
    hit.SetHPFCoef(0.9996439371675712, -0.9996439371675712, -0.9992878743351423);
    hit.SetLPFCoef(0.007073522215301396, 0.007073522215301396, -0.9858529555693972);

    // Mesh setup
    meshcl::Properties p {
        600.0,  // width mm
        33.0,  // height mm
        36.58996994711342,  // spatial sampling mm
    };
    // Mesh allocation and creation
    meshmem = new char[meshcl::GetMemSize(p)];
    mesh = new meshcl(p, meshmem);

    return true;
}

// render() is called regularly at the highest priority by the audio engine.
// Input and output are given from the audio hardware and the other
// ADCs and DACs (if available). If only audio is available, numMatrixFrames
// will be 0.

void render(BelaContext *context, void *userData)
{
    static int led_count = 0;  // LED used to signal hit to membrane

    for (unsigned int smp = 0; smp < context->audioFrames; smp++) {

        // Accelerometer reads
        float X_reading = analogRead(context, smp >> 1, kAnalogAccelX);
        float Y_reading = analogRead(context, smp >> 1, kAnalogAccelY);
        float Z_reading = analogRead(context, smp >> 1, kAnalogAccelZ);


        // Control code
        float pot_reading = analogRead(context, smp >> 1, kAnalogPot);
        bool but1_reading = digitalRead(context, smp, kDigButton1) == 0;
        bool but2_reading = digitalRead(context, smp, kDigButton2) == 0;

        // Audio code
        DetectHit::CurrentState accel_signal = hit.ProcessSample(Z_reading);
        float out = mesh->ProcessSample(true, accel_signal.value);
        // Mesh output seems quiiiiiiiet...
        out *= 10.;

        // Scope
        gScope.log(Z_reading, accel_signal.value, out);

        // LED code: count a few samples for each action just to make it go Ping!!!
        if (false) {
            led_count = kLEDCountLength;
        }
        if (led_count > 0) {
            led_count--;
        }
        digitalWriteOnce(context, smp, kDigLED, led_count > 0);

        // Prevent clipping for now, TODO sigmoid waveshaping for crunchy coolness
        float pad_gain = 1.;
        audioWrite(context, smp, 0, out * pad_gain);
        audioWrite(context, smp, 1, out * pad_gain);

    }
}


// cleanup_render() is called once at the end, after the audio has stopped.
// Release any resources that were allocated in initialise_render().

void cleanup(BelaContext *context, void *userData)
{
    delete mesh;
    delete meshmem;
}
