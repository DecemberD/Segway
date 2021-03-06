/**********************************************************************c
* Author: Marcin Dec
* Date: 14.07.2015
* FileName:        MotorCurve.c
* Dependencies:
* Processor:       dsPIC30F6013A
* Compiler:        MPLAB� C30 v3.00 or higher
*
************************************************************************/

//const int MCurve[250][3] = {
//16,     4,	1250,
//16,	4,	625,
//16,	4,	416,
//16,	4,	312,
//16,	4,	250,
//16,	4,	208,
//16,	4,	178,
//16,	4,	156,
//16,	4,	138,
//16,	4,	125,
//16,	4,	113,
//16,	4,	104,
//16,	4,	96,
//16,	4,	89,
//16,	4,	83,
//16,	4,	78,
//16,	4,	73,
//16,	4,	69,
//16,	4,	65,
//16,	4,	62,
//16,	4,	59,
//16,	4,	56,
//16,	4,	54,
//16,	4,	52,
//16,	4,	50,
//16,	4,	48,
//16,	4,	46,
//16,	4,	44,
//16,	4,	43,
//16,	4,	41,
//16,	4,	40,
//16,	4,	39,
//16,	4,	37,
//16,	4,	36,
//16,	4,	35,
//16,	4,	34,
//16,	4,	33,
//16,	4,	32,
//16,	4,	32,
//16,	4,	31,
//16,	4,	30,
//16,	4,	29,
//16,	4,	29,
//16,	4,	28,
//16,	4,	27,
//16,	4,	27,
//16,	4,	26,
//16,	4,	26,
//16,	4,	25,
//16,	4,	25,
//4,	16,	98,
//4,	16,	96,
//4,	16,	94,
//4,	16,	92,
//4,	16,	90,
//4,	16,	89,
//4,	16,	87,
//4,	16,	86,
//4,	16,	84,
//4,	16,	83,
//4,	16,	81,
//4,	16,	80,
//4,	16,	79,
//4,	16,	78,
//4,	16,	76,
//4,	16,	75,
//4,	16,	74,
//4,	16,	73,
//4,	16,	72,
//4,	16,	71,
//4,	16,	70,
//4,	16,	69,
//4,	16,	68,
//4,	16,	67,
//4,	16,	66,
//4,	16,	65,
//4,	16,	64,
//4,	16,	64,
//4,	16,	63,
//4,	16,	62,
//4,	16,	61,
//4,	16,	60,
//4,	16,	60,
//4,	16,	59,
//4,	16,	58,
//4,	16,	58,
//4,	16,	57,
//4,	16,	56,
//4,	16,	56,
//4,	16,	55,
//4,	16,	54,
//4,	16,	54,
//4,	16,	53,
//4,	16,	53,
//4,	16,	52,
//4,	16,	52,
//4,	16,	51,
//4,	16,	51,
//4,	16,	50,
//4,	16,	50,
//4,	16,	49,
//4,	16,	49,
//4,	16,	48,
//4,	16,	48,
//4,	16,	47,
//4,	16,	47,
//4,	16,	46,
//4,	16,	46,
//4,	16,	45,
//4,	16,	45,
//4,	16,	45,
//4,	16,	44,
//4,	16,	44,
//4,	16,	43,
//4,	16,	43,
//4,	16,	43,
//4,	16,	42,
//4,	16,	42,
//4,	16,	42,
//4,	16,	41,
//4,	16,	41,
//4,	16,	40,
//4,	16,	40,
//4,	16,	40,
//4,	16,	40,
//4,	16,	39,
//4,	16,	39,
//4,	16,	39,
//4,	16,	38,
//4,	16,	38,
//4,	16,	38,
//4,	16,	37,
//4,	16,	37,
//4,	16,	37,
//4,	16,	37,
//4,	16,	36,
//4,	16,	36,
//4,	16,	36,
//4,	16,	35,
//4,	16,	35,
//4,	16,	35,
//4,	16,	35,
//4,	16,	34,
//4,	16,	34,
//4,	16,	34,
//4,	16,	34,
//4,	16,	34,
//4,	16,	33,
//4,	16,	33,
//4,	16,	33,
//4,	16,	33,
//4,	16,	32,
//4,	16,	32,
//4,	16,	32,
//4,	16,	32,
//4,	16,	32,
//4,	16,	31,
//4,	16,	31,
//4,	16,	31,
//4,	16,	31,
//4,	16,	31,
//4,	16,	30,
//4,	16,	30,
//4,	16,	30,
//4,	16,	30,
//4,	16,	30,
//4,	16,	29,
//4,	16,	29,
//4,	16,	29,
//4,	16,	29,
//4,	16,	29,
//4,	16,	29,
//4,	16,	28,
//4,	16,	28,
//4,	16,	28,
//4,	16,	28,
//4,	16,	28,
//4,	16,	28,
//4,	16,	27,
//4,	16,	27,
//4,	16,	27,
//4,	16,	27,
//4,	16,	27,
//4,	16,	27,
//4,	16,	27,
//4,	16,	26,
//4,	16,	26,
//4,	16,	26,
//4,	16,	26,
//4,	16,	26,
//4,	16,	26,
//4,	16,	26,
//4,	16,	25,
//4,	16,	25,
//4,	16,	25,
//4,	16,	25,
//4,	16,	25,
//4,	16,	25,
//4,	16,	25,
//4,	16,	25,
//4,	16,	24,
//4,	16,	24,
//4,	16,	24,
//4,	16,	24,
//4,	16,	24,
//4,	16,	24,
//4,	16,	24,
//4,	16,	24,
//4,	16,	23,
//4,	16,	23,
//4,	16,	23,
//4,	16,	23,
//4,	16,	23,
//4,	16,	23,
//4,	16,	23,
//4,	16,	23,
//4,	16,	23,
//4,	16,	22,
//4,	16,	22,
//4,	16,	22,
//4,	16,	22,
//4,	16,	22,
//4,	16,	22,
//4,	16,	22,
//4,	16,	22,
//4,	16,	22,
//4,	16,	22,
//4,	16,	21,
//4,	16,	21,
//4,	16,	21,
//4,	16,	21,
//4,	16,	21,
//4,	16,	21,
//4,	16,	21,
//4,	16,	21,
//4,	16,	21,
//4,	16,	21,
//4,	16,	21,
//4,	16,	20,
//4,	16,	20,
//4,	16,	20,
//4,	16,	20,
//4,	16,	20,
//4,	16,	20,
//4,	16,	20,
//4,	16,	20,
//4,	16,	20,
//4,	16,	20,
//4,	16,	20,
//4,	16,	20
//};