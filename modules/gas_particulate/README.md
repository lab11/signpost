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

## What to Measure?
A survey of outdoor air quality sensors yielded:
 - Particulate (usually PM2.5)
 - Ozone (O<sub>3</sub>)
 - Nitrogen Dioxide (NO<sub>2</sub>)
 - Carbon Monoxide (CO)
 - Sulfur Dioxide (SO<sub>2</sub>)

Of these, EPA reports most focus on particulate matter, 
ozone, and nitrogen dioxide, with carbon monoxide and sulfur dioxide taking
less of a focus. This is not the most concrete scientific evidence for
choosing these gasses, but to effectively implement this module without
too much effort, the sensor should be limited to those which have clear and 
working examples.

### PM2.5 and  Other Particulate

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
The first version of the PurpleAir used the PMS1003, and also performed well, but not as well as the updated version.

Luckily the PMS5003 is both small, can be ordered for a reasonable price, and has an integrated fan. It
also streams data about many particulate classes over UART, which makes it easy to integrated with an MCU.
This seems like the best option for a Signpost module. The inclusion of two sensors in the PurpleAir does make us
question its reliability, and we may consider the option of stacking two sensors into a module box.

### Ozone

[Several reports](https://www.epa.gov/air-sensor-toolbox/evaluation-emerging-air-pollution-sensor-performance) show that the MICS-2614 and MICS-2610 Ozone sensor perform well
under testing. Additionally, these Metal Oxide sensor are cheap, small and
relatively easy to use because they have a resistive response.

Unfortunately the company that made these sensors, e2v scientific, was acquired
by SGX Sensortech, and SGX stopped producing ozone sensors after the acquisition.

You can find knock-off version of the same sensor from Chinese suppliers, however
their performance is unclear, and after asking for quotes from many of these
suppliers, only one has the sensors in stock.

While we could use an electrochemical ozone sensor, there are not very many
on the market, and their ease of integration is questionable (electrochemical
sensors require analog front ends to convert and amplify the signal). Prebuilt
ozone detection modules that use semiconductor based technologies like the ones from [Aeroqual](http://www.gas-sensing.com/fixed-mount-monitors/aeroqual-sm-50.html?sel=2341&gclid=EAIaIQobChMIzcrMvo7F2QIVxF5-Ch20Bg3uEAQYAyABEgK2x_D_BwE) have [shown to be accurate](https://www.atmos-meas-tech.net/9/5281/2016/amt-9-5281-2016-discussion.html),
however they are very expensive.

We don't have a good recommendation for an available and easy to integrate
ozone sensor at this time.


### Nitrogen Dioxide and Carbon Monoxide

Similar to Ozone, there are existing metal oxide sensors to measure both
[Nitrogen Dioxide and Carbon Monoxide](https://sgx.cdistore.com/Manufacturers/e2v/FP/metaloxide-gas-sensor/?type=10420&manf=364&cate=364:5&NavType=2&sd=true#null). These have been shown to work pretty well, although not perfectly in 
[several](https://www.epa.gov/air-sensor-toolbox/evaluation-emerging-air-pollution-sensor-performance) 
[different](http://www.aqmd.gov/docs/default-source/aq-spec/field-evaluations/smart-citizen-kit---field-evaluation.pdf?sfvrsn=2),
scenarios, but the sensors may need to individually calibrated and could drift over time.

This seems like a reasonable choice for a low-cost gas sensor, however it's unclear the
actual impact that these measurements may have due to the potentially
for drifting baselines and low-reliability data.


### Sulfur Dioxide

The only cheaper sulfur dioxide sensor that we can find is the 
SPEC SO<sub>2</sub> sensor. Given the poor performance of the other spec sensors and lack
of testing we don't think it is worth trying to measure sulfur dioxide at this time.


## Conclusions
It seems that building a PM2.5 module out of the PMS5003 sensor could be a good
start to deploying low-cost, medium-reliability particle sensors on the Signpost platform, however
expanding to other sensors begins to go down a path of high difficulty for
potentially limited impact due to unreliable and untrusted data.
