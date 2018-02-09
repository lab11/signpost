#!/usr/bin/env python3
import sys
import os
import csv
from multiprocessing import Pool
import pysolar
import pendulum
import numpy as np

def irr_parser(inputs):
    print(inputs)
    input_fname = inputs[0]
    output_fname = inputs[1]
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

            for row in reader:
                #create a datetime
                time = pendulum.create(int(row[YEAR]),int(row[MONTH]),int(row[DAY]),int(row[HOUR]),int(row[MINUTE]),0,0)

                #get the recorded zenith
                #zenith = float(row[ZENITH])

                #get the elevation and azimuth from pysolar
                elevation = pysolar.solar.get_altitude(lat, lon, time)

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

                if(dhi > dni):
                    irradiances = dni*factors + dhi*factors
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

def averager(inputs):
    print(inputs)
    input_fname = inputs[0]
    output_fname = inputs[1]
    days = inputs[2]
    with open(output_fname, 'w') as out:
        with open(input_fname, 'r') as inp:
            reader = csv.reader(inp)

            #skip the first header line
            head = next(reader)
            out.write(head[0] + ',' + head[1] + '\n')
            head = next(reader)
            out.write(head[0] + ',' + head[1] + ',' + head[2] + ',' + head[3] + ',' + head[4] + '\n')

            avlist = [0,0,0,0]
            tavlist = []
            count = 0
            first = True
            for row in reader:
                dt = row[0]
                time = pendulum.strptime(dt,"%m/%d/%Y %H:%M")
                if(first):
                    last_time = time
                    first = False

                if((time - last_time).in_days() >= int(days)):
                    out.write(last_time.strftime("%m/%d/%Y %H:%M") + ',' + str(avlist[0]/count) + ',' + str(avlist[1]/count) + ',' + str(avlist[2]/count) + ',' + str(avlist[3]/count) + '\n')
                    last_time = time
                    count = 1
                    powers = [float(row[1]),float(row[2]),float(row[3]),float(row[4])]
                    avlist[0] = powers[0]
                    avlist[1] = powers[1]
                    avlist[2] = powers[2]
                    avlist[3] = powers[3]
                else:
                    powers = [float(row[1]),float(row[2]),float(row[3]),float(row[4])]
                    avlist[0] += powers[0]
                    avlist[1] += powers[1]
                    avlist[2] += powers[2]
                    avlist[3] += powers[3]
                    count += 1
            out.write(last_time.strftime("%m/%d/%Y %H:%M") + ',' + str(avlist[0]/count) + ',' + str(avlist[1]/count) + ',' + str(avlist[2]/count) + ',' + str(avlist[3]/count) + '\n')

if __name__ == '__main__':

    if(len(sys.argv) < 3):
        print("You must pass a input data file and date ranges into this script")
        sys.exit(1)

    input_folder = sys.argv[1]
    output_folder = sys.argv[2]
    if not os.path.exists(output_folder):
        os.mkdir(output_folder)

    infiles = []
    for dirpath,_,filenames in os.walk(input_folder):
        for f in filenames:
            infiles.append(os.path.abspath(os.path.join(dirpath, f)))

    outfiles1 = []
    outfiles2 = []
    if not os.path.exists('scratch1'):
        os.mkdir('scratch1')

    i = 0
    for f in infiles:
        outfiles1.append('scratch1/out'+str(i)+'.csv')
        outfiles2.append(output_folder + '/out'+str(i)+'.csv')
        i += 1

    args = []
    for i in range(0,len(infiles)):
        args.append([infiles[i],outfiles1[i]])

    with Pool(20) as p:
        p.map(irr_parser, args)

    args = []
    for i in range(0,len(outfiles1)):
        args.append([outfiles1[i],outfiles2[i],7])

    with Pool(20) as p:
        p.map(averager, args)
