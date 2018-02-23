Plans for making a Gas and Particulate Sensor
=============================================

We have been theorizing about making a module that senses key indicators
of outdoor air quality through the duration of the project, and air quality
is a key motivator many of Signpost's potentially 'killer applications'.
These include dynamic rerouting of traffic to shift hot-spots for poor 
air quality and pedestrian routing around these locations along with simpler
long term, highly granular air quality measurement to test the hypothesis
that air quality can change the lives of city residents block-by-bock. 
We also had high particulate counts in Berkeley in the relatively
recent past due to the North Bay fires, and it would be nice to measure
how equally this air quality is impacting the city.

That being said, we have held off on creating an air quality sensor because
they are notoriously difficult to produce with high accuracy. 
[EPA and AQMD reports](http://www.aqmd.gov/aq-spec/sensors)
largely support our skepticism, with many air quality sensors less than $1000-$2000
being very poor indicators of the key metrics in the space. The desire to have
an air quality module hasn't subsided, however, and what follows is a high level
analysis of the potential sensors we can use to create an easy, cheap, and
relatively accurate air quality monitor for the platform. 

##What to Measure?
A survey of outdoor air quality sensors yielded:
 - Particulate (usually PM2.5)
 - Ozone (O~3)
 - Nitrogen Dioxide (NO~2)
 - Carbon Monoxide (CO)
 - Sulfur Dioxide (SO~2)

Of these, EPA reports most focus on particulate matter, 
ozone, and nitrogen dioxide, with carbon monoxide and sulfur dioxide taking
less of a focus. This is not the most concrete scientific evidence for
choosing these gasses, but to effectively implement this module without
too much effort, the sensor should be limited to those which have clear and 
working examples.

###PM2.5 and  Other Particulate

PM2.5 sensors found in most cheaper air quality monitors are produced by
[Shinyei](http://www.shinyei.co.jp/stc/eng/optical/index.html). These
sensors seem to very greatly in accuracy, however some of them
prove to be rather accurate when compared to very expensive reference
sensors. Specifically, the [PPD60PV-T2](http://www.shinyei.co.jp/stc/eng/optical/main_ppd60pv.html)
used in the [AirBeam sensor](http://www.takingspace.org/aircasting/airbeam/)
and the [Air Quality Egg V2](https://airqualityegg.wickeddevice.com/)
has been shown 
[by](http://www.aqmd.gov/docs/default-source/aq-spec/field-evaluations/air-quality-egg-v2_pm---field-evaluation.pdf?sfvrsn=2) 
[several](https://www.atmos-meas-tech.net/9/5281/2016/amt-9-5281-2016-discussion.html) 
[authors](http://www.aqmd.gov/docs/default-source/aq-spec/field-evaluations/airbeam---field-evaluation.pdf?sfvrsn=4) 
to have a relatively good linear fit to FEM sensors, however it still seems
to be overly sensitive and in need of calibration. The EPA shows
several of the other sensors by Shinyei to perform 
[similarly](https://cfpub.epa.gov/si/si_public_record_report.cfm?dirEntryId=297517&simpleSearch=1&searchAll=EPA%2F600%2FR-14%2F464).
The PPD42NJ, a similar sensor used in the Air Quality Egg V1 produces 
[embarrassingly bad results](http://www.aqmd.gov/docs/default-source/aq-spec/field-evaluations/air-quality-egg-v1---field-evaluation.pdf?sfvrsn=17),
and Shinyei has at least one new product out that has not been tested.

While the PPD60PV-T2 would probably fit our purposes well, it does not
fit in the Signpost module case. This isn't an absolute problem - we could
build an external enclosure, but it's not ideal. The new Shinyei sensor
would fit into the case, however we don't want to use an untested sensor given
that Shinyei's quality seems to be shaky.

The only other sensor that seems to disclose that they use an OEM module for
PM2.5 is the [PurpleAir](https://www.purpleair.com/). The purple air uses two
PMS5003 sensors, and performs very well for this class of sensor 
in both 
[field](http://www.aqmd.gov/docs/default-source/aq-spec/field-evaluations/purple-air-pa-ii---field-evaluation.pdf?sfvrsn=4) and 
[lab](http://www.aqmd.gov/docs/default-source/aq-spec/laboratory-evaluations/purple-air-pa-ii---lab-evaluation.pdf?sfvrsn=4) testing.
The first version of the PurpleAir used the PMS1003, and also performed well, but not as good as the updated version.

Luckily the PMS5003 is both small, can be ordered for a reasonable price from China, and has an integrated fan. It
also streams data about many particulate classes over UART, which makes it easy to integrated with a module MCU.
This seems like the best option for a Signpost module. The inclusion of two sensors in the PurpleAir does make us
question its reliability.

###Ozone

- Metal Oxide
    - Easy
    - Accurate
    - MIC-2610/MIC-2614
    - No longer on the market
    - May be available from China

- Electrochemical
    - More difficult
    - No tested and open source designs

###Nitrogen Dioxide

- Metal Oxide
    - Easy
    - Accurate
    - MIC-2714

    - MIC-4514
        - Never shown to be accurate

###Carbon Monoxide

- Metal Oxide
    - Easy
    - MIC-4514
    - Never shown to be accurate?

- Electrochemical
    - Spec w/ temp correction
    - Alpha sense
    - City tech
    - All much more expensive and/or difficult
    - Still sense we have done it before it might be worth it for the accuracy.

###Sulfur Dioxide

The only cheaper SO~2~ sensor that we can find is the 
SPEC SO~2~ sensor. Given the poor performance of the other spec sensors and lack
of testing we don't think it is worth trying to measure SO~2~ at this time.

