/*
 * Copyright (c) 2016, Alliance for Open Media. All rights reserved
 *
 * This source code is subject to the terms of the BSD 2 Clause License and
 * the Alliance for Open Media Patent License 1.0. If the BSD 2 Clause License
 * was not distributed with this source code in the LICENSE file, you can
 * obtain it at www.aomedia.org/license/software. If the Alliance for Open
 * Media Patent License 1.0 was not distributed with this source code in the
 * PATENTS file, you can obtain it at www.aomedia.org/license/patent.
 */

#ifndef AV1_COMMON_BLOCKD_H_
#define AV1_COMMON_BLOCKD_H_

#include "./aom_config.h"

#include "aom_dsp/aom_dsp_common.h"
#include "aom_ports/mem.h"
#include "aom_scale/yv12config.h"

#include "av1/common/common_data.h"
#include "av1/common/quant_common.h"
#include "av1/common/entropy.h"
#include "av1/common/entropymode.h"
#include "av1/common/mv.h"
#include "av1/common/scale.h"
#include "av1/common/seg_common.h"
#include "av1/common/tile_common.h"
#if CONFIG_PVQ
#include "av1/common/pvq.h"
#include "av1/common/pvq_state.h"
#include "av1/decoder/decint.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define SUB8X8_COMP_REF 1

#define MAX_MB_PLANE 3

#if CONFIG_EXT_INTER
// Should we try rectangular interintra predictions?
#define USE_RECT_INTERINTRA 1

#if CONFIG_COMPOUND_SEGMENT

// Set COMPOUND_SEGMENT_TYPE to one of the three
// 0: Uniform
// 1: Difference weighted
#define COMPOUND_SEGMENT_TYPE 1

#if COMPOUND_SEGMENT_TYPE == 0
#define MAX_SEG_MASK_BITS 1
// SEG_MASK_TYPES should not surpass 1 << MAX_SEG_MASK_BITS
typedef enum {
  UNIFORM_45 = 0,
  UNIFORM_45_INV,
  SEG_MASK_TYPES,
} SEG_MASK_TYPE;

#elif COMPOUND_SEGMENT_TYPE == 1
#define MAX_SEG_MASK_BITS 1
// SEG_MASK_TYPES should not surpass 1 << MAX_SEG_MASK_BITS
typedef enum {
  DIFFWTD_42 = 0,
  DIFFWTD_42_INV,
  SEG_MASK_TYPES,
} SEG_MASK_TYPE;

#endif  // COMPOUND_SEGMENT_TYPE
#endif  // CONFIG_COMPOUND_SEGMENT
#endif  // CONFIG_EXT_INTER

typedef enum {
  KEY_FRAME = 0,
  INTER_FRAME = 1,
  FRAME_TYPES,
} FRAME_TYPE;

static INLINE int is_inter_mode(PREDICTION_MODE mode) {
#if CONFIG_EXT_INTER
  return mode >= NEARESTMV && mode <= NEW_NEWMV;
#else
  return mode >= NEARESTMV && mode <= NEWMV;
#endif  // CONFIG_EXT_INTER
}

#if CONFIG_PVQ
typedef struct PVQ_INFO {
  int theta[PVQ_MAX_PARTITIONS];
  int max_theta[PVQ_MAX_PARTITIONS];
  int qg[PVQ_MAX_PARTITIONS];
  int k[PVQ_MAX_PARTITIONS];
  od_coeff y[OD_TXSIZE_MAX * OD_TXSIZE_MAX];
  int nb_bands;
  int off[PVQ_MAX_PARTITIONS];
  int size[PVQ_MAX_PARTITIONS];
  int skip_rest;
  int skip_dir;
  int bs;  // log of the block size minus two,
           // i.e. equivalent to aom's TX_SIZE
  // Block skip info, indicating whether DC/AC, is coded.
  PVQ_SKIP_TYPE ac_dc_coded;  // bit0: DC coded, bit1 : AC coded (1 means coded)
  tran_low_t dq_dc_residue;
} PVQ_INFO;

typedef struct PVQ_QUEUE {
  PVQ_INFO *buf;  // buffer for pvq info, stored in encoding order
  int curr_pos;   // curr position to write PVQ_INFO
  int buf_len;    // allocated buffer length
  int last_pos;   // last written position of PVQ_INFO in a tile
} PVQ_QUEUE;
#endif

typedef struct {
  uint8_t *plane[MAX_MB_PLANE];
  int stride[MAX_MB_PLANE];
} BUFFER_SET;

#if CONFIG_EXT_INTER
static INLINE int is_inter_singleref_mode(PREDICTION_MODE mode) {
  return mode >= NEARESTMV && mode <= NEWFROMNEARMV;
}

static INLINE int is_inter_compound_mode(PREDICTION_MODE mode) {
  return mode >= NEAREST_NEARESTMV && mode <= NEW_NEWMV;
}

static INLINE PREDICTION_MODE compound_ref0_mode(PREDICTION_MODE mode) {
  static PREDICTION_MODE lut[MB_MODE_COUNT] = {
    MB_MODE_COUNT,  // DC_PRED
    MB_MODE_COUNT,  // V_PRED
    MB_MODE_COUNT,  // H_PRED
    MB_MODE_COUNT,  // D45_PRED
    MB_MODE_COUNT,  // D135_PRED
    MB_MODE_COUNT,  // D117_PRED
    MB_MODE_COUNT,  // D153_PRED
    MB_MODE_COUNT,  // D207_PRED
    MB_MODE_COUNT,  // D63_PRED
#if CONFIG_ALT_INTRA
    MB_MODE_COUNT,  // SMOOTH_PRED
#endif              // CONFIG_ALT_INTRA
    MB_MODE_COUNT,  // TM_PRED
    MB_MODE_COUNT,  // NEARESTMV
    MB_MODE_COUNT,  // NEARMV
    MB_MODE_COUNT,  // ZEROMV
    MB_MODE_COUNT,  // NEWMV
    MB_MODE_COUNT,  // NEWFROMNEARMV
    NEARESTMV,      // NEAREST_NEARESTMV
    NEARESTMV,      // NEAREST_NEARMV
    NEARMV,         // NEAR_NEARESTMV
    NEARMV,         // NEAR_NEARMV
    NEARESTMV,      // NEAREST_NEWMV
    NEWMV,          // NEW_NEARESTMV
    NEARMV,         // NEAR_NEWMV
    NEWMV,          // NEW_NEARMV
    ZEROMV,         // ZERO_ZEROMV
    NEWMV,          // NEW_NEWMV
  };
  assert(is_inter_compound_mode(mode));
  return lut[mode];
}

static INLINE PREDICTION_MODE compound_ref1_mode(PREDICTION_MODE mode) {
  static PREDICTION_MODE lut[MB_MODE_COUNT] = {
    MB_MODE_COUNT,  // DC_PRED
    MB_MODE_COUNT,  // V_PRED
    MB_MODE_COUNT,  // H_PRED
    MB_MODE_COUNT,  // D45_PRED
    MB_MODE_COUNT,  // D135_PRED
    MB_MODE_COUNT,  // D117_PRED
    MB_MODE_COUNT,  // D153_PRED
    MB_MODE_COUNT,  // D207_PRED
    MB_MODE_COUNT,  // D63_PRED
#if CONFIG_ALT_INTRA
    MB_MODE_COUNT,  // SMOOTH_PRED
#endif              // CONFIG_ALT_INTRA
    MB_MODE_COUNT,  // TM_PRED
    MB_MODE_COUNT,  // NEARESTMV
    MB_MODE_COUNT,  // NEARMV
    MB_MODE_COUNT,  // ZEROMV
    MB_MODE_COUNT,  // NEWMV
    MB_MODE_COUNT,  // NEWFROMNEARMV
    NEARESTMV,      // NEAREST_NEARESTMV
    NEARMV,         // NEAREST_NEARMV
    NEARESTMV,      // NEAR_NEARESTMV
    NEARMV,         // NEAR_NEARMV
    NEWMV,          // NEAREST_NEWMV
    NEARESTMV,      // NEW_NEARESTMV
    NEWMV,          // NEAR_NEWMV
    NEARMV,         // NEW_NEARMV
    ZEROMV,         // ZERO_ZEROMV
    NEWMV,          // NEW_NEWMV
  };
  assert(is_inter_compound_mode(mode));
  return lut[mode];
}

static INLINE int have_newmv_in_inter_mode(PREDICTION_MODE mode) {
  return (mode == NEWMV || mode == NEWFROMNEARMV || mode == NEW_NEWMV ||
          mode == NEAREST_NEWMV || mode == NEW_NEARESTMV ||
          mode == NEAR_NEWMV || mode == NEW_NEARMV);
}

static INLINE int use_masked_motion_search(COMPOUND_TYPE type) {
  return (type == COMPOUND_WEDGE);
}

static INLINE int is_masked_compound_type(COMPOUND_TYPE type) {
#if CONFIG_COMPOUND_SEGMENT
  return (type == COMPOUND_WEDGE || type == COMPOUND_SEG);
#else
  return (type == COMPOUND_WEDGE);
#endif  // CONFIG_COMPOUND_SEGMENT
}
#else

static INLINE int have_newmv_in_inter_mode(PREDICTION_MODE mode) {
  return (mode == NEWMV);
}
#endif  // CONFIG_EXT_INTER

/* For keyframes, intra block modes are predicted by the (already decoded)
   modes for the Y blocks to the left and above us; for interframes, there
   is a single probability table. */

typedef struct {
  PREDICTION_MODE as_mode;
  int_mv as_mv[2];  // first, second inter predictor motion vectors
#if CONFIG_REF_MV
  int_mv pred_mv[2];
#endif
#if CONFIG_EXT_INTER
  int_mv ref_mv[2];
#endif  // CONFIG_EXT_INTER
} b_mode_info;

typedef int8_t MV_REFERENCE_FRAME;

#if CONFIG_PALETTE
typedef struct {
  // Number of base colors for Y (0) and UV (1)
  uint8_t palette_size[2];
// Value of base colors for Y, U, and V
#if CONFIG_AOM_HIGHBITDEPTH
  uint16_t palette_colors[3 * PALETTE_MAX_SIZE];
#else
  uint8_t palette_colors[3 * PALETTE_MAX_SIZE];
#endif  // CONFIG_AOM_HIGHBITDEPTH
  // Only used by encoder to store the color index of the top left pixel.
  // TODO(huisu): move this to encoder
  uint8_t palette_first_color_idx[2];
} PALETTE_MODE_INFO;
#endif  // CONFIG_PALETTE

#if CONFIG_FILTER_INTRA
#define USE_3TAP_INTRA_FILTER 1  // 0: 4-tap; 1: 3-tap
typedef struct {
  // 1: an ext intra mode is used; 0: otherwise.
  uint8_t use_filter_intra_mode[PLANE_TYPES];
  FILTER_INTRA_MODE filter_intra_mode[PLANE_TYPES];
} FILTER_INTRA_MODE_INFO;
#endif  // CONFIG_FILTER_INTRA

#if CONFIG_VAR_TX
#if CONFIG_RD_DEBUG
#define TXB_COEFF_COST_MAP_SIZE (2 * MAX_MIB_SIZE)
#endif
#endif

// TODO(angiebird): Merge RD_COST and RD_STATS
typedef struct RD_STATS {
  int rate;
  int64_t dist;
  int64_t sse;
  int skip;
#if CONFIG_RD_DEBUG
  int txb_coeff_cost[MAX_MB_PLANE];
#if CONFIG_VAR_TX
  int txb_coeff_cost_map[MAX_MB_PLANE][TXB_COEFF_COST_MAP_SIZE]
                        [TXB_COEFF_COST_MAP_SIZE];
#endif  // CONFIG_VAR_TX
#endif  // CONFIG_RD_DEBUG
} RD_STATS;

#if CONFIG_EXT_INTER
typedef struct {
  COMPOUND_TYPE type;
  int wedge_index;
  int wedge_sign;
#if CONFIG_COMPOUND_SEGMENT
  SEG_MASK_TYPE mask_type;
  DECLARE_ALIGNED(16, uint8_t, seg_mask[2 * MAX_SB_SQUARE]);
#endif  // CONFIG_COMPOUND_SEGMENT
} INTERINTER_COMPOUND_DATA;
#endif  // CONFIG_EXT_INTER

// This structure now relates to 8x8 block regions.
typedef struct {
  // Common for both INTER and INTRA blocks
  BLOCK_SIZE sb_type;
  PREDICTION_MODE mode;
  TX_SIZE tx_size;
#if CONFIG_VAR_TX
  // TODO(jingning): This effectively assigned a separate entry for each
  // 8x8 block. Apparently it takes much more space than needed.
  TX_SIZE inter_tx_size[MAX_MIB_SIZE][MAX_MIB_SIZE];
  TX_SIZE min_tx_size;
#endif
  int8_t skip;
  int8_t segment_id;
#if CONFIG_SUPERTX
  // Minimum of all segment IDs under the current supertx block.
  int8_t segment_id_supertx;
#endif                      // CONFIG_SUPERTX
  int8_t seg_id_predicted;  // valid only when temporal_update is enabled

  // Only for INTRA blocks
  PREDICTION_MODE uv_mode;
#if CONFIG_PALETTE
  PALETTE_MODE_INFO palette_mode_info;
#endif  // CONFIG_PALETTE

// Only for INTER blocks
#if CONFIG_DUAL_FILTER
  InterpFilter interp_filter[4];
#else
  InterpFilter interp_filter;
#endif
  MV_REFERENCE_FRAME ref_frame[2];
  TX_TYPE tx_type;

#if CONFIG_FILTER_INTRA
  FILTER_INTRA_MODE_INFO filter_intra_mode_info;
#endif  // CONFIG_FILTER_INTRA
#if CONFIG_EXT_INTRA
  // The actual prediction angle is the base angle + (angle_delta * step).
  int8_t angle_delta[2];
#if CONFIG_INTRA_INTERP
  // To-Do (huisu): this may be replaced by interp_filter
  INTRA_FILTER intra_filter;
#endif  // CONFIG_INTRA_INTERP
#endif  // CONFIG_EXT_INTRA

#if CONFIG_EXT_INTER
  INTERINTRA_MODE interintra_mode;
  // TODO(debargha): Consolidate these flags
  int use_wedge_interintra;
  int interintra_wedge_index;
  int interintra_wedge_sign;
  INTERINTER_COMPOUND_DATA interinter_compound_data;
#endif  // CONFIG_EXT_INTER
  MOTION_MODE motion_mode;
#if CONFIG_MOTION_VAR
  int overlappable_neighbors[2];
#endif  // CONFIG_MOTION_VAR
  int_mv mv[2];
  int_mv pred_mv[2];
#if CONFIG_REF_MV
  uint8_t ref_mv_idx;
#endif
#if CONFIG_EXT_PARTITION_TYPES
  PARTITION_TYPE partition;
#endif
#if CONFIG_NEW_QUANT
  int dq_off_index;
  int send_dq_bit;
#endif  // CONFIG_NEW_QUANT
  /* deringing gain *per-superblock* */
  int8_t dering_gain;
#if CONFIG_DELTA_Q
  int current_q_index;
#endif
#if CONFIG_RD_DEBUG
  RD_STATS rd_stats;
  int mi_row;
  int mi_col;
#endif
#if CONFIG_WARPED_MOTION
  int num_proj_ref[2];
  WarpedMotionParams wm_params[2];
#endif  // CONFIG_WARPED_MOTION

  BOUNDARY_TYPE boundary_info;
} MB_MODE_INFO;

typedef struct MODE_INFO {
  MB_MODE_INFO mbmi;
  b_mode_info bmi[4];
} MODE_INFO;

static INLINE PREDICTION_MODE get_y_mode(const MODE_INFO *mi, int block) {
#if CONFIG_CB4X4
  (void)block;
  return mi->mbmi.mode;
#else
  return mi->mbmi.sb_type < BLOCK_8X8 ? mi->bmi[block].as_mode : mi->mbmi.mode;
#endif
}

static INLINE int is_inter_block(const MB_MODE_INFO *mbmi) {
  return mbmi->ref_frame[0] > INTRA_FRAME;
}

static INLINE int has_second_ref(const MB_MODE_INFO *mbmi) {
  return mbmi->ref_frame[1] > INTRA_FRAME;
}

PREDICTION_MODE av1_left_block_mode(const MODE_INFO *cur_mi,
                                    const MODE_INFO *left_mi, int b);

PREDICTION_MODE av1_above_block_mode(const MODE_INFO *cur_mi,
                                     const MODE_INFO *above_mi, int b);

enum mv_precision { MV_PRECISION_Q3, MV_PRECISION_Q4 };

struct buf_2d {
  uint8_t *buf;
  uint8_t *buf0;
  int width;
  int height;
  int stride;
};

typedef struct macroblockd_plane {
  tran_low_t *dqcoeff;
  PLANE_TYPE plane_type;
  int subsampling_x;
  int subsampling_y;
  struct buf_2d dst;
  struct buf_2d pre[2];
  ENTROPY_CONTEXT *above_context;
  ENTROPY_CONTEXT *left_context;
  int16_t seg_dequant[MAX_SEGMENTS][2];
#if CONFIG_NEW_QUANT
  dequant_val_type_nuq seg_dequant_nuq[MAX_SEGMENTS][QUANT_PROFILES]
                                      [COEF_BANDS];
#endif
#if CONFIG_PALETTE
  uint8_t *color_index_map;
#endif  // CONFIG_PALETTE

  // number of 4x4s in current block
  uint16_t n4_w, n4_h;
  // log2 of n4_w, n4_h
  uint8_t n4_wl, n4_hl;
  // block size in pixels
  uint8_t width, height;

#if CONFIG_AOM_QM
  const qm_val_t *seg_iqmatrix[MAX_SEGMENTS][2][TX_SIZES];
#endif
  // encoder
  const int16_t *dequant;
#if CONFIG_NEW_QUANT
  const dequant_val_type_nuq *dequant_val_nuq[QUANT_PROFILES];
#endif  // CONFIG_NEW_QUANT
#if CONFIG_AOM_QM
  const qm_val_t *seg_qmatrix[MAX_SEGMENTS][2][TX_SIZES];
#endif

#if CONFIG_PVQ || CONFIG_DAALA_DIST
  DECLARE_ALIGNED(16, int16_t, pred[MAX_SB_SQUARE]);
  // PVQ: forward transformed predicted image, a reference for PVQ.
  tran_low_t *pvq_ref_coeff;
#endif
} MACROBLOCKD_PLANE;

#define BLOCK_OFFSET(x, i) \
  ((x) + (i) * (1 << (tx_size_wide_log2[0] + tx_size_high_log2[0])))

typedef struct RefBuffer {
  // TODO(dkovalev): idx is not really required and should be removed, now it
  // is used in av1_onyxd_if.c
  int idx;
  YV12_BUFFER_CONFIG *buf;
  struct scale_factors sf;
} RefBuffer;

typedef struct macroblockd {
  struct macroblockd_plane plane[MAX_MB_PLANE];
  uint8_t bmode_blocks_wl;
  uint8_t bmode_blocks_hl;

  FRAME_COUNTS *counts;
  TileInfo tile;

  int mi_stride;

  MODE_INFO **mi;
  MODE_INFO *left_mi;
  MODE_INFO *above_mi;
  MB_MODE_INFO *left_mbmi;
  MB_MODE_INFO *above_mbmi;

  int up_available;
  int left_available;

  const aom_prob (*partition_probs)[PARTITION_TYPES - 1];

  /* Distance of MB away from frame edges */
  int mb_to_left_edge;
  int mb_to_right_edge;
  int mb_to_top_edge;
  int mb_to_bottom_edge;

  FRAME_CONTEXT *fc;

  /* pointers to reference frames */
  const RefBuffer *block_refs[2];

  /* pointer to current frame */
  const YV12_BUFFER_CONFIG *cur_buf;

  ENTROPY_CONTEXT *above_context[MAX_MB_PLANE];
  ENTROPY_CONTEXT left_context[MAX_MB_PLANE][2 * MAX_MIB_SIZE];

  PARTITION_CONTEXT *above_seg_context;
  PARTITION_CONTEXT left_seg_context[MAX_MIB_SIZE];

#if CONFIG_VAR_TX
  TXFM_CONTEXT *above_txfm_context;
  TXFM_CONTEXT *left_txfm_context;
  TXFM_CONTEXT left_txfm_context_buffer[MAX_MIB_SIZE];

  TX_SIZE max_tx_size;
#if CONFIG_SUPERTX
  TX_SIZE supertx_size;
#endif
#endif

  // block dimension in the unit of mode_info.
  uint8_t n8_w, n8_h;

#if CONFIG_REF_MV
  uint8_t ref_mv_count[MODE_CTX_REF_FRAMES];
  CANDIDATE_MV ref_mv_stack[MODE_CTX_REF_FRAMES][MAX_REF_MV_STACK_SIZE];
  uint8_t is_sec_rect;
#endif

#if CONFIG_PVQ
  daala_dec_ctx daala_dec;
#endif
#if CONFIG_EC_ADAPT
  FRAME_CONTEXT *tile_ctx;
#endif
#if CONFIG_AOM_HIGHBITDEPTH
  /* Bit depth: 8, 10, 12 */
  int bd;
#endif

  int qindex[MAX_SEGMENTS];
  int lossless[MAX_SEGMENTS];
  int corrupted;

  struct aom_internal_error_info *error_info;
#if CONFIG_GLOBAL_MOTION
  WarpedMotionParams *global_motion;
#endif  // CONFIG_GLOBAL_MOTION
#if CONFIG_DELTA_Q
  int prev_qindex;
  int delta_qindex;
  int current_qindex;
#endif
} MACROBLOCKD;

static INLINE BLOCK_SIZE get_subsize(BLOCK_SIZE bsize,
                                     PARTITION_TYPE partition) {
  if (partition == PARTITION_INVALID)
    return BLOCK_INVALID;
  else
    return subsize_lookup[partition][bsize];
}

static const TX_TYPE intra_mode_to_tx_type_context[INTRA_MODES] = {
  DCT_DCT,    // DC
  ADST_DCT,   // V
  DCT_ADST,   // H
  DCT_DCT,    // D45
  ADST_ADST,  // D135
  ADST_DCT,   // D117
  DCT_ADST,   // D153
  DCT_ADST,   // D207
  ADST_DCT,   // D63
#if CONFIG_ALT_INTRA
  ADST_ADST,  // SMOOTH
#endif        // CONFIG_ALT_INTRA
  ADST_ADST,  // TM
};

#if CONFIG_SUPERTX
static INLINE int supertx_enabled(const MB_MODE_INFO *mbmi) {
  TX_SIZE max_tx_size = txsize_sqr_map[mbmi->tx_size];
  return tx_size_wide[max_tx_size] >
         AOMMIN(block_size_wide[mbmi->sb_type], block_size_high[mbmi->sb_type]);
}
#endif  // CONFIG_SUPERTX

#define USE_TXTYPE_SEARCH_FOR_SUB8X8_IN_CB4X4 1

#if CONFIG_EXT_TX
#define ALLOW_INTRA_EXT_TX 1

typedef enum {
  // DCT only
  EXT_TX_SET_DCTONLY = 0,
  // DCT + Identity only
  EXT_TX_SET_DCT_IDTX = 1,
  // Discrete Trig transforms w/o flip (4) + Identity (1)
  EXT_TX_SET_DTT4_IDTX = 2,
  // Discrete Trig transforms w/o flip (4) + Identity (1) + 1D Hor/vert DCT (2)
  EXT_TX_SET_DTT4_IDTX_1DDCT = 3,
  // Discrete Trig transforms w/ flip (9) + Identity (1) + 1D Hor/Ver DCT (2)
  EXT_TX_SET_DTT9_IDTX_1DDCT = 4,
  // Discrete Trig transforms w/ flip (9) + Identity (1) + 1D Hor/Ver (6)
  EXT_TX_SET_ALL16 = 5,
  EXT_TX_SET_TYPES
} TxSetType;

// Number of transform types in each set type
static const int num_ext_tx_set[EXT_TX_SET_TYPES] = { 1, 2, 5, 7, 12, 16 };

// Maps intra set index to the set type
static const int ext_tx_set_type_intra[EXT_TX_SETS_INTRA] = {
  EXT_TX_SET_DCTONLY, EXT_TX_SET_DTT4_IDTX_1DDCT, EXT_TX_SET_DTT4_IDTX
};

// Maps inter set index to the set type
static const int ext_tx_set_type_inter[EXT_TX_SETS_INTER] = {
  EXT_TX_SET_DCTONLY, EXT_TX_SET_ALL16, EXT_TX_SET_DTT9_IDTX_1DDCT,
  EXT_TX_SET_DCT_IDTX
};

// Maps set types above to the indices used for intra
static const int ext_tx_set_index_intra[EXT_TX_SET_TYPES] = { 0, -1, 2,
                                                              1, -1, -1 };

// Maps set types above to the indices used for inter
static const int ext_tx_set_index_inter[EXT_TX_SET_TYPES] = {
  0, 3, -1, -1, 2, 1
};

static INLINE TxSetType get_ext_tx_set_type(TX_SIZE tx_size, BLOCK_SIZE bs,
                                            int is_inter, int use_reduced_set) {
  const TX_SIZE tx_size2 = txsize_sqr_up_map[tx_size];
  tx_size = txsize_sqr_map[tx_size];
#if CONFIG_CB4X4 && USE_TXTYPE_SEARCH_FOR_SUB8X8_IN_CB4X4
  (void)bs;
  if (tx_size > TX_32X32) return EXT_TX_SET_DCTONLY;
#else
  if (tx_size > TX_32X32 || bs < BLOCK_8X8) return EXT_TX_SET_DCTONLY;
#endif
  if (use_reduced_set)
    return is_inter ? EXT_TX_SET_DCT_IDTX : EXT_TX_SET_DTT4_IDTX;
  if (tx_size2 == TX_32X32)
    return is_inter ? EXT_TX_SET_DCT_IDTX : EXT_TX_SET_DCTONLY;
  if (is_inter)
    return (tx_size == TX_16X16 ? EXT_TX_SET_DTT9_IDTX_1DDCT
                                : EXT_TX_SET_ALL16);
  else
    return (tx_size == TX_16X16 ? EXT_TX_SET_DTT4_IDTX
                                : EXT_TX_SET_DTT4_IDTX_1DDCT);
}

static INLINE int get_ext_tx_set(TX_SIZE tx_size, BLOCK_SIZE bs, int is_inter,
                                 int use_reduced_set) {
  const TxSetType set_type =
      get_ext_tx_set_type(tx_size, bs, is_inter, use_reduced_set);
  return is_inter ? ext_tx_set_index_inter[set_type]
                  : ext_tx_set_index_intra[set_type];
}

static const int use_intra_ext_tx_for_txsize[EXT_TX_SETS_INTRA][EXT_TX_SIZES] =
    {
#if CONFIG_CB4X4
      { 1, 1, 1, 1, 1 },  // unused
      { 0, 1, 1, 0, 0 },
      { 0, 0, 0, 1, 0 },
#else
      { 1, 1, 1, 1 },  // unused
      { 1, 1, 0, 0 },
      { 0, 0, 1, 0 },
#endif  // CONFIG_CB4X4
    };

static const int use_inter_ext_tx_for_txsize[EXT_TX_SETS_INTER][EXT_TX_SIZES] =
    {
#if CONFIG_CB4X4
      { 1, 1, 1, 1, 1 },  // unused
      { 0, 1, 1, 0, 0 },
      { 0, 0, 0, 1, 0 },
      { 0, 0, 0, 0, 1 },
#else
      { 1, 1, 1, 1 },  // unused
      { 1, 1, 0, 0 },
      { 0, 0, 1, 0 },
      { 0, 0, 0, 1 },
#endif  // CONFIG_CB4X4
    };

// Transform types used in each intra set
static const int ext_tx_used_intra[EXT_TX_SETS_INTRA][TX_TYPES] = {
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0 },
  { 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 },
};

// Transform types used in each inter set
static const int ext_tx_used_inter[EXT_TX_SETS_INTER][TX_TYPES] = {
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 },
};

// 1D Transforms used in inter set, this needs to be changed if
// ext_tx_used_inter is changed
static const int ext_tx_used_inter_1D[EXT_TX_SETS_INTER][TX_TYPES_1D] = {
  { 1, 0, 0, 0 }, { 1, 1, 1, 1 }, { 1, 1, 1, 1 }, { 1, 0, 0, 1 },
};

static INLINE int get_ext_tx_types(TX_SIZE tx_size, BLOCK_SIZE bs, int is_inter,
                                   int use_reduced_set) {
  const int set_type =
      get_ext_tx_set_type(tx_size, bs, is_inter, use_reduced_set);
  return num_ext_tx_set[set_type];
}

#if CONFIG_RECT_TX
static INLINE int is_rect_tx_allowed_bsize(BLOCK_SIZE bsize) {
  static const char LUT[BLOCK_SIZES] = {
#if CONFIG_CB4X4
    0,  // BLOCK_2X2
    0,  // BLOCK_2X4
    0,  // BLOCK_4X2
#endif
    0,  // BLOCK_4X4
    1,  // BLOCK_4X8
    1,  // BLOCK_8X4
    0,  // BLOCK_8X8
    1,  // BLOCK_8X16
    1,  // BLOCK_16X8
    0,  // BLOCK_16X16
    1,  // BLOCK_16X32
    1,  // BLOCK_32X16
    0,  // BLOCK_32X32
    0,  // BLOCK_32X64
    0,  // BLOCK_64X32
    0,  // BLOCK_64X64
#if CONFIG_EXT_PARTITION
    0,  // BLOCK_64X128
    0,  // BLOCK_128X64
    0,  // BLOCK_128X128
#endif  // CONFIG_EXT_PARTITION
  };

  return LUT[bsize];
}

static INLINE int is_rect_tx_allowed(const MACROBLOCKD *xd,
                                     const MB_MODE_INFO *mbmi) {
  return is_rect_tx_allowed_bsize(mbmi->sb_type) &&
         !xd->lossless[mbmi->segment_id];
}

static INLINE int is_rect_tx(TX_SIZE tx_size) { return tx_size >= TX_SIZES; }
#endif  // CONFIG_RECT_TX
#endif  // CONFIG_EXT_TX

static INLINE TX_SIZE tx_size_from_tx_mode(BLOCK_SIZE bsize, TX_MODE tx_mode,
                                           int is_inter) {
  const TX_SIZE largest_tx_size = tx_mode_to_biggest_tx_size[tx_mode];
#if CONFIG_VAR_TX || (CONFIG_EXT_TX && CONFIG_RECT_TX)
  const TX_SIZE max_rect_tx_size = max_txsize_rect_lookup[bsize];
#else
  const TX_SIZE max_tx_size = max_txsize_lookup[bsize];
#endif  // CONFIG_VAR_TX || (CONFIG_EXT_TX && CONFIG_RECT_TX)
  (void)is_inter;
#if CONFIG_VAR_TX
#if CONFIG_CB4X4
  if (bsize == BLOCK_4X4)
    return AOMMIN(max_txsize_lookup[bsize], largest_tx_size);
#else
  if (bsize < BLOCK_8X8)
    return AOMMIN(max_txsize_lookup[bsize], largest_tx_size);
#endif
  if (txsize_sqr_map[max_rect_tx_size] <= largest_tx_size)
    return max_rect_tx_size;
  else
    return largest_tx_size;
#elif CONFIG_EXT_TX && CONFIG_RECT_TX
  if (txsize_sqr_up_map[max_rect_tx_size] <= largest_tx_size) {
    return max_rect_tx_size;
  } else {
    return largest_tx_size;
  }
#else
  return AOMMIN(max_tx_size, largest_tx_size);
#endif  // CONFIG_VAR_TX
}

#if CONFIG_EXT_INTRA
#define MAX_ANGLE_DELTA_UV 2
#define ANGLE_STEP_UV 4

static const uint8_t av1_angle_step_y[TX_SIZES] = {
  0, 4, 3, 3,
};
static const uint8_t av1_max_angle_delta_y[TX_SIZES] = {
  0, 2, 3, 3,
};

extern const int16_t dr_intra_derivative[90];
static const uint8_t mode_to_angle_map[INTRA_MODES] = {
  0, 90, 180, 45, 135, 111, 157, 203, 67, 0,
};

static INLINE int av1_get_angle_step(BLOCK_SIZE sb_type, int plane) {
  const TX_SIZE max_tx_size = max_txsize_lookup[sb_type];
  return plane ? ANGLE_STEP_UV : av1_angle_step_y[max_tx_size];
}

static INLINE int av1_get_max_angle_delta(BLOCK_SIZE sb_type, int plane) {
  const TX_SIZE max_tx_size = max_txsize_lookup[sb_type];
  return plane ? MAX_ANGLE_DELTA_UV : av1_max_angle_delta_y[max_tx_size];
}

#if CONFIG_INTRA_INTERP
// Returns whether filter selection is needed for a given
// intra prediction angle.
int av1_is_intra_filter_switchable(int angle);
#endif  // CONFIG_INTRA_INTERP
#endif  // CONFIG_EXT_INTRA

#if CONFIG_EXT_TILE
#define FIXED_TX_TYPE 1
#else
#define FIXED_TX_TYPE 0
#endif

static INLINE TX_TYPE get_default_tx_type(PLANE_TYPE plane_type,
                                          const MACROBLOCKD *xd, int block_idx,
                                          TX_SIZE tx_size) {
  const MB_MODE_INFO *const mbmi = &xd->mi[0]->mbmi;

  if (is_inter_block(mbmi) || plane_type != PLANE_TYPE_Y ||
      xd->lossless[mbmi->segment_id] || tx_size >= TX_32X32)
    return DCT_DCT;

  return intra_mode_to_tx_type_context[plane_type == PLANE_TYPE_Y
                                           ? get_y_mode(xd->mi[0], block_idx)
                                           : mbmi->uv_mode];
}

static INLINE TX_TYPE get_tx_type(PLANE_TYPE plane_type, const MACROBLOCKD *xd,
                                  int block_idx, TX_SIZE tx_size) {
  const MODE_INFO *const mi = xd->mi[0];
  const MB_MODE_INFO *const mbmi = &mi->mbmi;

  if (FIXED_TX_TYPE)
    return get_default_tx_type(plane_type, xd, block_idx, tx_size);

#if CONFIG_EXT_TX
  if (xd->lossless[mbmi->segment_id] || txsize_sqr_map[tx_size] > TX_32X32 ||
      (txsize_sqr_map[tx_size] >= TX_32X32 && !is_inter_block(mbmi)))
    return DCT_DCT;
  if (mbmi->sb_type >= BLOCK_8X8 || CONFIG_CB4X4) {
    if (plane_type == PLANE_TYPE_Y) {
#if !ALLOW_INTRA_EXT_TX
      if (is_inter_block(mbmi))
#endif  // ALLOW_INTRA_EXT_TX
        return mbmi->tx_type;
    }

    if (is_inter_block(mbmi)) {
// UV Inter only
#if CONFIG_CB4X4
      if (tx_size < TX_4X4) return DCT_DCT;
#endif
      return (mbmi->tx_type == IDTX && txsize_sqr_map[tx_size] >= TX_32X32)
                 ? DCT_DCT
                 : mbmi->tx_type;
    }
  }

#if CONFIG_CB4X4
  if (tx_size < TX_4X4)
    return DCT_DCT;
  else
    return intra_mode_to_tx_type_context[mbmi->uv_mode];
#endif

  // Sub8x8-Inter/Intra OR UV-Intra
  if (is_inter_block(mbmi))  // Sub8x8-Inter
    return DCT_DCT;
  else  // Sub8x8 Intra OR UV-Intra
    return intra_mode_to_tx_type_context[plane_type == PLANE_TYPE_Y
                                             ? get_y_mode(mi, block_idx)
                                             : mbmi->uv_mode];
#else   // CONFIG_EXT_TX
  (void)block_idx;
  if (plane_type != PLANE_TYPE_Y || xd->lossless[mbmi->segment_id] ||
      txsize_sqr_map[tx_size] >= TX_32X32)
    return DCT_DCT;
  return mbmi->tx_type;
#endif  // CONFIG_EXT_TX
}

void av1_setup_block_planes(MACROBLOCKD *xd, int ss_x, int ss_y);

static INLINE int tx_size_to_depth(const TX_SIZE tx_size) {
  return (int)(tx_size - TX_4X4);
}

static INLINE TX_SIZE depth_to_tx_size(const int depth) {
  return (TX_SIZE)(depth + TX_4X4);
}

static INLINE TX_SIZE get_uv_tx_size(const MB_MODE_INFO *mbmi,
                                     const struct macroblockd_plane *pd) {
  TX_SIZE uv_txsize;
#if CONFIG_CB4X4
  assert(mbmi->tx_size > TX_2X2);
#endif

#if CONFIG_SUPERTX
  if (supertx_enabled(mbmi))
    return uvsupertx_size_lookup[txsize_sqr_map[mbmi->tx_size]]
                                [pd->subsampling_x][pd->subsampling_y];
#endif  // CONFIG_SUPERTX

  uv_txsize = uv_txsize_lookup[mbmi->sb_type][mbmi->tx_size][pd->subsampling_x]
                              [pd->subsampling_y];
#if CONFIG_CB4X4 && !CONFIG_CHROMA_2X2
  uv_txsize = AOMMAX(uv_txsize, TX_4X4);
#endif
  assert(uv_txsize != TX_INVALID);
  return uv_txsize;
}

static INLINE TX_SIZE get_tx_size(int plane, const MACROBLOCKD *xd) {
  const MB_MODE_INFO *mbmi = &xd->mi[0]->mbmi;
  const MACROBLOCKD_PLANE *pd = &xd->plane[plane];
  const TX_SIZE tx_size = plane ? get_uv_tx_size(mbmi, pd) : mbmi->tx_size;
  return tx_size;
}

static INLINE BLOCK_SIZE
get_plane_block_size(BLOCK_SIZE bsize, const struct macroblockd_plane *pd) {
  return ss_size_lookup[bsize][pd->subsampling_x][pd->subsampling_y];
}

static INLINE void reset_skip_context(MACROBLOCKD *xd, BLOCK_SIZE bsize) {
  int i;
  for (i = 0; i < MAX_MB_PLANE; i++) {
    struct macroblockd_plane *const pd = &xd->plane[i];
    const BLOCK_SIZE plane_bsize = get_plane_block_size(bsize, pd);
    const int txs_wide = block_size_wide[plane_bsize] >> tx_size_wide_log2[0];
    const int txs_high = block_size_high[plane_bsize] >> tx_size_high_log2[0];
    memset(pd->above_context, 0, sizeof(ENTROPY_CONTEXT) * txs_wide);
    memset(pd->left_context, 0, sizeof(ENTROPY_CONTEXT) * txs_high);
  }
}

typedef void (*foreach_transformed_block_visitor)(int plane, int block,
                                                  int blk_row, int blk_col,
                                                  BLOCK_SIZE plane_bsize,
                                                  TX_SIZE tx_size, void *arg);

void av1_foreach_transformed_block_in_plane(
    const MACROBLOCKD *const xd, BLOCK_SIZE bsize, int plane,
    foreach_transformed_block_visitor visit, void *arg);

#if CONFIG_DAALA_DIST
void av1_foreach_8x8_transformed_block_in_plane(
    const MACROBLOCKD *const xd, BLOCK_SIZE bsize, int plane,
    foreach_transformed_block_visitor visit,
    foreach_transformed_block_visitor mi_visit, void *arg);
#endif

#if CONFIG_COEF_INTERLEAVE
static INLINE int get_max_4x4_size(int num_4x4, int mb_to_edge,
                                   int subsampling) {
  return num_4x4 + (mb_to_edge >= 0 ? 0 : mb_to_edge >> (5 + subsampling));
}

void av1_foreach_transformed_block_interleave(
    const MACROBLOCKD *const xd, BLOCK_SIZE bsize,
    foreach_transformed_block_visitor visit, void *arg);
#endif

void av1_set_contexts(const MACROBLOCKD *xd, struct macroblockd_plane *pd,
                      int plane, TX_SIZE tx_size, int has_eob, int aoff,
                      int loff);

#if CONFIG_EXT_INTER
static INLINE int is_interintra_allowed_bsize(const BLOCK_SIZE bsize) {
#if !USE_RECT_INTERINTRA
  if (block_size_wide[bsize] != block_size_high[bsize]) return 0;
#endif
  // TODO(debargha): Should this be bsize < BLOCK_LARGEST?
  return (bsize >= BLOCK_8X8) && (bsize < BLOCK_64X64);
}

static INLINE int is_interintra_allowed_mode(const PREDICTION_MODE mode) {
  return (mode >= NEARESTMV) && (mode <= NEWMV);
}

static INLINE int is_interintra_allowed_ref(const MV_REFERENCE_FRAME rf[2]) {
  return (rf[0] > INTRA_FRAME) && (rf[1] <= INTRA_FRAME);
}

static INLINE int is_interintra_allowed(const MB_MODE_INFO *mbmi) {
  return is_interintra_allowed_bsize(mbmi->sb_type) &&
         is_interintra_allowed_mode(mbmi->mode) &&
         is_interintra_allowed_ref(mbmi->ref_frame);
}

static INLINE int is_interintra_allowed_bsize_group(const int group) {
  int i;
  for (i = 0; i < BLOCK_SIZES; i++) {
    if (size_group_lookup[i] == group &&
        is_interintra_allowed_bsize((BLOCK_SIZE)i)) {
      return 1;
    }
  }
  return 0;
}

static INLINE int is_interintra_pred(const MB_MODE_INFO *mbmi) {
  return (mbmi->ref_frame[1] == INTRA_FRAME) && is_interintra_allowed(mbmi);
}
#endif  // CONFIG_EXT_INTER

#if CONFIG_MOTION_VAR || CONFIG_WARPED_MOTION
static INLINE int is_motion_variation_allowed_bsize(BLOCK_SIZE bsize) {
  return (bsize >= BLOCK_8X8);
}

#if CONFIG_MOTION_VAR
static INLINE int check_num_overlappable_neighbors(const MB_MODE_INFO *mbmi) {
  if (mbmi->overlappable_neighbors[0] == 0 &&
      mbmi->overlappable_neighbors[1] == 0)
    return 0;
  if (mbmi->overlappable_neighbors[0] > 2 ||
      mbmi->overlappable_neighbors[1] > 2)
    return 0;
  return 1;
}
#endif

static INLINE MOTION_MODE motion_mode_allowed(const MB_MODE_INFO *mbmi) {
#if CONFIG_EXT_INTER
  if (is_motion_variation_allowed_bsize(mbmi->sb_type) &&
      is_inter_mode(mbmi->mode) && mbmi->ref_frame[1] != INTRA_FRAME) {
#else
  if (is_motion_variation_allowed_bsize(mbmi->sb_type) &&
      is_inter_mode(mbmi->mode)) {
#endif  // CONFIG_EXT_INTER
#if CONFIG_MOTION_VAR
    if (!check_num_overlappable_neighbors(mbmi)) return SIMPLE_TRANSLATION;
#endif
#if CONFIG_WARPED_MOTION
    if (!has_second_ref(mbmi) && mbmi->num_proj_ref[0] >= 3)
      return WARPED_CAUSAL;
    else
#endif  // CONFIG_WARPED_MOTION
#if CONFIG_MOTION_VAR
      return OBMC_CAUSAL;
#else
    return SIMPLE_TRANSLATION;
#endif  // CONFIG_MOTION_VAR
  } else {
    return SIMPLE_TRANSLATION;
  }
}

#if CONFIG_MOTION_VAR
static INLINE int is_neighbor_overlappable(const MB_MODE_INFO *mbmi) {
  return (is_inter_block(mbmi));
}
#endif  // CONFIG_MOTION_VAR
#endif  // CONFIG_MOTION_VAR || CONFIG_WARPED_MOTION

// Returns sub-sampled dimensions of the given block.
// The output values for 'rows_within_bounds' and 'cols_within_bounds' will
// differ from 'height' and 'width' when part of the block is outside the right
// and/or bottom image boundary.
static INLINE void av1_get_block_dimensions(BLOCK_SIZE bsize, int plane,
                                            const MACROBLOCKD *xd, int *width,
                                            int *height,
                                            int *rows_within_bounds,
                                            int *cols_within_bounds) {
  const int block_height = block_size_high[bsize];
  const int block_width = block_size_wide[bsize];
  const int block_rows = (xd->mb_to_bottom_edge >= 0)
                             ? block_height
                             : (xd->mb_to_bottom_edge >> 3) + block_height;
  const int block_cols = (xd->mb_to_right_edge >= 0)
                             ? block_width
                             : (xd->mb_to_right_edge >> 3) + block_width;
  const struct macroblockd_plane *const pd = &xd->plane[plane];
  assert(IMPLIES(plane == PLANE_TYPE_Y, pd->subsampling_x == 0));
  assert(IMPLIES(plane == PLANE_TYPE_Y, pd->subsampling_y == 0));
  assert(block_width >= block_cols);
  assert(block_height >= block_rows);
  if (width) *width = block_width >> pd->subsampling_x;
  if (height) *height = block_height >> pd->subsampling_y;
  if (rows_within_bounds) *rows_within_bounds = block_rows >> pd->subsampling_y;
  if (cols_within_bounds) *cols_within_bounds = block_cols >> pd->subsampling_x;
}

#if CONFIG_GLOBAL_MOTION
static INLINE int is_nontrans_global_motion(const MACROBLOCKD *xd) {
  const MODE_INFO *mi = xd->mi[0];
  const MB_MODE_INFO *const mbmi = &mi->mbmi;
  int ref;
#if CONFIG_CB4X4
  const int unify_bsize = 1;
#else
  const int unify_bsize = 0;
#endif

  // First check if all modes are ZEROMV
  if (mbmi->sb_type >= BLOCK_8X8 || unify_bsize) {
#if CONFIG_EXT_INTER
    if (mbmi->mode != ZEROMV && mbmi->mode != ZERO_ZEROMV) return 0;
#else
    if (mbmi->mode != ZEROMV) return 0;
#endif  // CONFIG_EXT_INTER
  } else {
#if !GLOBAL_SUB8X8_USED
    return 0;
#endif  // !GLOBAL_SUB8X8_USED
#if CONFIG_EXT_INTER
    if (mi->bmi[0].as_mode != ZEROMV || mi->bmi[1].as_mode != ZEROMV ||
        mi->bmi[2].as_mode != ZEROMV || mi->bmi[3].as_mode != ZEROMV ||
        mi->bmi[0].as_mode != ZERO_ZEROMV ||
        mi->bmi[1].as_mode != ZERO_ZEROMV ||
        mi->bmi[2].as_mode != ZERO_ZEROMV || mi->bmi[3].as_mode != ZERO_ZEROMV)
      return 0;
#else
    if (mi->bmi[0].as_mode != ZEROMV || mi->bmi[1].as_mode != ZEROMV ||
        mi->bmi[2].as_mode != ZEROMV || mi->bmi[3].as_mode != ZEROMV)
      return 0;
#endif  // CONFIG_EXT_INTER
  }
  // Now check if all global motion is non translational
  for (ref = 0; ref < 1 + has_second_ref(mbmi); ++ref) {
    if (xd->global_motion[mbmi->ref_frame[ref]].wmtype <= TRANSLATION) return 0;
  }
  return 1;
}
#endif  // CONFIG_GLOBAL_MOTION

static INLINE PLANE_TYPE get_plane_type(const int plane) {
  return (plane == 0) ? PLANE_TYPE_Y : PLANE_TYPE_UV;
}

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // AV1_COMMON_BLOCKD_H_
