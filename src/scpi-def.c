/*-
 * BSD 2-Clause License
 *
 * Copyright (c) 2012-2018, Jan Breuer
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file   scpi-def.c
 * @date   Thu Nov 15 10:58:45 UTC 2012
 *
 * @brief  SCPI parser test
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scpi/scpi.h"
#include "scpi-def.h"
#include <ctype.h>
#include "routing.h"
#include "attenuation.h"
#include "system_def.h"

static scpi_result_t route_open_all(scpi_t * context)
{
  routing_disconnect_all();
  return SCPI_RES_OK;
}

static scpi_result_t _check_path_param(scpi_t * context, char *param, size_t len)
{
    char endpoint1, endpoint2;
    if (len != 2) {
      SCPI_ErrorPushEx(&scpi_context, SCPI_ERROR_INVAL_CHARACTER_DATA, "Wrong Length", 0);
      return SCPI_RES_ERR;
    }

    endpoint1 = toupper(param[0]);
    endpoint2 = toupper(param[1]);

    if ((endpoint1 < 'A') || (endpoint1 > 'G')) {
      SCPI_ErrorPushEx(&scpi_context, SCPI_ERROR_INVAL_CHARACTER_DATA, "Wrong endpoint1", 0);
      return SCPI_RES_ERR;
    }
    if ((endpoint2 < 'A') || (endpoint2 > 'G')) {
      SCPI_ErrorPushEx(&scpi_context, SCPI_ERROR_INVAL_CHARACTER_DATA, "Wrong endpoint2", 0);
      return SCPI_RES_ERR;
    }

    if (endpoint1 == endpoint2) {
      SCPI_ErrorPushEx(&scpi_context, SCPI_ERROR_INVAL_CHARACTER_DATA, "Same endpoint", 0);
      return SCPI_RES_ERR;
    }

    if (endpoint1 > endpoint2) {
      /* By convention we swap the two endpoint in this case */
      char tmp = endpoint1;
      endpoint1 = endpoint2;
      endpoint2 = tmp;
    }
    param[0] = endpoint1;
    param[1] = endpoint2;
    return SCPI_RES_OK;
}

static scpi_result_t route_disconnect(scpi_t * context) {
  scpi_result_t res = SCPI_RES_OK;
  const char *param;
  size_t param_len;
  
  /* read first parameter */
  if (!SCPI_ParamCharacters(context, &param, &param_len, TRUE)) {
    res = SCPI_RES_ERR;
  }

  if (res == SCPI_RES_OK) {
    res = _check_path_param(context, (char *) param, param_len);
  }

  if (res == SCPI_RES_OK) {
    /* Do the disconnection */
    if (routing_disconnection(param[0], param[1])) {
      SCPI_ErrorPushEx(&scpi_context, SCPI_ERROR_INVAL_CHARACTER_DATA, "Unsupported connection", 0);
      res = SCPI_RES_ERR;
    }
  }

  return res;
}

static scpi_result_t route_connect(scpi_t * context) {
  scpi_result_t res = SCPI_RES_OK;
  const char *param;
  size_t param_len;
  
  /* read first parameter */
  if (!SCPI_ParamCharacters(context, &param, &param_len, TRUE)) {
    res = SCPI_RES_ERR;
  }

  if (res == SCPI_RES_OK) {
    res = _check_path_param(context, (char *) param, param_len);
  }

  /* Make the connection */
  if (res == SCPI_RES_OK) {
    if (routing_connection(param[0], param[1])) {
      SCPI_ErrorPushEx(&scpi_context, SCPI_ERROR_INVAL_CHARACTER_DATA, "Unsupported connection", 0);
      res = SCPI_RES_ERR;
    }
  }

  return res;
}

static scpi_result_t att_frequency_start (scpi_t * context) {
  SCPI_ResultInt64(context, ATTENUATION_BASE_FREQUENCY);
  return SCPI_RES_OK;
}

static scpi_result_t att_frequency_step (scpi_t * context) {
  SCPI_ResultInt64(context, ATTENUATION_STEP_FREQUENCY);
  return SCPI_RES_OK;
}

static scpi_result_t att_frequency_points (scpi_t * context) {
  SCPI_ResultInt32(context, ATTENUATION_POINTS_FREQUENCY);
  return SCPI_RES_OK;
}

static scpi_result_t att_frequency_query (scpi_t * context) {
  scpi_result_t res = SCPI_RES_OK;
  const char *param;
  size_t param_len;
  int sw1, sw2;
  int64_t frequency;
  int64_t frequency_max = ATTENUATION_BASE_FREQUENCY+((int64_t) (ATTENUATION_POINTS_FREQUENCY - 1))*ATTENUATION_STEP_FREQUENCY;
  const attenuation_t *att;

  /* read first parameter, if present */
  if (SCPI_ParamCharacters(context, &param, &param_len, TRUE)) {
    res = _check_path_param(context, (char *) param, param_len);
  }

  if (res == SCPI_RES_OK) {
    sw1 = param[0] - 'A';
    sw2 = param[1] - 'A';

    att = attenuation_table[sw1][sw2];

    /* read second parameter, if present */
    if (!SCPI_ParamInt64(context, &frequency, FALSE)) {
      frequency = -1;
    } else {
      if ((frequency < ATTENUATION_BASE_FREQUENCY) ||
	  (frequency > frequency_max)) {
	SCPI_ErrorPushEx(&scpi_context, SCPI_ERROR_DATA_OUT_OF_RANGE, NULL, 0);
	res = SCPI_RES_ERR;
      }
    }
  }

  /* Query the attenuation */

  if (res == SCPI_RES_OK) {
    if (frequency == -1) {
      SCPI_ResultArrayUInt16(context,
			     att,
			     ATTENUATION_POINTS_FREQUENCY,
			     SCPI_FORMAT_ASCII);
    } else {
      /* Output value for closest frequencies */
      int32_t step = (frequency - ATTENUATION_BASE_FREQUENCY)/ATTENUATION_STEP_FREQUENCY;
      int32_t step_next = (step < (ATTENUATION_POINTS_FREQUENCY-1)) ? step + 1: step;
      
      int64_t array[4] = {
	ATTENUATION_BASE_FREQUENCY+step*(int64_t)ATTENUATION_STEP_FREQUENCY,
	att[step],
	ATTENUATION_BASE_FREQUENCY+step_next*(int64_t)ATTENUATION_STEP_FREQUENCY,
	att[step_next]
      };

      SCPI_ResultArrayInt64(context,
			     array,
			     4,
			     SCPI_FORMAT_ASCII);
    }
  }

  return res;
}

static scpi_result_t route_connect_query (scpi_t * context) {
    char buffer_out[128];
    scpi_result_t res = SCPI_RES_OK;
    int retval;
    const char *param;
    size_t param_len;
    
    int active_connection_only = 1;

    /* read first parameter if present */
    if (!SCPI_ParamCharacters(context, &param, &param_len, FALSE)) {
      param = NULL;
      param_len = 0;
    } else {
      if (strncmp(param, "ALL", param_len) == 0) {
	param = NULL;
	param_len = 0;
	active_connection_only = 0;
      }	else {
	res = _check_path_param(context, (char *) param, param_len);
      }
    }

    /* Query the connection */
    if (res == SCPI_RES_OK) {
      retval = routing_connection_query((char *) param, buffer_out, active_connection_only);
      res = retval ? SCPI_RES_ERR : res;
      if (res == SCPI_RES_ERR)
	SCPI_ErrorPushEx(&scpi_context, SCPI_ERROR_INVAL_CHARACTER_DATA, "Unsupported connection", 0);
      else
	SCPI_ResultCharacters(context, buffer_out, strlen(buffer_out));
    }

    return res;
}

struct _scpi_channel_value_t {
    int32_t row;
    int32_t col;
};
typedef struct _scpi_channel_value_t scpi_channel_value_t;
/**
 * @brief
 * parses lists
 * channel numbers > 0.
 * no checks yet.
 * valid: (@1), (@3!1:1!3), ...
 * (@1!1:3!2) would be 1!1, 1!2, 2!1, 2!2, 3!1, 3!2.
 * (@3!1:1!3) would be 3!1, 3!2, 3!3, 2!1, 2!2, 2!3, ... 1!3.
 *
 * @param channel_list channel list, compare to SCPI99 Vol 1 Ch. 8.3.2
 */
static scpi_result_t TEST_Chanlst(scpi_t *context) {
    scpi_parameter_t channel_list_param;
#define MAXROW 2    /* maximum number of rows */
#define MAXCOL 6    /* maximum number of columns */
#define MAXDIM 2    /* maximum number of dimensions */
    scpi_channel_value_t array[MAXROW * MAXCOL]; /* array which holds values in order (2D) */
    size_t chanlst_idx; /* index for channel list */
    size_t arr_idx = 0; /* index for array */
    size_t n, m = 1; /* counters for row (n) and columns (m) */

    /* get channel list */
    if (SCPI_Parameter(context, &channel_list_param, TRUE)) {
        scpi_expr_result_t res;
        scpi_bool_t is_range;
        int32_t values_from[MAXDIM];
        int32_t values_to[MAXDIM];
        size_t dimensions;

        bool for_stop_row = FALSE; /* true if iteration for rows has to stop */
        bool for_stop_col = FALSE; /* true if iteration for columns has to stop */
        int32_t dir_row = 1; /* direction of counter for rows, +/-1 */
        int32_t dir_col = 1; /* direction of counter for columns, +/-1 */

        /* the next statement is valid usage and it gets only real number of dimensions for the first item (index 0) */
        if (!SCPI_ExprChannelListEntry(context, &channel_list_param, 0, &is_range, NULL, NULL, 0, &dimensions)) {
            chanlst_idx = 0; /* call first index */
            arr_idx = 0; /* set arr_idx to 0 */
            do { /* if valid, iterate over channel_list_param index while res == valid (do-while cause we have to do it once) */
                res = SCPI_ExprChannelListEntry(context, &channel_list_param, chanlst_idx, &is_range, values_from, values_to, 4, &dimensions);
		(void) res;
                if (is_range == FALSE) { /* still can have multiple dimensions */
                    if (dimensions == 1) {
                        /* here we have our values
                         * row == values_from[0]
                         * col == 0 (fixed number)
                         * call a function or something */
                        array[arr_idx].row = values_from[0];
                        array[arr_idx].col = 0;
                    } else if (dimensions == 2) {
                        /* here we have our values
                         * row == values_fom[0]
                         * col == values_from[1]
                         * call a function or something */
                        array[arr_idx].row = values_from[0];
                        array[arr_idx].col = values_from[1];
                    } else {
                        return SCPI_RES_ERR;
                    }
                    arr_idx++; /* inkrement array where we want to save our values to, not neccessary otherwise */
                    if (arr_idx >= MAXROW * MAXCOL) {
                        return SCPI_RES_ERR;
                    }
                } else if (is_range == TRUE) {
                    if (values_from[0] > values_to[0]) {
                        dir_row = -1; /* we have to decrement from values_from */
                    } else { /* if (values_from[0] < values_to[0]) */
                        dir_row = +1; /* default, we increment from values_from */
                    }

                    /* iterating over rows, do it once -> set for_stop_row = false
                     * needed if there is channel list index isn't at end yet */
                    for_stop_row = FALSE;
                    for (n = values_from[0]; for_stop_row == FALSE; n += dir_row) {
                        /* usual case for ranges, 2 dimensions */
                        if (dimensions == 2) {
                            if (values_from[1] > values_to[1]) {
                                dir_col = -1;
                            } else if (values_from[1] < values_to[1]) {
                                dir_col = +1;
                            }
                            /* iterating over columns, do it at least once -> set for_stop_col = false
                             * needed if there is channel list index isn't at end yet */
                            for_stop_col = FALSE;
                            for (m = values_from[1]; for_stop_col == FALSE; m += dir_col) {
                                /* here we have our values
                                 * row == n
                                 * col == m
                                 * call a function or something */
                                array[arr_idx].row = n;
                                array[arr_idx].col = m;
                                arr_idx++;
                                if (arr_idx >= MAXROW * MAXCOL) {
                                    return SCPI_RES_ERR;
                                }
                                if (m == (size_t)values_to[1]) {
                                    /* endpoint reached, stop column for-loop */
                                    for_stop_col = TRUE;
                                }
                            }
                            /* special case for range, example: (@2!1) */
                        } else if (dimensions == 1) {
                            /* here we have values
                             * row == n
                             * col == 0 (fixed number)
                             * call function or sth. */
                            array[arr_idx].row = n;
                            array[arr_idx].col = 0;
                            arr_idx++;
                            if (arr_idx >= MAXROW * MAXCOL) {
                                return SCPI_RES_ERR;
                            }
                        }
                        if (n == (size_t)values_to[0]) {
                            /* endpoint reached, stop row for-loop */
                            for_stop_row = TRUE;
                        }
                    }
                } else {
                    return SCPI_RES_ERR;
                }
                /* increase index */
                chanlst_idx++;
            } while (SCPI_EXPR_OK == SCPI_ExprChannelListEntry(context, &channel_list_param, chanlst_idx, &is_range, values_from, values_to, 4, &dimensions));
            /* while checks, whether incremented index is valid */
        }
        /* do something at the end if needed */
        /* array[arr_idx].row = 0; */
        /* array[arr_idx].col = 0; */
    }

    {
        size_t i;
        my_fprintf(stderr, "TEST_Chanlst: ");
        for (i = 0; i< arr_idx; i++) {
            my_fprintf(stderr, "%ld!%ld, ", array[i].row, array[i].col);
        }
        my_fprintf(stderr, "\r\n");
    }
    return SCPI_RES_OK;
}

static scpi_result_t My_CoreTstQ(scpi_t * context) {

    SCPI_ResultInt32(context, 0);

    return SCPI_RES_OK;
}

const scpi_command_t scpi_commands[] = {
    /* IEEE Mandated Commands (SCPI std V1999.0 4.1.1) */
    { .pattern = "*CLS", .callback = SCPI_CoreCls,},
    { .pattern = "*ESE", .callback = SCPI_CoreEse,},
    { .pattern = "*ESE?", .callback = SCPI_CoreEseQ,},
    { .pattern = "*ESR?", .callback = SCPI_CoreEsrQ,},
    { .pattern = "*IDN?", .callback = SCPI_CoreIdnQ,},
    { .pattern = "*OPC", .callback = SCPI_CoreOpc,},
    { .pattern = "*OPC?", .callback = SCPI_CoreOpcQ,},
    { .pattern = "*RST", .callback = SCPI_CoreRst,},
    { .pattern = "*SRE", .callback = SCPI_CoreSre,},
    { .pattern = "*SRE?", .callback = SCPI_CoreSreQ,},
    { .pattern = "*STB?", .callback = SCPI_CoreStbQ,},
    { .pattern = "*TST?", .callback = My_CoreTstQ,},
    { .pattern = "*WAI", .callback = SCPI_CoreWai,},

    /* Required SCPI commands (SCPI std V1999.0 4.2.1) */
    {.pattern = "SYSTem:ERRor[:NEXT]?", .callback = SCPI_SystemErrorNextQ,},
    {.pattern = "SYSTem:ERRor:COUNt?", .callback = SCPI_SystemErrorCountQ,},
    {.pattern = "SYSTem:VERSion?", .callback = SCPI_SystemVersionQ,},

    /* {.pattern = "STATus:OPERation?", .callback = scpi_stub_callback,}, */
    /* {.pattern = "STATus:OPERation:EVENt?", .callback = scpi_stub_callback,}, */
    /* {.pattern = "STATus:OPERation:CONDition?", .callback = scpi_stub_callback,}, */
    /* {.pattern = "STATus:OPERation:ENABle", .callback = scpi_stub_callback,}, */
    /* {.pattern = "STATus:OPERation:ENABle?", .callback = scpi_stub_callback,}, */

    {.pattern = "STATus:QUEStionable[:EVENt]?", .callback = SCPI_StatusQuestionableEventQ,},
    /* {.pattern = "STATus:QUEStionable:CONDition?", .callback = scpi_stub_callback,}, */
    {.pattern = "STATus:QUEStionable:ENABle", .callback = SCPI_StatusQuestionableEnable,},
    {.pattern = "STATus:QUEStionable:ENABle?", .callback = SCPI_StatusQuestionableEnableQ,},

    {.pattern = "STATus:PRESet", .callback = SCPI_StatusPreset,},

    /* Signal Switchers */
    {.pattern = "ROUTE:CLOSe",           .callback = TEST_Chanlst,},
    {.pattern = "ROUTE:CLOSe:STATe?",    .callback = SCPI_StubQ,},
    {.pattern = "ROUTE:DISCONnect",      .callback = route_disconnect,},
    {.pattern = "ROUTE:CONNEct",         .callback = route_connect,},
    {.pattern = "ROUTE:CONNEct?",        .callback = route_connect_query,}, /* ROUTE:CONNEct? (active connection) ROUTE:CONNEct? "AB" (AB connected)  ROUTE:CONNEct? "ALL" all possible connections */
    {.pattern = "ROUTE:OPEN",            .callback = route_open_all,},
    {.pattern = "ROUTE:OPEN:ALL",        .callback = route_open_all,},
    {.pattern = "ROUTE:ATTenuation?",    .callback = att_frequency_query,}, /* Report attenuation for a path (@1,2) at given <frequency> or all frequencies (frequency points) */
    {.pattern = "ROUTE:ATTenuation:FREQuency:START?",    .callback = att_frequency_start,}, /* Start frequencys available */
    {.pattern = "ROUTE:ATTenuation:FREQuency:STEP?",    .callback = att_frequency_step,}, /* Step frequency available */
    {.pattern = "ROUTE:ATTenuation:FREQuency:POINts?",    .callback = att_frequency_points,}, /* Number of points available */
#if 0
    {.pattern = "ROUTe:PATH:CATalog?",     .callback = SCPI_StubQ,},
    {.pattern = "SYSTem:COMMunication:TCPIP:CONTROL?", .callback = SCPI_SystemCommTcpipControlQ,},
#endif
    SCPI_CMD_LIST_END
};

scpi_interface_t scpi_interface = {
    .error = SCPI_Error,
    .write = SCPI_Write,
    .control = SCPI_Control,
    .flush = SCPI_Flush,
    .reset = SCPI_Reset,
};

char scpi_input_buffer[SCPI_INPUT_BUFFER_LENGTH];
scpi_error_t scpi_error_queue_data[SCPI_ERROR_QUEUE_SIZE];

scpi_t scpi_context;
