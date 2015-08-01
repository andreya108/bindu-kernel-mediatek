/**
 * Multiple Display support in Display Driver
 */
#include <linux/slab.h>
#include <linux/list.h>

#define LOG_TAG "MGR"	// multiple display

#include "disp_svp.h"
#include "disp_mgr.h"
#include "disp_sync_ext.h"
// Client interface definition
#include "mtkfb.h"
#include "disp_drv.h"
#include "ddp_hal.h"
#include "debug.h"
///=============================================================================
// external variables declarations
///==========================

static BOOL log_on = 0;
void disp_mgr_log_on(BOOL on) {
	log_on = on;
}
EXPORT_SYMBOL(disp_mgr_log_on);

///=============================================================================
// structure declarations
///===========================
#define MAX_SESSION_COUNT		5
static UINT session_config[MAX_SESSION_COUNT];
static DEFINE_MUTEX(disp_session_lock);

///=============================================================================
// local variables
///==========================
static LIST_HEAD(todo_queue_head);			// ONE or MORE job for each session
static DEFINE_MUTEX(todo_queue_lock);
static LIST_HEAD(active_queue_head);		// ONLY ONE job for each session
static DEFINE_MUTEX(active_queue_lock);
static LIST_HEAD(done_queue_head);			// ONLY ONE job for each session
static DEFINE_MUTEX(done_queue_lock);
static LIST_HEAD(job_pool_head);
static DEFINE_MUTEX(job_pool_lock);

///=============================================================================
// local function forward declarations
///=============================
static disp_job* disp_create_job (void);
static void disp_init_job(disp_job *job);

///=============================================================================
// global function definitions
///=============================

//------------------------------------------------------------------------------
// Buffer queue management
//-------------------------
DCP_STATUS disp_create_session (struct disp_session_config_t *config) {
	DCP_STATUS ret = DCP_STATUS_OK;
	UINT session = MAKE_DISP_SESSION(config->mode, config->type, config->device_id);
	int i, idx = -1;
	//1.To check if this session exists already
	mutex_lock(&disp_session_lock);
	for (i = 0; i < MAX_SESSION_COUNT; i++) {
		if (session_config[i] == 0 && idx == -1) {
			idx = i;
		}
		if (session_config[i] == session) {
			config->session_id = session;
			ret = DCP_STATUS_ALREADY_EXIST;
			XLOG_WARN("session(0x%x) already exists\n", session);
			break;
		}
	}
	//1.To check if support this session (mode,type,dev)
	//TODO:

	//2. Create this session
	if (ret != DCP_STATUS_ALREADY_EXIST) {
		config->session_id = session;
		session_config[idx] = session;
		XLOG_DBG("New session(0x%x)\n", session);
	}
	mutex_unlock(&disp_session_lock);

	return ret;
}

DCP_STATUS disp_destroy_session (struct disp_session_config_t *config) {
	DCP_STATUS ret = DCP_STATUS_DONT_EXIST;
	UINT session = config->session_id;
	int i, idx;
	//1.To check if this session exists already, and remove it
	mutex_lock(&disp_session_lock);
	for (i = 0; i < MAX_SESSION_COUNT; i++) {
		if (session_config[i] == session) {
			session_config[i] = 0;
			ret = DCP_STATUS_OK;
			break;
		}
	}
	mutex_unlock(&disp_session_lock);
	//2. Destroy this session
	if (ret == DCP_STATUS_OK) {
		XLOG_DBG("Destroy session(0x%x)\n", session);
	} else {
		XLOG_WARN("session(0x%x) does not exists\n", session);
	}

	return ret;
}


DCP_STATUS disp_set_session_input (struct disp_session_input_config_t *input) {
	DCP_STATUS ret = DCP_STATUS_DONT_EXIST;
	int i, id, layerpitch, layerbpp;
	disp_input_config *config;
	disp_job *job = NULL;
	//1. check if this session exits
	mutex_lock(&disp_session_lock);
	for (i = 0; i < MAX_SESSION_COUNT; i++) {
		if (input->session_id == session_config[i]) {
			ret = DCP_STATUS_OK;
			break;
		}
	}
	mutex_unlock(&disp_session_lock);
	if (ret != DCP_STATUS_OK) {
		return ret;
	}
	//2. Reset active job of this session
	job = disp_deque_job(input->session_id);
	mutex_lock(&job->lock);
	for (i = 0; i < MAX_INPUT_CONFIG; i++) {
		config = &input->config[i];
		id = config->layer_id;
		job->input[id].layer_id = id;
		if (id >= MAX_INPUT_CONFIG) {
			//layer_dirty is false, will be ignored
			job->input[id].layer_id = DISP_NO_USE_LAEYR_ID;
			continue;
		}
		if (config->layer_enable) {
			switch (config->src_fmt)
			{
			case MTK_FB_FORMAT_YUV422:
				job->input[id].format = eYUY2;
				layerpitch = 2;
				layerbpp = 24;
				break;

			case MTK_FB_FORMAT_RGB565:
				job->input[id].format = eRGB565;
				layerpitch = 2;
				layerbpp = 16;
				break;

			case MTK_FB_FORMAT_RGB888:
				job->input[id].format = eRGB888;
				layerpitch = 3;
				layerbpp = 24;
				break;
			case MTK_FB_FORMAT_BGR888:
				job->input[id].format = eBGR888;
				layerpitch = 3;
				layerbpp = 24;
				break;

			case MTK_FB_FORMAT_ARGB8888:
				job->input[id].format = ePARGB8888;
				layerpitch = 4;
				layerbpp = 32;
				break;
			case MTK_FB_FORMAT_ABGR8888:
				job->input[id].format = ePABGR8888;
				layerpitch = 4;
				layerbpp = 32;
				break;
			case MTK_FB_FORMAT_XRGB8888:
				job->input[id].format = eARGB8888;
				layerpitch = 4;
				layerbpp = 32;
				break;
			case MTK_FB_FORMAT_XBGR8888:
				job->input[id].format = eABGR8888;
				layerpitch = 4;
				layerbpp = 32;
				break;
			case MTK_FB_FORMAT_UYVY:
				job->input[id].format = eUYVY;
				layerpitch = 2;
				layerbpp = 16;
				break;
			default:
				XLOG_ERR("Invalid color format: 0x%x\n", config->src_fmt);
				//layer_dirty is false, will be ignored
				continue;
			}
		    job->input[id].security = config->security;
		    if (config->src_phy_addr != NULL) {
		    	job->input[id].address = (unsigned int)config->src_phy_addr;
		    } else {
		    	job->input[id].address = disp_sync_query_buffer_mva(config->layer_id, (unsigned int)config->next_buff_idx);
		    }
		    job->input[id].index = config->next_buff_idx;

		    /**
		     * NOT USED now
		        job->input[id].tdshp = layerInfo->isTdshp;
		    	job->input[id].identity = layerInfo->identity;
		    	job->input[id].connected_type = layerInfo->connected_type;
		    	job->input[id].sharp = layerInfo->isTdshp;
		     */
		    //set Alpha blending
		    if (config->alpha_enable) {
		    	job->input[id].alpha_enable = TRUE;
		    	job->input[id].alpha = config->alpha;
		    } else {
		    	job->input[id].alpha_enable = FALSE;
		    }
		    if (MTK_FB_FORMAT_ARGB8888 == config->src_fmt ||
		    		MTK_FB_FORMAT_ABGR8888 == config->src_fmt)
		    {
		    	job->input[id].alpha_enable = TRUE;
		    	job->input[id].alpha = 0xff;
		    }
		    //set src width, src height
			job->input[id].src_x = config->src_offset_x;
			job->input[id].src_y = config->src_offset_y;
			job->input[id].dst_x = config->tgt_offset_x;
			job->input[id].dst_y = config->tgt_offset_y;
			if (config->src_width != config->tgt_width || config->src_height != config->tgt_height) {
				XLOG_ERR("OVL cannot support clip:src(%d,%d), dst(%d,%d)\n", config->src_width, config->src_height, config->tgt_width, config->tgt_height);
			}
			job->input[id].width = config->tgt_width;
			job->input[id].height = config->tgt_height;

			job->input[id].pitch = config->src_pitch*layerpitch;

		    //set color key
		    job->input[id].color_key = config->src_color_key;
		    job->input[id].color_key_enable = config->src_use_color_key;

		    //data transferring is triggerred in MTKFB_TRIG_OVERLAY_OUT
		    job->input[id].layer_enable = config->layer_enable;
		    job->input[id].dirty = 1;

		} else {
			job->input[id].index = DISP_INVALID_FENCE_INDEX;
			job->input[id].dirty = 1;
		}
		XLOG_DBG("L%d input:%d,%d\n", id, job->input[id].layer_enable, job->input[id].index);
	}
	mutex_unlock(&job->lock);

	return ret;
}

DCP_STATUS disp_set_session_output (struct disp_session_output_config_t *output) {
	DCP_STATUS ret = DCP_STATUS_DONT_EXIST;
	disp_job *job = NULL;
	disp_output_config* config;
	int i, bpp, yuv;
	//1. check if this session exits
	mutex_lock(&disp_session_lock);
	for (i = 0; i < MAX_SESSION_COUNT; i++) {
		if (output->session_id == session_config[i]) {
			ret = DCP_STATUS_OK;
			break;
		}
	}
	mutex_unlock(&disp_session_lock);
	if (ret != DCP_STATUS_OK) {
		XLOG_ERR("NO such session 0x%x\n", output->session_id);
		return ret;
	}
	//2. Reset active job of this session
	job = disp_deque_job(output->session_id);
	mutex_lock(&job->lock);

	config = &output->config;
	switch (config->fmt)
	{
	case MTK_FB_FORMAT_YUV422:
		job->output.format = eYUY2;
		bpp = 2;
		yuv = 1;
		break;
	case MTK_FB_FORMAT_RGB565:
		job->output.format = eRGB565;
		bpp = 2;
		yuv = 0;
		break;
	case MTK_FB_FORMAT_RGB888:
		job->output.format = eRGB888;
		bpp = 3;
		yuv = 0;
		break;
	case MTK_FB_FORMAT_BGR888:
		job->output.format = eBGR888;
		bpp = 3;
		yuv = 0;
		break;
	case MTK_FB_FORMAT_ARGB8888:
		job->output.format = eABGR8888;
		bpp = 4;
		yuv = 0;
		break;
	case MTK_FB_FORMAT_ABGR8888:
		job->output.format = eABGR8888;
		bpp = 4;
		yuv = 0;
		break;
	case MTK_FB_FORMAT_XRGB8888:
		job->output.format = eXARGB8888;
		bpp = 4;
		yuv = 0;
		break;
	case MTK_FB_FORMAT_XBGR8888:
		job->output.format = eABGR8888;
		bpp = 4;
		yuv = 0;
		break;
	case MTK_FB_FORMAT_UYVY:
		job->output.format = eUYVY;
		bpp = 2;
		yuv = 1;
		break;
	default:
		XLOG_ERR("Invalid color format: 0x%x\n", config->fmt);
	}
	if (config->pa != NULL) {
		job->output.address = config->pa;
	} else {
		job->output.address = disp_sync_query_buffer_mva(MAX_INPUT_CONFIG, config->buff_idx);
	}
	job->output.layer_id = MAX_INPUT_CONFIG;
	job->output.x = config->x;
	job->output.y = config->y;
	job->output.width = config->width;
	job->output.height = config->height;
	job->output.pitch = config->pitch * bpp;
	job->output.pitchUV = config->pitchUV * bpp * yuv;
	job->output.security = config->security;
	job->output.dirty = 1;
	job->output.index = config->buff_idx;

	mutex_unlock(&job->lock);
	XLOG_DBG("L%d output:%d,0x%x\n", config->buff_idx, config->pa);
	return ret;
}

//------------------------------------------------------------------------------
// job managements
// Job's lifetime:
// 1. [FREE] Add a active job for the new session
// 2. [ACTIVE] Push configuration into the active job for given session
// 3. [QUEUED] Queue this active job into JOB Queue
// 4. [ACQUIRED] Handling jobs in job queue one by one until empty
// 5. [FREE] Recycle finished job
// 6. Step for 1~5
//
// Job's status:
// 1. FREE, be created if needed, as a freed job
// 2. ACTIVE, set as the current active job for a certain Session
//    all information of this Session will be fill into this active job
// 3. QUEUED, Add to Job Queue, waiting for processed by hw
// 4. ACQUIRED, Removed from Job Queue, be processing now
//----------------------------------------------------------

disp_job* disp_deque_job(UINT gid) {
	disp_job *pos, *job = NULL;
	mutex_lock(&active_queue_lock);
	//1. Reture the current Active Job if it exists already
	if (!list_empty(&active_queue_head)) {
		list_for_each_entry(pos, &active_queue_head, list) {
			if (pos->group_id == gid) {
				job = pos;
				break;
			}
		}
	}
	//2. Create a new Active Job for this @gid
	if (job == NULL) {
		job = disp_create_job();
		job->status = ACTIVE;
		job->group_id = gid;
		list_add_tail(&job->list, &active_queue_head);
	}
	mutex_unlock(&active_queue_lock);
	XLOG_DBG("S%x deque: 0x%p\n", gid, job);
	MMProfileLogEx(MTKFB_MMP_Events.deque, MMProfileFlagPulse, gid, (job==NULL)?0:job);

	return job;
}

//FIXME: if deque job thread is not same as enque job thread, then we need to
// sync to make only one active job exists!!
DCP_STATUS disp_enque_job (UINT gid) {
	DCP_STATUS ret = DCP_STATUS_DONT_EXIST;
	disp_job *pos, *n;
	mutex_lock(&active_queue_lock);
	if (!list_empty(&active_queue_head)) {
		list_for_each_entry_safe(pos, n, &active_queue_head, list) {
			if (gid == pos->group_id) {
				//1. remove from active job list
				mutex_lock(&pos->lock);
				list_del_init(&pos->list);
				pos->status = QUEUED;
				mutex_unlock(&pos->lock);

				//2. add to job queue list
				mutex_lock(&todo_queue_lock);
				list_add_tail(&pos->list, &todo_queue_head);
				mutex_unlock(&todo_queue_lock);
				ret = DCP_STATUS_OK;

				XLOG_DBG("S%x enque: 0x%p\n", gid, pos);
				MMProfileLogEx(MTKFB_MMP_Events.enque, MMProfileFlagPulse, gid, pos);
				break;
			}
		}
	}
	mutex_unlock(&active_queue_lock);
	return ret;
}

disp_job* disp_acquire_job (void) {
	disp_job *pos, *job = NULL;
	mutex_lock(&todo_queue_lock);
	if (!list_empty(&todo_queue_head)) {
		list_for_each_entry(pos, &todo_queue_head, list) {
			if (pos->status == QUEUED) {
				job = pos;
				mutex_lock(&job->lock);
				job->status = ACQUIRED;
				mutex_unlock(&job->lock);
				MMProfileLogEx(MTKFB_MMP_Events.acquire, MMProfileFlagPulse, job->group_id, job);
				break;
			}
		}
	}
	mutex_unlock(&todo_queue_lock);
	return job;
}

UINT disp_release_job (void) {
	UINT cnt = 0;
	disp_job *pos, *n;
	mutex_lock(&todo_queue_lock);
	if (!list_empty(&todo_queue_head)) {
		list_for_each_entry_safe(pos, n, &todo_queue_head, list) {
			if (pos->status == ACQUIRED) {
				cnt++;
				//1. remove from todo job list
				mutex_lock(&pos->lock);
				list_del_init(&pos->list);
				pos->status = DONE;
				mutex_unlock(&pos->lock);
				//2. add to done job list
				mutex_lock(&done_queue_lock);
				list_add_tail(&pos->list, &done_queue_head);
				mutex_unlock(&done_queue_lock);
				XLOG_DBG("release:0x%p,%d\n", pos, cnt);
				MMProfileLogEx(MTKFB_MMP_Events.release, MMProfileFlagPulse, cnt, pos);
			}
		}
	}
	mutex_unlock(&todo_queue_lock);

	return cnt;
}

disp_job* disp_query_job (void) {
	disp_job *job = NULL;
	disp_job *pos, *n;
	mutex_lock(&done_queue_lock);
	if (!list_empty(&done_queue_head)) {
		list_for_each_entry(pos, &done_queue_head, list) {
			if (pos->status == DONE) {
				//1. remove from processing job list
				mutex_lock(&pos->lock);
				pos->status = FREE;
				mutex_unlock(&pos->lock);
				job = pos;
				XLOG_DBG("query:0x%p\n", pos);
				MMProfileLogEx(MTKFB_MMP_Events.query, MMProfileFlagPulse, job->group_id, job);
			}
		}
	}
	mutex_unlock(&done_queue_lock);
	return job;
}

UINT disp_recycle_job (void) {
	int cnt = 0;
	disp_job *pos, *n;
	mutex_lock(&done_queue_lock);
	if (!list_empty(&done_queue_head)) {
		list_for_each_entry_safe(pos, n, &done_queue_head, list) {
			if (pos->status == FREE) {
				cnt++;
				//1. remove from processing job list
				list_del_init(&pos->list);
				//2. re-initialize this job before use it
				disp_init_job(pos);
				//3. add to job pool list
				mutex_lock(&job_pool_lock);
				list_add_tail(&pos->list, &job_pool_head);
				mutex_unlock(&job_pool_lock);
				XLOG_DBG("recycle:0x%p,%d\n", pos, cnt);
				MMProfileLogEx(MTKFB_MMP_Events.recycle, MMProfileFlagPulse, cnt, pos);
			}
		}
	}
	mutex_unlock(&done_queue_lock);
	return cnt;
}

DCP_STATUS disp_cancel_job(UINT gid) {
	DCP_STATUS ret = DCP_STATUS_DONT_EXIST;
	disp_job *pos, *n;
	mutex_lock(&active_queue_lock);
	if (!list_empty(&active_queue_head)) {
		list_for_each_entry_safe(pos, n, &active_queue_head, list) {
			if (pos->group_id == gid) {
				//1. remove from active job list
				list_del_init(&pos->list);
				//2. re-initialize this job before use it
				disp_init_job(pos);
				//3. add to job pool list
				mutex_lock(&job_pool_lock);
				list_add_tail(&pos->list, &job_pool_head);
				mutex_unlock(&job_pool_lock);
				XLOG_DBG("cancel:%d,0x%p\n", gid, pos);
				ret = DCP_STATUS_OK;
			}
		}
	}
	mutex_unlock(&active_queue_lock);
	return ret;
}

//------------------------------------------------------------------------------
// Buffer queue management
//----------------------------
DCP_STATUS disp_buffer_queue_init (disp_buffer_queue *que, UINT address[], UINT cnt) {
	DCP_STATUS ret = DCP_STATUS_NOT_IMPLEMENTED;
	UINT index, offset;
	if (cnt <= MAX_QUEUE_BUFFER_COUNT) {
		spin_lock_init(&que->lock);
		que->buffer_count = cnt;
		que->read_slot = 0;
		que->write_slot = 1;
		for (index = 0; index < cnt; index++) {
			que->buffer_queue[index] = address[index];
		}
		ret = DCP_STATUS_OK;
	}

	return ret;
}

DCP_STATUS disp_buffer_queue_init_continous (disp_buffer_queue *que, UINT address, UINT size, UINT cnt) {
	DCP_STATUS ret = DCP_STATUS_NOT_IMPLEMENTED;
	UINT index, offset;
	if (cnt <= MAX_QUEUE_BUFFER_COUNT) {
		spin_lock_init(&que->lock);
		que->buffer_count = cnt;
		que->read_slot = 0;
		que->write_slot = 1;
		offset = size / cnt;
		for (index = 0; index < cnt; index++) {
			que->buffer_queue[index] = address + index * offset;
		}
		ret = DCP_STATUS_OK;
	}

	return ret;
}

UINT disp_deque_buffer (disp_buffer_queue *que) {
	UINT slot = 0;
	unsigned long flag;
	spin_lock_irqsave(&que->lock, flag);
	slot = que->write_slot;
	spin_unlock_irqrestore(&que->lock, flag);
	MMProfileLogEx(MTKFB_MMP_Events.deque_buf, MMProfileFlagPulse, que->read_slot, que->write_slot);
	return que->buffer_queue[slot];
}

void disp_enque_buffer (disp_buffer_queue *que) {
	UINT slot = 0;
	unsigned long flag;
	spin_lock_irqsave(&que->lock, flag);
	que->reserved++;
	if (que->reserved == 1) {
		que->read_slot = que->write_slot;
	}
	slot = (que->write_slot + 1) % que->buffer_count;
	if (slot != que->read_slot) {
		que->write_slot = slot;
	}
	spin_unlock_irqrestore(&que->lock, flag);
	MMProfileLogEx(MTKFB_MMP_Events.enque_buf, MMProfileFlagPulse, que->read_slot, que->write_slot);
}

UINT disp_request_buffer (disp_buffer_queue *que) {
	UINT slot;
	unsigned long flag;
	spin_lock_irqsave(&que->lock, flag);
	slot = que->read_slot;
	spin_unlock_irqrestore(&que->lock, flag);
	MMProfileLogEx(MTKFB_MMP_Events.request_buf, MMProfileFlagPulse, que->read_slot, que->write_slot);
	return que->buffer_queue[slot];
}

void disp_release_buffer (disp_buffer_queue *que) {
	UINT slot;
	unsigned long flag;
	spin_lock_irqsave(&que->lock, flag);
	slot = (que->read_slot + 1) % que->buffer_count;
	if (slot != que->write_slot) {
		que->read_slot = slot;
	}
	spin_unlock_irqrestore(&que->lock, flag);
	MMProfileLogEx(MTKFB_MMP_Events.release_buf, MMProfileFlagPulse, que->read_slot, que->write_slot);
}

BOOL disp_acquire_buffer (disp_buffer_queue *que) {
	BOOL ret = 0;
	unsigned long flag;
	spin_lock_irqsave(&que->lock, flag);
	if (que->reserved > 0) {
		que->reserved--;
		ret = 1;
	}
	spin_unlock_irqrestore(&que->lock, flag);
	MMProfileLogEx(MTKFB_MMP_Events.acquire_buf, MMProfileFlagPulse, ret, que->reserved);
	return ret;
}


///=============================================================================
// local function definitions
///==========================
static void disp_init_job(disp_job *job) {
	mutex_init(&job->lock);
	INIT_LIST_HEAD(&job->list);
	job->status = FREE;

	job->group_id = UNKNOWN_GROUP_ID;
	memset(&job->input, 0, sizeof(job->input));
	memset(&job->output, 0, sizeof(job->output));
}

/**
 * Get a freed job from Job Pool, if empty create a new one
 * @return job is initialized
 */
static disp_job* disp_create_job (void) {
	disp_job *job = NULL;
	mutex_lock(&job_pool_lock);
	if (!list_empty(&job_pool_head)) {
		job = list_first_entry(&job_pool_head, disp_job, list);
		list_del_init(&job->list);
	}
	mutex_unlock(&job_pool_lock);
	if (job == NULL) {
		job = kzalloc(sizeof(disp_job), GFP_KERNEL);
		XLOG_DBG("create new job node 0x%p\n", job);
	}
	disp_init_job(job);
	return job;
}



