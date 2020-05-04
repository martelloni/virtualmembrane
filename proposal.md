# Sensor-Controlled Digital Waveguide Model of Vibrating Drum Membrane

The task proposed is to implement the physical model of a
drum membrane running in real time on Bela, controlled by
a force, acceleration or displacement sensor fitted in a
wooden box the size of a small drum. This will serve as a
preliminary study for an implement on the
acoustic guitar, to be used for percussive interaction. For
this application we require real-time interaction at a satisfactory
sampling rate, and a model quality that allows expert level
percussion playing.

The model will be based on Fontana and Rocchesso's
digital waveguide approximation [Fontana 1998]. This paper
is the basis for the popular Sound Design Toolkit (SDT)
library in C
[Baldan 2017]; it also provides solutions to
many problems beyond the wave propagation model, such as
membrane excitation and air load in a real-world
cylindrical drum. There are two reasons that motivate
re-implementation from scratch. Firstly, the SDT currently
only provides a set "cartoonified" models, where
the parameters are tuned to maximise perceptual
effect for sound design; our goal on the other hand
is to achieve real-world consistency with a physical
drum, or at least optimise the perception from
the point of view of the percussionist. Moreover,
the excitation in the SDT models is event-based,
whereas our goal is to implement continuous control
from a sensor's signal. We are going to limit the
implementation to
a circular membrane with an air load.

The project will try to re-use materials I already
have with me. Wooden
boxes are quite easy to source even under lockdown
conditions, and the accelerometer
used in Assignment 2 may be re-purposed.
Any extra equipment or components
will be covered by the AIM
grant as this task is part of my PhD project.
Below is a provisional task breakdown with time
estimates in man-days:
- Review of extra useful sources, especially around
sensor mapping to physical quantities in the model.
E.g.: is it possible to map the accelerometer to
the acceleration/force in the excitation model? (1 day)
- Implement the model in standard C++11 and
write a test/simulation harness in Python using Boost.Python,
with the same procedure used in Assignment 1. (4 days)
- Build the box and code wrapping the signals on
Bela. Record a few seconds of drumming on the surface,
process it in the simulation, and plot/listen to effects.
Implement any possible mitigation around instabilities
in the activation, which seem to be an issue in
physical models. A change to a contact piezo transducer may
be considered at this stage. (3 days)
- Run the physical model on Bela, initially with
very conservative mesh sizes and sampling rate. Increase
or decrease precision of the model until we reach
a good compromise between real-time operation
and sound quality. Optimisation with SIMD intrinsics
may be a good tool to use here. (2 days)
- Video shooting and report write-up. (1 day)

Total time allocated: 11 days.

## References

[Fontana 1998]
Fontana, F. and Rocchesso, D., 1998. Physical modeling of membranes for percussion instruments. Acta Acustica united with Acustica, 84(3), pp.529-542.

[Baldan 2017]
Baldan, S., Delle Monache, S. and Rocchesso, D., 2017. The sound design toolkit. SoftwareX, 6, pp.255-260.