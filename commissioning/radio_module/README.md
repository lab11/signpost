This commissioning script writes trusted root certificates to the radio module.

It takes them from the /certs folder and runs the correct AT commands to put
them onto a ublox sara U260.

To use the script hook a USB to serial converter up to the TX/RX pins of
the UBLOX SARA U260 on the radio module.

To add certificates, place them in the ./certs fold in der  or pem format.

At the end it prints the list of certificates installed on module.
