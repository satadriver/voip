/***********************************************************************
Copyright (c) 2006-2010, Skype Limited. All rights reserved. 
Redistribution and use in source and binary forms, with or without 
modification, (subject to the limitations in the disclaimer below) 
are permitted provided that the following conditions are met:
- Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright 
notice, this list of conditions and the following disclaimer in the 
documentation and/or other materials provided with the distribution.
- Neither the name of Skype Limited, nor the names of specific 
contributors, may be used to endorse or promote products derived from 
this software without specific prior written permission.
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED 
BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
CONTRIBUTORS ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF 
USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***********************************************************************/

#include "SKP_Silk_main_FLP.h"
#include "SKP_Silk_perceptual_parameters_FLP.h"

/*
* SKP_Silk_prefilter. Prefilter for finding Quantizer input signal  
*/
SKP_INLINE void SKP_Silk_prefilt_FLP(
    SKP_Silk_prefilter_state_FLP *P,/* (I/O) state */
    SKP_float st_res[],             /* (I) */
    SKP_float xw[],                 /* (O) */
    SKP_float *HarmShapeFIR,        /* (I) */
    SKP_float Tilt,                 /* (I) */
    SKP_float LF_MA_shp,            /* (I) */
    SKP_float LF_AR_shp,            /* (I) */
    SKP_int   lag,                  /* (I) */
    SKP_int   length                /* (I) */
);

/*
* SKP_Silk_prefilter. Main Prefilter Function   
*/
void SKP_Silk_prefilter_FLP(
    SKP_Silk_encoder_state_FLP          *psEnc,         /* I/O  Encoder state FLP                       */
    const SKP_Silk_encoder_control_FLP  *psEncCtrl,     /* I    Encoder control FLP                     */
          SKP_float                     xw[],           /* O    Weighted signal                         */
    const SKP_float                     x[]             /* I    Speech signal                           */
)
{
    SKP_Silk_prefilter_state_FLP *P = &psEnc->sPrefilt;
    SKP_int   j, k, lag;
    SKP_float HarmShapeGain, Tilt, LF_MA_shp, LF_AR_shp;
    SKP_float B[ 2 ];
    const SKP_float *AR1_shp;
    const SKP_float *px;
    SKP_float *pxw, *pst_res;
    SKP_float HarmShapeFIR[ 3 ];
    SKP_float st_res[ MAX_FRAME_LENGTH / NB_SUBFR + MAX_LPC_ORDER ];

    /* Setup pointers */
    px  = x;
    pxw = xw;
    lag = P->lagPrev;
    for( k = 0; k < NB_SUBFR; k++ ) {
        /* Update Variables that change per sub frame */
        if( psEncCtrl->sCmn.sigtype == SIG_TYPE_VOICED ) {
            lag = psEncCtrl->sCmn.pitchL[ k ];
        }

        /* Noise shape parameters */
        HarmShapeGain = psEncCtrl->HarmShapeGain[ k ] * ( 1.0f - psEncCtrl->HarmBoost[ k ] );
        HarmShapeFIR[ 0 ] = SKP_Silk_HarmShapeFIR_FLP[ 0 ] * HarmShapeGain;
        HarmShapeFIR[ 1 ] = SKP_Silk_HarmShapeFIR_FLP[ 1 ] * HarmShapeGain;
        HarmShapeFIR[ 2 ] = SKP_Silk_HarmShapeFIR_FLP[ 2 ] * HarmShapeGain;
        Tilt      =  psEncCtrl->Tilt[ k ];
        LF_MA_shp =  psEncCtrl->LF_MA_shp[ k ];
        LF_AR_shp =  psEncCtrl->LF_AR_shp[ k ];
        AR1_shp   = &psEncCtrl->AR1[ k * SHAPE_LPC_ORDER_MAX ];

        /* Short term FIR filtering*/
        SKP_Silk_LPC_analysis_filter_FLP( st_res, AR1_shp, px - psEnc->sCmn.shapingLPCOrder, 
            psEnc->sCmn.subfr_length + psEnc->sCmn.shapingLPCOrder, psEnc->sCmn.shapingLPCOrder );

        pst_res = st_res + psEnc->sCmn.shapingLPCOrder; // Point to first sample

        /* reduce (mainly) low frequencies during harmonic emphasis */
        B[ 0 ] =  psEncCtrl->GainsPre[ k ];
        B[ 1 ] = -psEncCtrl->GainsPre[ k ] * 
            ( psEncCtrl->HarmBoost[ k ] * HarmShapeGain + INPUT_TILT + psEncCtrl->coding_quality * HIGH_RATE_INPUT_TILT );
        pxw[ 0 ] = B[ 0 ] * pst_res[ 0 ] + B[ 1 ] * P->sHarmHP;
        for( j = 1; j < psEnc->sCmn.subfr_length; j++ ) {
            pxw[ j ] = B[ 0 ] * pst_res[ j ] + B[ 1 ] * pst_res[ j - 1 ];
        }
        P->sHarmHP = pst_res[ psEnc->sCmn.subfr_length - 1 ];

        SKP_Silk_prefilt_FLP( P, pxw, pxw, HarmShapeFIR, Tilt, LF_MA_shp, LF_AR_shp, lag, psEnc->sCmn.subfr_length );
        
        px  += psEnc->sCmn.subfr_length;
        pxw += psEnc->sCmn.subfr_length;
    }
    P->lagPrev = psEncCtrl->sCmn.pitchL[ NB_SUBFR - 1 ];
}
/*
* SKP_Silk_prefilter_part1. Prefilter for finding Quantizer input signal    
*/
SKP_INLINE void SKP_Silk_prefilt_FLP(
    SKP_Silk_prefilter_state_FLP *P,/* (I/O) state */
    SKP_float st_res[],             /* (I) */
    SKP_float xw[],                 /* (O) */
    SKP_float *HarmShapeFIR,        /* (I) */
    SKP_float Tilt,                 /* (I) */
    SKP_float LF_MA_shp,            /* (I) */
    SKP_float LF_AR_shp,            /* (I) */
    SKP_int   lag,                  /* (I) */
    SKP_int   length                /* (I) */
)
{
    SKP_int   i;
    SKP_int   idx, LTP_shp_buf_idx;
    SKP_float n_Tilt, n_LF, n_LTP; 
    SKP_float sLF_AR_shp, sLF_MA_shp;
    SKP_float *LTP_shp_buf;

    /* To speed up use temp variables instead of using the struct */
    LTP_shp_buf     = P->sLTP_shp1;
    LTP_shp_buf_idx = P->sLTP_shp_buf_idx1;
    sLF_AR_shp      = P->sLF_AR_shp1;
    sLF_MA_shp      = P->sLF_MA_shp1;
        
    for( i = 0; i < length; i++ ) {
        if( lag > 0 ) {
            SKP_assert( HARM_SHAPE_FIR_TAPS == 3 );
            idx = lag + LTP_shp_buf_idx;
            n_LTP  = LTP_shp_buf[ ( idx - HARM_SHAPE_FIR_TAPS / 2 - 1) & LTP_MASK ] * HarmShapeFIR[ 0 ];
            n_LTP += LTP_shp_buf[ ( idx - HARM_SHAPE_FIR_TAPS / 2    ) & LTP_MASK ] * HarmShapeFIR[ 1 ];
            n_LTP += LTP_shp_buf[ ( idx - HARM_SHAPE_FIR_TAPS / 2 + 1) & LTP_MASK ] * HarmShapeFIR[ 2 ];
        } else {
            n_LTP = 0;
        }

        n_Tilt = sLF_AR_shp * Tilt;
        n_LF   = sLF_AR_shp * LF_AR_shp + sLF_MA_shp * LF_MA_shp;

        sLF_AR_shp = st_res[ i ] - n_Tilt;
        sLF_MA_shp = sLF_AR_shp - n_LF;

        LTP_shp_buf_idx = ( LTP_shp_buf_idx - 1 ) & LTP_MASK;
        LTP_shp_buf[ LTP_shp_buf_idx ] = sLF_MA_shp;
        xw[ i ] = sLF_MA_shp - n_LTP;
    }
    /* Copy temp variable back to state */
    P->sLF_AR_shp1       = sLF_AR_shp;
    P->sLF_MA_shp1       = sLF_MA_shp;
    P->sLTP_shp_buf_idx1 = LTP_shp_buf_idx;
}
