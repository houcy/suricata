/* Copyright (C) 2007-2010 Open Information Security Foundation
 *
 * You can copy, redistribute or modify this Program under the terms of
 * the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

/**
 * \file
 *
 * \author Victor Julien <victor@inliniac.net>
 *
 * Implements rawbytes keyword support
 *
 * \todo Provide un-normalized telnet dce/rpc buffers to match on
 */

#include "suricata-common.h"

#include "decode.h"
#include "detect.h"
#include "detect-parse.h"
#include "flow-var.h"

#include "detect-content.h"
#include "detect-pcre.h"

#include "util-debug.h"

static int DetectRawbytesSetup (DetectEngineCtx *, Signature *, char *);

void DetectRawbytesRegister (void)
{
    sigmatch_table[DETECT_RAWBYTES].name = "rawbytes";
    sigmatch_table[DETECT_RAWBYTES].Match = NULL;
    sigmatch_table[DETECT_RAWBYTES].Setup = DetectRawbytesSetup;
    sigmatch_table[DETECT_RAWBYTES].Free  = NULL;
    sigmatch_table[DETECT_RAWBYTES].RegisterTests = NULL;

    sigmatch_table[DETECT_RAWBYTES].flags |= SIGMATCH_NOOPT;
    sigmatch_table[DETECT_RAWBYTES].flags |= SIGMATCH_PAYLOAD;
}

static int DetectRawbytesSetup (DetectEngineCtx *de_ctx, Signature *s, char *nullstr)
{
    SCEnter();

    if (nullstr != NULL) {
        SCLogError(SC_ERR_INVALID_VALUE, "rawbytes has no value");
        return -1;
    }

    if (s->list != DETECT_SM_LIST_NOTSET) {
        SCLogError(SC_ERR_RAWBYTES_FILE_DATA, "\"rawbytes\" cannot be combined with \"file_data\"");
        SCReturnInt(-1);
    }

    SigMatch *pm =  SigMatchGetLastSMFromLists(s, 2,
                                               DETECT_CONTENT, s->sm_lists_tail[DETECT_SM_LIST_PMATCH]);
    if (pm == NULL) {
        SCLogError(SC_ERR_RAWBYTES_MISSING_CONTENT, "\"rawbytes\" needs a preceding content option");
        SCReturnInt(-1);
    }

    switch (pm->type) {
        case DETECT_CONTENT:
        {
            DetectContentData *cd = (DetectContentData *)pm->ctx;
            if (cd->flags & DETECT_CONTENT_RAWBYTES) {
                SCLogError(SC_ERR_INVALID_SIGNATURE, "can't use multiple rawbytes modifiers for the same content. ");
                SCReturnInt(-1);
            }
            cd->flags |= DETECT_CONTENT_RAWBYTES;
            break;
        }
        default:
            SCLogError(SC_ERR_RAWBYTES_MISSING_CONTENT, "\"rawbytes\" needs a preceding content option");
            SCReturnInt(-1);
    }

    SCReturnInt(0);
}

