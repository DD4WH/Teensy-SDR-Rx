#include "filters.h"

extern const short fir18[];
extern const short fir26[];
extern const short fir36[];
extern const short fir44[];
extern const short fir65[];
extern const short fir80[];

const short fir18[BPF_COEFFS] = {
#include "fir_18.h" 
};
const short fir26[BPF_COEFFS] = {
#include "fir_26.h" 
};
const short fir36[BPF_COEFFS] = {
#include "fir_36.h" 
};
const short fir44[BPF_COEFFS] = {
#include "fir_44.h" 
};
const short fir65[BPF_COEFFS] = {
#include "fir_65.h" 
};
const short fir80[BPF_COEFFS] = {
#include "fir_80.h" 
};

const short H_45_1_8kHz[HILBERT_COEFFS] = {
#include "H_45_1_8kHz.h" 
};
const short H_m45_1_8kHz[HILBERT_COEFFS] = {
#include "H_m45_1_8kHz.h" 
};
const short H_45_2_6kHz[HILBERT_COEFFS] = {
#include "H_45_2_6kHz.h" 
};
const short H_m45_2_6kHz[HILBERT_COEFFS] = {
#include "H_m45_2_6kHz.h" 
};
const short H_45_3_6kHz[HILBERT_COEFFS] = {
#include "H_45_3_6kHz.h" 
};
const short H_m45_3_6kHz[HILBERT_COEFFS] = {
#include "H_m45_3_6kHz.h" 
};
const short H_45_4_4kHz[HILBERT_COEFFS] = {
#include "H_45_4_4kHz.h" 
};
const short H_m45_4_4kHz[HILBERT_COEFFS] = {
#include "H_m45_4_4kHz.h" 
};
const short H_45_6_5kHz[HILBERT_COEFFS] = {
#include "H_45_6_5kHz.h" 
};
const short H_m45_6_5kHz[HILBERT_COEFFS] = {
#include "H_m45_6_5kHz.h" 
};
const short H_45_8kHz[HILBERT_COEFFS] = {
#include "H_45_8kHz.h" 
};
const short H_m45_8kHz[HILBERT_COEFFS] = {
#include "H_m45_8kHz.h" 
};
const short H_45_11kHz[HILBERT_COEFFS] = {
#include "H_45_11kHz.h" 
};
const short H_m45_11kHz[HILBERT_COEFFS] = {
#include "H_m45_11kHz.h" 
};

const short H_0_8kHz[HILBERT_COEFFS] = {
#include "H_0_8kHz.h" 
};
const short H_90_8kHz[HILBERT_COEFFS] = {
#include "H_90_8kHz.h" 
};




