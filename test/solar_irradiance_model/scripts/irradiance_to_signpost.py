#!/usr/bin/env python3

import sys
import os
import csv
import numpy as np
import pendulum
import datetime

try:
    import pysolar
except:
    print("Pysolar is required to calculate azimuth and zenith angles")
    print("Install the pysolar package")
    sys.exit(1)

if(len(sys.argv) < 3):
    print("You must pass a path to a PSM solar irradiance data file and an output file name")
    print("Data files should include DHI, DNI, GHI, C-DHI, C-DNI, C-GHI, Solar Zenith")
    sys.exit(1)


def azel_to_thetaphi(az, el):
    """ Az-El to Theta-Phi conversion.

    Args:
        az (float or np.array): Azimuth angle, in radians
        el (float or np.array): Elevation angle, in radians

    Returns:
      (theta, phi): Tuple of corresponding (theta, phi) angles, in radians
    """

    el = np.deg2rad(el)
    az = np.deg2rad(az)
    cos_theta = np.cos(el) * np.cos(az)
    tan_phi   = np.tan(el) / np.sin(az)
    theta     = np.arccos(cos_theta)
    phi       = np.arctan2(np.tan(el), np.sin(az))
    phi = (phi + 2 * np.pi) % (2 * np.pi)

    return np.rad2deg(theta), np.rad2deg(phi)


YEAR = 0
MONTH = 1
DAY = 2
HOUR = 3
MINUTE = 4
DHI = 5
DNI = 6
GHI = 7
CLEAR_DHI = 8
ZENITH = 12

input_fname = sys.argv[1]
output_fname = sys.argv[2]
print("Parsing data from " + input_fname)

with open(output_fname, 'w') as out:

    with open(input_fname, 'r') as f:
        reader = csv.reader(f)

        #skip the first header line
        head = next(reader)

        #grab the lat/lon from the next one
        head = next(reader)
        lat = float(head[5])
        lon = float(head[6])

        out.write('Signpost power data at ' + str(lat) + ',' + str(lon) + '\n')
        out.write('Date, South Power, West Power, North Power, East Power\n')
        #skip the last header line
        head = next(reader)
        blist = []
        lastel = -1
        br = False

        while True:
            rowlist = []
            while True:
                try:
                    row = next(reader)
                except:
                    sys.exit(1)

                #create a datetime
                time = pendulum.create(int(row[YEAR]),int(row[MONTH]),int(row[DAY]),int(row[HOUR]),int(row[MINUTE]),0,0)

                #get the recorded zenith
                #zenith = float(row[ZENITH])

                #get the elevation and azimuth from pysolar
                elevation = pysolar.solar.get_altitude(lat, lon, time)

                if (lastel > 0 and elevation < 0):
                    br = True

                lastel = elevation

                #get azimuth from pysolar
                azimuth = pysolar.solar.get_azimuth(lat, lon, time)

                #now transpose the elevation and azimuth calculations into a theta
                #angle for one of the cardinal directions
                #to do this flip around the azimuth (it is south referenced)
                #if the angle is > 90, there is effectively zero direct sunlight
                #so just cap it at 90 to make the cos(angle) calc easy

                #south
                southangle, nop = azel_to_thetaphi(azimuth, elevation)

                #east
                eastangle, nop = azel_to_thetaphi(azimuth-90, elevation)

                #west
                westangle, nop = azel_to_thetaphi(azimuth+90, elevation)

                #north
                northangle, nop = azel_to_thetaphi(azimuth-180, elevation)

                #now we can calculate the predicted solar irradiation for signpost on that
                #day
                #then we can multiply that my size and efficiency for the final prediction

                #predicted irradiation = DNI*cos(solar_angle) + DHI
                dni = float(row[DNI])
                dhi = float(row[DHI])

                angles = np.array([southangle, westangle, northangle, eastangle])

                factors = np.cos(np.deg2rad(angles))
                mask = factors < 0
                factors[mask] = 0

                rowlist.append([time, factors, dni, dhi, elevation])

                if(br == True):
                    br = False
                    break


            #calculate brightness
            blist = []
            lastelv = -1
            for r in rowlist:
                if(r[4] > 0):
                    if(r[2] > 70):
                        blist.append(1)
                    else:
                        blist.append(0)

            brightness = np.mean(np.array(blist))

            for r in rowlist:
                dni = r[2]
                dhi = r[3]
                time = r[0]
                factors = r[1]

                if(dhi > dni):
                    irradiances = dni*factors + dhi*0.5*brightness
                else:
                    irradiances = dni*factors + dhi*0.5

                #efficiency of our solar panel
                solar_eff = 0.17

                #size in m^2
                size = 0.096

                #predicted power output for signpost at this time in our input data
                powers = irradiances*size*solar_eff

                #now we should print this to the output csv
                out.write(time.strftime("%m/%d/%Y %H:%M") + ',' + str(powers[0]) + ',' + str(powers[1]) + ',' + str(powers[2]) + ',' + str(powers[3]) + '\n')
