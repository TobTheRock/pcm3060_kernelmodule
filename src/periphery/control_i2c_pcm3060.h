
#ifndef KERNELMODULE_PCM3060_PERIPHERY_I2C_PCM3060_H
#define KERNELMODULE_PCM3060_PERIPHERY_I2C_PCM3060_H

#define PCM3060_BUFSIZE 8			//NOF Elements in Input/Output buffer
#define PCM3060_SAMPLE_LEN_BIT 16

#define PCM3060_CHANNEL_LEFT 1
#define PCM3060_CHANNEL_LEFT 2

#define PCM3060_ADR_REG64 64
#define PCM3060_ADR_REG65 65
#define PCM3060_ADR_REG66 66
#define PCM3060_ADR_REG67 67
#define PCM3060_ADR_REG68 68
#define PCM3060_ADR_REG69 69
#define PCM3060_ADR_REG70 70
#define PCM3060_ADR_REG71 71
#define PCM3060_ADR_REG72 72
#define PCM3060_ADR_REG73 73

/*REG 64:*/
#define PCM3060_R64_MRST	7	//Mode control register reset (ADC and DAC)
#define PCM3060_R64_SRST	6	//System reset (ADC and DAC)
#define PCM3060_R64_ADPSV	5	//ADC power-save control (ADC)
#define PCM3060_R64_DAPSV	4	//DAC power-save control (DAC)
#define PCM3060_R64_SE		0	//VOUT configuration control (DAC)
//Rest: Reserved

/*REG 65,66
8bits for Digital Attenuation Control of L(R65)/R(R66) DAC
REG VAL		| DEC	| Attenuation[dB]
---------------------------
1111 1111   |	255	| 0(default)
1111 1110   |	254	| -0.5
	.		|	.	|
	.		|	.	|
	.		|	.	|
0011 0111   |	 55	| -100
0011 0110	|	54	| MUTE
	.		|	.	|
	.		|	.	|
	.		|	.	|
0000 0000	|	0	| MUTE
*/

/*REG 67*/
#define PCM3060_R67_CSEL2	7	//Clock select for DAC operation (DAC)
#define PCM3060_R67_MS22	6	//3bit:Master/slave mode for DAC audio interface (DAC)
#define PCM3060_R67_MS21	5
#define PCM3060_R67_MS20	4
#define PCM3060_R67_FMT21	1   //2bit: Interface format for DAC audio interface (DAC)
#define PCM3060_R67_FMT20	0

/*REG 68*/
#define PCM3060_R68_OVER	6	//Oversampling rate control (DAC)
#define PCM3060_R68_DREV2	2	//Output phase select (DAC)
#define PCM3060_R68_MUT22	1	//L: Soft-mute control (DAC)
#define PCM3060_R68_MUT21	0	//R: Soft-mute control (DAC)

/*REG 69*/
#define PCM3060_R69_FLT		7	//Digital filter rolloff control (DAC)
#define PCM3060_R69_DMF1	6	//2bit: De-emphasis sampling rate selection (DAC)
#define PCM3060_R69_DMF0	5
#define PCM3060_R69_DMC		4	//De-emphasis function control (DAC)
#define PCM3060_R69_ZREV	1	//Zero-flag polarity control (DAC)
#define PCM3060_R69_DMF0	0	//Zero-flag form select (DAC)

/*REG 70,71
8bits for Digital Attenuation Control of L(R70)/R(R71) DAC
REG VAL		| DEC	| Attenuation[dB]
---------------------------
1111 1111   |	255	| 20
1111 1110   |	254	| 19.5
	.		|	.	|
	.		|	.	|
	.		|	.	|
1101 0111   |	215	| 0(default)
	.		|	.	|
	.		|	.	|
	.		|	.	|
0000 1111   |	15	| -100
0000 1110	|	14	| MUTE
	.		|	.	|
	.		|	.	|
	.		|	.	|
0000 0000	|	0	| MUTE
*/
/*REG 72*/
#define PCM3060_R72_CSEL1	7	//Clock select for ADC operation(ADC)
#define PCM3060_R72_MS12	6	//3bit : Master/slave mode for ADC audio interface (ADC)
#define PCM3060_R72_MS11	5	//
#define PCM3060_R72_MS10	4	//
#define PCM3060_R72_FMT11	1	//Interface format for ADC audio interface (ADC)
#define PCM3060_R72_FMT10	0	//

/*REG 73*/
#define PCM3060_R73_ZCDD	4	//Zero-cross detection disable for digital attenuation control (ADC)
#define PCM3060_R73_BYP		3	//HPF bypass control (ADC)
#define PCM3060_R73_DREV1	2	//Input phase select (ADC)
#define PCM3060_R73_MUT11	1	//Soft-mute control (ADC)
#define PCM3060_R73_MUT10	0	//

#endif // !KERNELMODULE_PCM3060_PERIPHERY_I2C_PCM3060_H

