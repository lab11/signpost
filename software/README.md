Signpost Software
=================

Software for the Signpost platform and its modules.

Although not a requirement for modules, most hardware on signpost uses the
[Tock](https://github.com/helena-project/tock) operating system. Tock is an
embedded operating system targeted towards low-power ARM Cortex-M class devices
that provides safety and robustness guarantees. The Tock kernel is written in
Rust, but application code can be written in any language that can be compiled
to a valid binary. Most Signpost boards have applications written in C, with a
few written in C++.

## Apps

Tock application code for Signpost platform and modules.

## Docs

Documentation on various aspects of Signpost software. 

## Edison

Tools for loading a Signpost image onto an Intel Edison, such as exists on the
Control Module.

## Kernel

Tock kernel code for Signpost platform and modules. Get started [here](kernel/).

## Receiver

Various methods for collecting data from Signposts.

## Signbus

Tools for interacting directly with the I2C bus on a Signpost Backplane.

