// DD4WH LPF DC - 3.1 kHz
// (sampling rate 44.117 kHz)
// 12.2.2016
// Parameters generated using Iowa Hills IIR Filter Designer
// 8th order Inv Cheby Lowpass (4-stage cascaded biquad)
// 
// 
// 
// The array of coefficients is in order: B0, B1, B2, A1, A2. Each coefficient must be less than 2.0 and greater than -2.0. The array should be type double. 
// unlike output by Iowa Hils IIR Filter Designer !

const double IIR_31_Coeffs_0[] =
{   
   0.516676615096921532,
   -0.819792954551865849,
   0.516676615096921532, 
   -1.094656125530478310,
   0.308216401172455579,
};

const double IIR_31_Coeffs_1[] =
{  
   0.358508197603467582,
   -0.518847077512281984,
   0.358508197603467582,
   -1.257630547496234370,
   0.455799865190887388
};

const double IIR_31_Coeffs_2[] =
{  
   0.173478188104338238,
   -0.163591138608289915,
   0.173478188104338238,
   -1.480176015318265480,
   0.663541252918652180
};

const double IIR_31_Coeffs_3[] =
{ 
   0.061411582893547395,
   0.060040463689562951,
   0.061411582893547395,
   -1.699310973939165190,
   0.882174603415822989
};

/*
// ELLIPTIC filter coefficients 
// works, but with an unpleasant and annoying hiss sound!
const double IIR_31_Coeffs_0[] =
{   
   0.211735067967220886,
   -0.365798354728391173,
   0.211735067967220886,
   -1.568543543770871020,
   0.626215324976921672
};

const double IIR_31_Coeffs_1[] =
{  
   0.326582053814130580,
   -0.544927310238951357,
   0.326582053814130580,
   -1.640791514445660180,
   0.749028311834969984
};

const double IIR_31_Coeffs_2[] =
{  
   0.274329980165261500,
   -0.390258125345878171,
   0.274329980165261500,
   -1.718502911800378290,
   0.876904746785023170
};

const double IIR_31_Coeffs_3[] =
{ 
   0.085499318375426445,
   0.014729186060807581,
   0.085499318375426445,
   -1.778873148815911740,
   0.964600971627572412
};
*/

