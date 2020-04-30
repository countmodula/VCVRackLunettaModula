<table style="width:1000px; border: 0px solid black;">
<tr style="border: 0px solid black;">
<td style="border: 0px solid black;">
<center>
<img src="./img/CountModulaLunettaLogo.png" alt="Count Modula">
<h1 style="border-bottom: 0px;font-size:50px;">Lunetta Modula by Count Modula</h1>
<h2 style="border-bottom: 0px;">Plugin "Lunetta" modules for VCV Rack v1 by Adam Verspaget (Count Modula)</h2>
</center>
</td>
</tr>
</table>
<hr style="width:1000px; border: 1px solid black;"/>
Whilst these modules are offered free of charge, if you like them or are using them to make money, please consider a small donation to The Count for the effort.
<p>&nbsp;</p>
<a href="https://www.paypal.me/CountModula" target="_donate"><img src="https://www.paypalobjects.com/en_AU/i/btn/btn_donateCC_LG.gif" border="0" alt="Donate with PayPal"/></a>
<hr style="width:1000px; border: 1px solid black;"/>

<h2>User Guide</h2>
Inspired by the works of Stanley Lunetta who's Moosack Machines were experimental electronic instruments made from digital integrated circuits, these modules represent raw building blocks that can be patched together to create your own "Lunetta". 
Please note these are NOT normal synth modules, they are in no way HiFi and are probably fit for experimental use only.

<ul>
<b>Basic operational notes:</b>
<li>Logic level lnputs are designated by red jacks.</li>
<li>Logic level outputs are designated by blue jacks.</li>
</ul>

<b>I/O Mode</b>
There are two I/O modes available which determine the input and output levels of the modules.
The first (and default mode) is based on the VCV Rack voltage standards. In this mode all logic outputs will use 10 volts as the high value and all logic inputs use schmitt triggers with the levels set at 0.1 and 2 volts for the low/high thresholds. 

The second "CMOS Schmitt Trigger" is experimental and intended to emulate the CMOS inputs on the integrated circuits that the modules represent. In this mode logic levels are based on the basic CMOS IC principals of a high output being equal to that of the power supply which in this case will be 12V. Inputs on this mode have schmitt triggers on them set at the standard 1/3 and 2/3 power suply levels or around 4 and 8 volts.

A third mode will be avaiabe at some point which will implement modelling of the standard CMOS input in which there are no schmitt triggers on the inputs, and the threshold for the transition from one state to another is an arbitrary value and input voltages around this point will cause instability.

<h2>Modules</h2>
Most modules in this collection represent CMOS integrated circuits and are named after thier assocated part numbers (CDxxxx) . Details on how each one works can be found by searching for the part number in your favourite internet search engine. Any module without a CDxxxx number is outlined below.

<h3>Buttons</h3>
This is a set of six manual logic buttons than can be configured idividually to operate in either latched or momentary fashion via the context menu.

<h3>Ones</h3>
A set of constant logical "Ones".

<h3>Zeroes</h3>
A set of constant logical "Zeroes".