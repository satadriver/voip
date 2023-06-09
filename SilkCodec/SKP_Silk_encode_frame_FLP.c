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

#include <stdlib.h>
#include "SKP_Silk_main_FLP.h"



/****************/
/* Encode frame */
/****************/
SKP_int SKP_Silk_encode_frame_FLP( 
    SKP_Silk_encoder_state_FLP      *psEnc,             /* I/O  Encoder state FLP                       */
          SKP_uint8                 *pCode,             /* O    Payload                                 */
          SKP_int16                 *pnBytesOut,        /* I/O  Number of payload bytes;                */
                                                        /*      input: max length; output: used         */
    const SKP_int16                 *pIn                /* I    Input speech frame                      */
)
{
    SKP_Silk_encoder_control_FLP sEncCtrl;
    SKP_int     k, nBytes, ret = 0;
    SKP_float   *x_frame, *res_pitch_frame;
    SKP_int16   pIn_HP[    MAX_FRAME_LENGTH ];
    SKP_int16   pIn_HP_LP[ MAX_FRAME_LENGTH ];
    SKP_float   xfw[       MAX_FRAME_LENGTH ];
    SKP_float   res_pitch[ 2 * MAX_FRAME_LENGTH + LA_PITCH_MAX ];
    SKP_int     LBRR_idx, frame_terminator;

    /* Low bitrate redundancy parameters */
    SKP_uint8   LBRRpayload[ MAX_ARITHM_BYTES ];
    SKP_int16   nBytesLBRR;

    const SKP_uint16 *FrameTermination_CDF;


    sEncCtrl.sCmn.Seed = psEnc->sCmn.frameCounter++ & 3;
    /**************************************************************/
    /* Setup Input Pointers, and insert frame in input buffer    */
    /*************************************************************/
    /* pointers aligned with start of frame to encode */
    x_frame         = psEnc->x_buf + psEnc->sCmn.frame_length; // start of frame to encode
    res_pitch_frame = res_pitch    + psEnc->sCmn.frame_length; // start of pitch LPC residual frame

    /****************************/
    /* Voice Activity Detection */
    /****************************/
    SKP_Silk_VAD_FLP( psEnc, &sEncCtrl, pIn );

    /*******************************************/
    /* High-pass filtering of the input signal */
    /*******************************************/
#if HIGH_PASS_INPUT
    /* Variable high-pass filter */
    SKP_Silk_HP_variable_cutoff_FLP( psEnc, &sEncCtrl, pIn_HP, pIn );
#else
    SKP_memcpy( pIn_HP, pIn, psEnc->sCmn.frame_length * sizeof( SKP_int16 ) );
#endif

#if SWITCH_TRANSITION_FILTERING
    /* Ensure smooth bandwidth transitions */
    SKP_Silk_LP_variable_cutoff( &psEnc->sCmn.sLP, pIn_HP_LP, pIn_HP, psEnc->sCmn.frame_length );
#else
    SKP_memcpy( pIn_HP_LP, pIn_HP, psEnc->sCmn.frame_length * sizeof( SKP_int16 ) );
#endif

    /*******************************************/
    /* Copy new frame to front of input buffer */
    /*******************************************/
    SKP_short2float_array( x_frame + psEnc->sCmn.la_shape, pIn_HP_LP, psEnc->sCmn.frame_length );

    /* Add tiny signal to avoid high CPU load from denormalized floating point numbers */
    for( k = 0; k < 8; k++ ) {
        x_frame[ psEnc->sCmn.la_shape + k * ( psEnc->sCmn.frame_length >> 3 ) ] += ( 1 - ( k & 2 ) ) * 1e-6f;
    }

    /*****************************************/
    /* Find pitch lags, initial LPC analysis */
    /*****************************************/
    SKP_Silk_find_pitch_lags_FLP( psEnc, &sEncCtrl, res_pitch, x_frame );

    /************************/
    /* Noise shape analysis */
    /************************/
    SKP_Silk_noise_shape_analysis_FLP( psEnc, &sEncCtrl, res_pitch_frame, x_frame );

    /*****************************************/
    /* Prefiltering for noise shaper         */
    /*****************************************/
    SKP_Silk_prefilter_FLP( psEnc, &sEncCtrl, xfw, x_frame );

    /***************************************************/
    /* Find linear prediction coefficients (LPC + LTP) */
    /***************************************************/
    SKP_Silk_find_pred_coefs_FLP( psEnc, &sEncCtrl, res_pitch );

    /****************************************/
    /* Process gains                        */
    /****************************************/
    SKP_Silk_process_gains_FLP( psEnc, &sEncCtrl );
    
    /****************************************/
    /* Low Bitrate Redundant Encoding       */
    /****************************************/
    nBytesLBRR = MAX_ARITHM_BYTES;
    SKP_Silk_LBRR_encode_FLP( psEnc, &sEncCtrl, LBRRpayload, &nBytesLBRR, xfw );

    /*****************************************/
    /* Noise shaping quantization            */
    /*****************************************/
    SKP_Silk_NSQ_wrapper_FLP( psEnc, &sEncCtrl, xfw, psEnc->sCmn.q, 0 );

    /**************************************************/
    /* Convert speech activity into VAD and DTX flags */
    /**************************************************/
    if( psEnc->speech_activity < SPEECH_ACTIVITY_DTX_THRES ) {
        psEnc->sCmn.vadFlag = NO_VOICE_ACTIVITY;
        psEnc->sCmn.noSpeechCounter++;
        if( psEnc->sCmn.noSpeechCounter > NO_SPEECH_FRAMES_BEFORE_DTX ) {
            psEnc->sCmn.inDTX = 1;
        }
        if( psEnc->sCmn.noSpeechCounter > MAX_CONSECUTIVE_DTX ) {
            psEnc->sCmn.noSpeechCounter = 0;
            psEnc->sCmn.inDTX           = 0;
        }
    } else {
        psEnc->sCmn.noSpeechCounter = 0;
        psEnc->sCmn.inDTX           = 0;
        psEnc->sCmn.vadFlag         = VOICE_ACTIVITY;
    }

    /****************************************/
    /* Initialize arithmetic coder          */
    /****************************************/
    if( psEnc->sCmn.nFramesInPayloadBuf == 0 ) {
        SKP_Silk_range_enc_init( &psEnc->sCmn.sRC );
        psEnc->sCmn.nBytesInPayloadBuf = 0;
    }

    /****************************************/
    /* Encode Parameters                    */
    /****************************************/
    SKP_Silk_encode_parameters( &psEnc->sCmn, &sEncCtrl.sCmn, &psEnc->sCmn.sRC, psEnc->sCmn.q );
    FrameTermination_CDF = SKP_Silk_FrameTermination_CDF;

    /****************************************/
    /* Update Buffers and State             */
    /****************************************/
    /* Update input buffer */
    SKP_memmove( psEnc->x_buf, &psEnc->x_buf[ psEnc->sCmn.frame_length ], 
        ( psEnc->sCmn.frame_length + psEnc->sCmn.la_shape ) * sizeof( SKP_float ) );
    
    /* Parameters needed for next frame */
    psEnc->sCmn.prev_sigtype = sEncCtrl.sCmn.sigtype;
    psEnc->sCmn.prevLag      = sEncCtrl.sCmn.pitchL[ NB_SUBFR - 1];
    psEnc->sCmn.first_frame_after_reset = 0;

    if( psEnc->sCmn.sRC.error ) {
        /* Encoder returned error: Clear payload buffer */
        psEnc->sCmn.nFramesInPayloadBuf = 0;
    } else {
        psEnc->sCmn.nFramesInPayloadBuf++;
    }

    /****************************************/
    /* Finalize payload and copy to output  */
    /****************************************/
    if( psEnc->sCmn.nFramesInPayloadBuf * FRAME_LENGTH_MS >= psEnc->sCmn.PacketSize_ms ) {

        LBRR_idx = ( psEnc->sCmn.oldest_LBRR_idx + 1 ) & LBRR_IDX_MASK;

        /* Check if FEC information should be added */
        frame_terminator = SKP_SILK_LAST_FRAME;
        if( psEnc->sCmn.LBRR_buffer[ LBRR_idx ].usage == SKP_SILK_ADD_LBRR_TO_PLUS1 ) {
            frame_terminator = SKP_SILK_LBRR_VER1;
        }
        if( psEnc->sCmn.LBRR_buffer[ psEnc->sCmn.oldest_LBRR_idx ].usage == SKP_SILK_ADD_LBRR_TO_PLUS2 ) {
            frame_terminator = SKP_SILK_LBRR_VER2;
            LBRR_idx = psEnc->sCmn.oldest_LBRR_idx;
        }

        /* Add the frame termination info to stream */
        SKP_Silk_range_encoder( &psEnc->sCmn.sRC, frame_terminator, FrameTermination_CDF );

        /* Payload length so far */
        SKP_Silk_range_coder_get_length( &psEnc->sCmn.sRC, &nBytes );

        /* Check that there is enough space in external output buffer, and move data */
        if( *pnBytesOut >= nBytes ) {
            SKP_Silk_range_enc_wrap_up( &psEnc->sCmn.sRC );
            SKP_memcpy( pCode, psEnc->sCmn.sRC.buffer, nBytes * sizeof( SKP_uint8 ) );

            if( frame_terminator > SKP_SILK_MORE_FRAMES && 
                    *pnBytesOut >= nBytes + psEnc->sCmn.LBRR_buffer[ LBRR_idx ].nBytes ) {
                /* Get old packet and add to payload. */
                SKP_memcpy( &pCode[ nBytes ],
                    psEnc->sCmn.LBRR_buffer[ LBRR_idx ].payload,
                    psEnc->sCmn.LBRR_buffer[ LBRR_idx ].nBytes * sizeof( SKP_uint8 ) );
                nBytes += psEnc->sCmn.LBRR_buffer[ LBRR_idx ].nBytes;
            }
            *pnBytesOut = nBytes;
        
            /* Update FEC buffer */
            SKP_memcpy( psEnc->sCmn.LBRR_buffer[ psEnc->sCmn.oldest_LBRR_idx ].payload, LBRRpayload, 
                nBytesLBRR * sizeof( SKP_uint8 ) );
            psEnc->sCmn.LBRR_buffer[ psEnc->sCmn.oldest_LBRR_idx ].nBytes = nBytesLBRR;
            /* The below line describes how FEC should be used */ 
            psEnc->sCmn.LBRR_buffer[ psEnc->sCmn.oldest_LBRR_idx ].usage = sEncCtrl.sCmn.LBRR_usage;
            psEnc->sCmn.oldest_LBRR_idx = ( ( psEnc->sCmn.oldest_LBRR_idx + 1 ) & LBRR_IDX_MASK );

            /* Reset the number of frames in payload buffer */
            psEnc->sCmn.nFramesInPayloadBuf = 0;
        } else {
            /* Not enough space: Payload will be discarded */
            *pnBytesOut = 0;
            nBytes      = 0;
            psEnc->sCmn.nFramesInPayloadBuf = 0;
            ret = SKP_SILK_ENC_PAYLOAD_BUF_TOO_SHORT;
        }
    } else {
        /* No payload for you this time */
        *pnBytesOut = 0;

        /* Encode that more frames follows */
        frame_terminator = SKP_SILK_MORE_FRAMES;
        SKP_Silk_range_encoder( &psEnc->sCmn.sRC, frame_terminator, FrameTermination_CDF );

        /* Payload length so far */
        SKP_Silk_range_coder_get_length( &psEnc->sCmn.sRC, &nBytes );
    }

    /* Check for arithmetic coder errors */
    if( psEnc->sCmn.sRC.error ) {
        ret = SKP_SILK_ENC_INTERNAL_ERROR;
    }

    /* simulate number of ms buffered in channel because of exceeding TargetRate */
    psEnc->BufferedInChannel_ms   += ( 8.0f * 1000.0f * ( nBytes - psEnc->sCmn.nBytesInPayloadBuf ) ) / psEnc->sCmn.TargetRate_bps;
    psEnc->BufferedInChannel_ms   -= FRAME_LENGTH_MS;
    psEnc->BufferedInChannel_ms    = SKP_LIMIT_float( psEnc->BufferedInChannel_ms, 0.0f, 100.0f );
    psEnc->sCmn.nBytesInPayloadBuf = nBytes;

    if( psEnc->speech_activity > WB_DETECT_ACTIVE_SPEECH_LEVEL_THRES ) {
        psEnc->sCmn.sSWBdetect.ActiveSpeech_ms = SKP_ADD_POS_SAT32( psEnc->sCmn.sSWBdetect.ActiveSpeech_ms, FRAME_LENGTH_MS ); 
    }

    return( ret );
}

/* Low Bitrate Redundancy (LBRR) encoding. Reuse all parameters but encode with lower bitrate           */
void SKP_Silk_LBRR_encode_FLP(
    SKP_Silk_encoder_state_FLP      *psEnc,             /* I/O  Encoder state FLP                       */
    SKP_Silk_encoder_control_FLP    *psEncCtrl,         /* I/O  Encoder control FLP                     */
          SKP_uint8                 *pCode,             /* O    Payload                                 */
          SKP_int16                 *pnBytesOut,        /* I/O  Payload bytes; in: max; out: used       */
    const SKP_float                 xfw[]               /* I    Input signal                            */
)
{
    SKP_int32   Gains_Q16[ NB_SUBFR ];
    SKP_int     k, TempGainsIndices[ NB_SUBFR ], frame_terminator;
    SKP_int     nBytes, nFramesInPayloadBuf;
    SKP_float   TempGains[ NB_SUBFR ];
    SKP_int     typeOffset, LTP_scaleIndex, Rate_only_parameters = 0;
    /* Control use of inband LBRR */
    SKP_Silk_LBRR_ctrl_FLP( psEnc, &psEncCtrl->sCmn );

    if( psEnc->sCmn.LBRR_enabled ) {
        /* Save original gains */
        SKP_memcpy( TempGainsIndices, psEncCtrl->sCmn.GainsIndices, NB_SUBFR * sizeof( SKP_int   ) );
        SKP_memcpy( TempGains,        psEncCtrl->Gains,             NB_SUBFR * sizeof( SKP_float ) );

        typeOffset     = psEnc->sCmn.typeOffsetPrev; // Temp save as cannot be overwritten
        LTP_scaleIndex = psEncCtrl->sCmn.LTP_scaleIndex;

        /* Set max rate where quant signal is encoded */
        if( psEnc->sCmn.fs_kHz == 8 ) {
            Rate_only_parameters = 13500;
        } else if( psEnc->sCmn.fs_kHz == 12 ) {
            Rate_only_parameters = 15500;
        } else if( psEnc->sCmn.fs_kHz == 16 ) {
            Rate_only_parameters = 17500;
        } else if( psEnc->sCmn.fs_kHz == 24 ) {
            Rate_only_parameters = 19500;
        } else {
            SKP_assert( 0 );
        }

        if( psEnc->sCmn.Complexity > 0 && psEnc->sCmn.TargetRate_bps > Rate_only_parameters ) {
            if( psEnc->sCmn.nFramesInPayloadBuf == 0 ) {
                /* First frame in packet copy everything */
                SKP_memcpy( &psEnc->sNSQ_LBRR, &psEnc->sNSQ, sizeof( SKP_Silk_nsq_state ) );
                psEnc->sCmn.LBRRprevLastGainIndex = psEnc->sShape.LastGainIndex;
                /* Increase Gains to get target LBRR rate */
                psEncCtrl->sCmn.GainsIndices[ 0 ] += psEnc->sCmn.LBRR_GainIncreases;
                psEncCtrl->sCmn.GainsIndices[ 0 ]  = SKP_LIMIT( psEncCtrl->sCmn.GainsIndices[ 0 ], 0, N_LEVELS_QGAIN - 1 );
            }
            /* Decode to get Gains in sync with decoder */
            SKP_Silk_gains_dequant( Gains_Q16, psEncCtrl->sCmn.GainsIndices, 
                &psEnc->sCmn.LBRRprevLastGainIndex, psEnc->sCmn.nFramesInPayloadBuf );

            /* Overwrite unquantized gains with quantized gains and convert back to Q0 from Q16 */
            for( k = 0; k < NB_SUBFR; k++ ) {
                psEncCtrl->Gains[ k ] = Gains_Q16[ k ] / 65536.0f;
            }

            /*****************************************/
            /* Noise shaping quantization            */
            /*****************************************/
            SKP_Silk_NSQ_wrapper_FLP( psEnc, psEncCtrl, xfw, psEnc->sCmn.q_LBRR, 1 );
        } else {
            SKP_memset( psEnc->sCmn.q_LBRR, 0, psEnc->sCmn.frame_length * sizeof( SKP_int8 ) );
            psEncCtrl->sCmn.LTP_scaleIndex = 0;
        }
        /****************************************/
        /* Initialize arithmetic coder          */
        /****************************************/
        if( psEnc->sCmn.nFramesInPayloadBuf == 0 ) {
            SKP_Silk_range_enc_init( &psEnc->sCmn.sRC_LBRR );
            psEnc->sCmn.nBytesInPayloadBuf = 0;
        }

        /****************************************/
        /* Encode Parameters                    */
        /****************************************/
        SKP_Silk_encode_parameters( &psEnc->sCmn, &psEncCtrl->sCmn, &psEnc->sCmn.sRC_LBRR, psEnc->sCmn.q_LBRR );
        /****************************************/
        /* Encode Parameters                    */
        /****************************************/
        if( psEnc->sCmn.sRC_LBRR.error ) {
            /* Encoder returned error: Clear payload buffer */
            nFramesInPayloadBuf = 0;
        } else {
            nFramesInPayloadBuf = psEnc->sCmn.nFramesInPayloadBuf + 1;
        }

        /****************************************/
        /* Finalize payload and copy to output  */
        /****************************************/
        if( SKP_SMULBB( nFramesInPayloadBuf, FRAME_LENGTH_MS ) >= psEnc->sCmn.PacketSize_ms ) {

            /* Check if FEC information should be added */
            frame_terminator = SKP_SILK_LAST_FRAME;

            /* Add the frame termination info to stream */
            SKP_Silk_range_encoder( &psEnc->sCmn.sRC_LBRR, frame_terminator, SKP_Silk_FrameTermination_CDF );

            /* Payload length so far */
            SKP_Silk_range_coder_get_length( &psEnc->sCmn.sRC_LBRR, &nBytes );

            /* Check that there is enough space in external output buffer and move data */
            if( *pnBytesOut >= nBytes ) {
                SKP_Silk_range_enc_wrap_up( &psEnc->sCmn.sRC_LBRR );
                SKP_memcpy( pCode, psEnc->sCmn.sRC_LBRR.buffer, nBytes * sizeof( SKP_uint8 ) );
                
                *pnBytesOut = nBytes;               
            } else {
                /* Not enough space: Payload will be discarded */
                *pnBytesOut = 0;
                SKP_assert( 0 );
            }
        } else {
            /* No payload for you this time */
            *pnBytesOut = 0;

            /* Encode that more frames follows */
            frame_terminator = SKP_SILK_MORE_FRAMES;
            SKP_Silk_range_encoder( &psEnc->sCmn.sRC_LBRR, frame_terminator, SKP_Silk_FrameTermination_CDF );
        }

        /* Restore original Gains */
        SKP_memcpy( psEncCtrl->sCmn.GainsIndices, TempGainsIndices, NB_SUBFR * sizeof( SKP_int   ) );
        SKP_memcpy( psEncCtrl->Gains,             TempGains,        NB_SUBFR * sizeof( SKP_float ) );
    
        /* Restore LTP scale index and typeoffset */
        psEncCtrl->sCmn.LTP_scaleIndex = LTP_scaleIndex;
        psEnc->sCmn.typeOffsetPrev     = typeOffset;
    }
}
