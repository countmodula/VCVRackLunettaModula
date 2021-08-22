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
<li>Analogue inputs are designated by white jacks</li>
<li>Analogue outputs are designated by purple jacks</li>
</ul>

<h2>Modules</h2>
Most modules in this collection represent CMOS integrated circuits and are named after thier assocated part numbers (CDxxxx, MCxxxx etc). Details on how each one works can be found by searching for the part number in your favourite internet search engine. Any module without a CDxxxx,MCxxxx number is outlined below.

<h3>ADC</h3>
A simple 2 - 8 bit analogue to digital converter. 0 volts = all bits off. Reference voltage sets the voltage that all bits on will represent. Input level can be adjusted to bring the input signal within the conversion range.

<h3>DAC</h3>
A simple 2 - 8 bit digital to analogue converter. Scale sets the voltage that all bits on will represent, offset adds or subtracts up 5 volts the result of the conversion. When less than 8 bits are selected, the surplus bits are ignored. The output light indicates the output levle relative to the selected scale.

<h3>Buttons</h3>
This is a set of six manual logic buttons than can be configured idividually to operate in either latched or momentary fashion via the context menu.

<h3>Ones</h3>
A set of constant logical "Ones".

<h3>Zeroes</h3>
A set of constant logical "Zeroes".

<h3>CD40106</h3>
The CD10406 Hex Schmitt-Trigger Inverter is a logical inverter with a schmitt-trigger input having a positive trigger threshold voltage of approximately 7V and a negative trigger threshold voltage of approximately 4.6V. These are consitent with a Vdd supply of 12V. As such, the outputs of some modules in VCV Rack may not reach a sufficient voltage trigger the inverter.