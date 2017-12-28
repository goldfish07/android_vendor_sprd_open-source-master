/*
 *  log_ctrl.cpp - The log controller class implementation.
 *
 *  Copyright (C) 2015 Spreadtrum Communications Inc.
 *
 *  History:
 *  2015-2-16 Zhang Ziyi
 *  Initial version.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#ifdef HOST_TEST_
	#include "prop_test.h"
#else
	#include <cutils/properties.h>
#endif

#include "cp_dir.h"
#include "cp_stor.h"
#include "def_config.h"
#include "ext_wcn_log.h"
#include "log_ctrl.h"
#include "req_err.h"
#include "slog_config.h"
#include "wan_modem_log.h"

LogController::LogController()
	:m_config {0},
	 m_save_md {false},
	 m_cli_mgr {0},
	 m_modem_state {0},
	 m_wcn_state {0}
{
}

LogController::~LogController()
{
	delete m_wcn_state;
	delete m_modem_state;
	clear_ptr_container(m_log_pipes);
	delete m_cli_mgr;
}

LogPipeHandler* LogController::create_handler(const LogConfig::ConfigEntry* e)
{
	LogPipeHandler* log_pipe;

	if (CT_WCN == e->type) {
		log_pipe = new ExtWcnLogHandler(this, &m_multiplexer, e,
						m_stor_mgr);
	} else {
		log_pipe = new WanModemLogHandler(this, &m_multiplexer, e,
						  m_stor_mgr);
	}

	if (e->enable) {
		bool same;
		LogString log_path;
		LogString diag_path;
		int err = get_dev_paths(e->type, same, log_path, diag_path);

		if (!err) {
			if (same) {
				info_log("CP %s dev %s",
					 ls2cstring(e->modem_name),
					 ls2cstring(log_path));
				log_pipe->set_log_diag_dev_path(log_path);
			} else {
				info_log("CP %s dev %s %s",
					 ls2cstring(e->modem_name),
					 ls2cstring(log_path),
					 ls2cstring(diag_path));
				log_pipe->set_log_diag_dev_path(log_path,
								diag_path);
			}
			err = log_pipe->start();
			if (err < 0) {
				err_log("slogcp: start %s log failed",
					ls2cstring(e->modem_name));
			}
		} else {
			err_log("Can not get log/diag path for %s",
				ls2cstring(e->modem_name));
		}
	}
	return log_pipe;
}

int LogController::init(LogConfig* config)
{
	// Init the /data statistics
	uint64_t log_sz = config->max_data_part_size();
	log_sz <<= 20;
	m_stor_mgr.set_data_part_limit(log_sz);
	log_sz = config->max_sd_size();
	log_sz <<= 20;
	m_stor_mgr.set_sd_limit(log_sz);
	m_stor_mgr.set_file_size_limit(config->max_log_file() << 20);
	m_stor_mgr.set_overwrite(config->overwrite_old_log());

	// Initialize the storage manager
	int err = m_stor_mgr.init(config->sd_top_dir(), this, &m_multiplexer);
	if (err < 0) {
		return -1;
	}
	m_stor_mgr.set_ext_stor_type(config->ext_stor_type());

	// Create LogPipeHandlers according to settings in config
	// And add them to m_log_pipes
	m_config = config;
	m_save_md = config->md_enabled();

	const LogConfig::ConfigList& conf_list = config->get_conf();
	for(LogConfig::ConstConfigIter it = conf_list.begin();
	    it != conf_list.end(); ++it) {
		LogConfig::ConfigEntry* pc = *it;
		LogPipeHandler* handler = create_handler(pc);
		if (handler) {
			m_log_pipes.push_back(handler);
		}
	}

	// Server socket for clients (Engineering mode)
	m_cli_mgr = new ClientManager(this, &m_multiplexer);
	if (m_cli_mgr->init(SLOG_MODEM_SERVER_SOCK_NAME) < 0) {
		delete m_cli_mgr;
		m_cli_mgr = 0;
	}

	// Connection to modemd/wcnd
	init_state_handler(m_modem_state, MODEM_SOCKET_NAME);
	init_wcn_state_handler(m_wcn_state, WCN_SOCKET_NAME);

	return 0;
}

void LogController::init_state_handler(ModemStateHandler*& handler,
				       const char* serv_name)
{
	ModemStateHandler* p = new ModemStateHandler(this, &m_multiplexer,
						     serv_name);

	int err = p->init();
	if (err < 0) {
		delete p;
		err_log("slogcp: connect to %s error", serv_name);
	} else {
		handler = p;
	}
}

template<typename T>
void LogController::init_wcn_state_handler(T*& handler,
					   const char* serv_name)
{
	T* p = new T(this, &m_multiplexer, serv_name);

	int err = p->init();
	if (err < 0) {
		delete p;
		err_log("connect to %s error", serv_name);
	} else {
		handler = p;
	}
}

int LogController::run()
{
	return m_multiplexer.run();
}

void LogController::process_cp_alive(CpType type)
{
	// Open on alive
	LogList<LogPipeHandler*>::iterator it;

	for (it = m_log_pipes.begin(); it != m_log_pipes.end(); ++it) {
		LogPipeHandler* p = *it;
		if (p->type() == type) {
			p->open_on_alive();
			break;
		}
	}
}

void LogController::process_cp_assert(CpType type)
{
	LogList<LogPipeHandler*>::iterator it = find_log_handler(m_log_pipes,
								 type);
	if (it == m_log_pipes.end()) {
		err_log("slogcp: unknown MODEM type %d asserts", type);
		return;
	}

	info_log("CP %s assert", ls2cstring((*it)->name()));

	LogPipeHandler* log_hdl = *it;
	log_hdl->process_assert(m_save_md);
}

void LogController::process_cp_blocked(CpType type)
{
	process_cp_assert(type);
}

int LogController::save_mini_dump(CpStorage* stor,
				  const struct tm& lt)
{
	char md_name[80];

	snprintf(md_name, 80,
		 "mini_dump_%04d-%02d-%02d_%02d-%02d-%02d.bin",
		 lt.tm_year + 1900,
		 lt.tm_mon + 1,
		 lt.tm_mday,
		 lt.tm_hour,
		 lt.tm_min,
		 lt.tm_sec);

	LogFile* f = stor->create_file(LogString(md_name),
				       LogFile::LT_MINI_DUMP);
	int ret = -1;
	if (f) {
		ret = f->copy(MINI_DUMP_SRC_FILE);
		f->close();
		if (ret) {
			err_log("save mini dump error");
			f->dir()->remove(f);
		}
	} else {
		err_log("create mini dump file %s failed",
			md_name);
	}
	return ret;
}

int LogController::save_sipc_dump(CpStorage* stor,
				  const struct tm& lt)
{
	char fn[64];
	int year = lt.tm_year + 1900;
	int mon = lt.tm_mon + 1;

	// /d/sipc/smsg
	snprintf(fn, 64, "smsg_%04d-%02d-%02d_%02d-%02d-%02d.log",
		 year, mon, lt.tm_mday,
		 lt.tm_hour, lt.tm_min, lt.tm_sec);
	int err = 0;

	LogFile* f = stor->create_file(LogString(fn), LogFile::LT_SIPC);
	if (f) {
		if (f->copy(DEBUG_SMSG_PATH)) {
			err_log("slogcp: save SIPC smsg info failed");
			err = -1;
		}
		f->close();
	}

	// /d/sipc/sbuf
	snprintf(fn, 64, "sbuf_%04d-%02d-%02d_%02d-%02d-%02d.log",
		 year, mon, lt.tm_mday,
		 lt.tm_hour, lt.tm_min, lt.tm_sec);
	f = stor->create_file(LogString(fn), LogFile::LT_SIPC);
	if (f) {
		if (f->copy(DEBUG_SBUF_PATH)) {
			err_log("slogcp: save SIPC sbuf info failed");
			err = -1;
		}
		f->close();
	}

	// /d/sipc/sblock
	snprintf(fn, 64, "sblock_%04d-%02d-%02d_%02d-%02d-%02d.log",
		 year, mon, lt.tm_mday,
		 lt.tm_hour, lt.tm_min, lt.tm_sec);
	f = stor->create_file(LogString(fn), LogFile::LT_SIPC);
	if (f) {
		if (f->copy(DEBUG_SBLOCK_PATH)) {
			err_log("slogcp: save SIPC sblock info failed");
			err = -1;
		}
		f->close();
	}

	return err;
}

LogList<LogPipeHandler*>::iterator
LogController::find_log_handler(LogList<LogPipeHandler*>& handlers,
				CpType t)
{
	LogList<LogPipeHandler*>::iterator it;

	for (it = handlers.begin(); it != handlers.end(); ++it) {
		if (t == (*it)->type()) {
			break;
		}
	}

	return it;
}

int LogController::enable_log(const CpType* cps, size_t num)
{
	int mod_num = 0;
	size_t i;

	for (i = 0; i < num; ++i) {
		LogList<LogPipeHandler*>::iterator it = find_log_handler(m_log_pipes,
									 cps[i]);
		if (it != m_log_pipes.end()) {
			LogPipeHandler* p = *it;
			if (!p->enabled()) {
				bool same;
				LogString log_path;
				LogString diag_path;
				int err = get_dev_paths(p->type(), same,
							log_path, diag_path);
				if (!err) {
					if (same) {
						p->set_log_diag_dev_path(log_path);
					} else {
						p->set_log_diag_dev_path(log_path,
										diag_path);
					}
					p->start();
					m_config->enable_log(p->type());
				} else {
					err_log("Can not get log/diag path for %s",
						ls2cstring(p->name()));
				}
				++mod_num;
			}
		}
	}

	if (mod_num) {
		int err = m_config->save();
		if (err) {
			err_log("save config file failed");
		}
	}

	return 0;
}

int LogController::disable_log(const CpType* cps, size_t num)
{
	int mod_num = 0;
	size_t i;

	for (i = 0; i < num; ++i) {
		LogList<LogPipeHandler*>::iterator it = find_log_handler(m_log_pipes,
									 cps[i]);
		if (it != m_log_pipes.end()) {
			LogPipeHandler* p = *it;
			if (p->enabled()) {
				p->stop();
				m_config->enable_log(p->type(), false);
				++mod_num;
			}
		}
	}

	if (mod_num) {
		int err = m_config->save();
		if (err) {
			err_log("save config file failed");
		}
	}

	return 0;
}

int LogController::enable_md()
{
	if (!m_save_md) {
		m_save_md = true;
		m_config->enable_md();
		if (m_config->save()) {
			err_log("save config file failed");
		}
	}

	return 0;
}

int LogController::disable_md()
{
	if (m_save_md) {
		m_save_md = false;
		m_config->enable_md(false);
		if (m_config->save()) {
			err_log("save config file failed");
		}
	}

	return 0;
}

int LogController::mini_dump()
{
	time_t t = time(0);
	struct tm lt;

	if (static_cast<time_t>(-1) == t || !localtime_r(&t, &lt)) {
		return -1;
	}
	// TODO: use m_stor_mgr to save file
	return -1;
}

int LogController::reload_slog_conf()
{
	// Reload the /data/local/tmp/slog/slog.conf, and only update
	// the CP enable states.
	SLogConfig slogc;

	if (slogc.read_config(TMP_SLOG_CONFIG)) {
		err_log("load slog.conf failed");
		return -1;
	}

	const SLogConfig::ConfigList& clist = slogc.get_conf();
	SLogConfig::ConstConfigIter it;

	for (it = clist.begin(); it != clist.end(); ++it) {
		const SLogConfig::ConfigEntry* p = *it;
		LogList<LogPipeHandler*>::iterator it_cp = find_log_handler(m_log_pipes,
									    p->type);
		if (it_cp != m_log_pipes.end()) {
			LogPipeHandler* pipe_hdl = *it_cp;
			if (pipe_hdl->enabled() != p->enable) {
				info_log("%s CP %s",
					 p->enable ? "enable" : "disable",
					 ls2cstring(pipe_hdl->name()));
				if (p->enable) {
					bool same;
					LogString log_path;
					LogString diag_path;
					int err = get_dev_paths(p->type, same,
								log_path, diag_path);
					if (!err) {
						if (same) {
							pipe_hdl->set_log_diag_dev_path(log_path);
						} else {
							pipe_hdl->set_log_diag_dev_path(log_path, diag_path);
						}
						pipe_hdl->start();
					} else {
						err_log("Can not get log/diag path for %s",
							ls2cstring(pipe_hdl->name()));
					}
				} else {
					pipe_hdl->stop();
				}
				m_config->enable_log(p->type, p->enable);
			}
		}
	}

	if (m_config->dirty()) {
		if (m_config->save()) {
			err_log("save config failed");
		}
	}

	return 0;
}

size_t LogController::get_log_file_size() const
{
	return m_config->max_log_file();
}

int LogController::set_log_file_size(size_t len)
{
	m_stor_mgr.set_file_size_limit(len << 20);

	m_config->set_log_file_size(len);
	if (m_config->dirty()) {
		if (m_config->save()) {
			err_log("save config file failed");
		}
	}

	return 0;
}

int LogController::set_log_overwrite(bool en)
{
	m_stor_mgr.set_overwrite(en);

	m_config->set_overwrite(en);
	if (m_config->dirty()) {
		if (m_config->save()) {
			err_log("save config file failed");
		}
	}

	return 0;
}

size_t LogController::get_data_part_size() const
{
	return m_config->max_data_part_size();
}

int LogController::set_data_part_size(size_t sz)
{
	uint64_t byte_sz = sz;

	byte_sz <<= 20;
	m_stor_mgr.set_data_part_limit(byte_sz);

	m_config->set_data_part_size(sz);
	if (m_config->dirty()) {
		if (m_config->save()) {
			err_log("save config file failed");
		}
	}

	return 0;
}

size_t LogController::get_sd_size() const
{
	return m_config->max_sd_size();
}

int LogController::set_sd_size(size_t sz)
{
	uint64_t byte_sz = sz;

	byte_sz <<= 20;
	m_stor_mgr.set_sd_limit(byte_sz);

	m_config->set_sd_size(sz);
	if (m_config->dirty()) {
		if (m_config->save()) {
			err_log("save config file failed");
		}
	}

	return 0;
}

bool LogController::get_log_overwrite() const
{
	return m_config->overwrite_old_log();
}

int LogController::clear_log()
{
	LogList<LogPipeHandler*>::iterator it;

	for (it = m_log_pipes.begin(); it != m_log_pipes.end(); ++it) {
		if ((*it)->in_transaction()) {
			return LCR_IN_TRANSACTION;
		}
	}
	m_stor_mgr.clear();

	return LCR_SUCCESS;
}

LogPipeHandler* LogController::get_cp(CpType type)
{
	LogList<LogPipeHandler*>::iterator it = find_log_handler(m_log_pipes,
								 type);
	LogPipeHandler* p = 0;
	if (it != m_log_pipes.end()) {
		p = (*it);
	}
	return p;
}
