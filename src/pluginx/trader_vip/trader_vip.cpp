/*
* Copyright (c) 2018-2018 the TradeX authors
* All rights reserved.
*
* The project sponsor and lead author is Xu Rendong.
* E-mail: xrd@ustc.edu, QQ: 277195007, WeChat: ustc_xrd
* You can get more information at https://xurendong.github.io
* For names of other contributors see the contributors file.
*
* Commercial use of this code in source and binary forms is
* governed by a LGPL v3 license. You may get a copy from the
* root directory. Or else you should get a specific written
* permission from the project author.
*
* Individual and educational use of this code in source and
* binary forms is governed by a 3-clause BSD license. You may
* get a copy from the root directory. Certainly welcome you
* to contribute code of all sorts.
*
* Be sure to retain the above copyright notice and conditions.
*/

#include <boost/algorithm/string.hpp>

#include "trader_vip.h"
#include "trader_vip_.h"

TraderVIP the_app; // ����

// Ԥ�� TraderVIP_P ����ʵ��

TraderVIP_P::TraderVIP_P()
	: m_location( "" )
	, m_info_file_path( "" )
	, m_cfg_file_path( "" )
	, m_service_running( false )
	, m_work_thread_number( CFG_WORK_THREAD_NUM )
	, m_plugin_running( false )
	, m_task_id( 0 )
	, m_count_session_id( 0 )
	, m_net_server_trade( nullptr )
	, m_log_cate( "<TRADER_VIP>" ) {
	m_json_reader_builder["collectComments"] = false;
	m_json_reader_trader = m_json_reader_builder.newCharReader();
	m_syslog = basicx::SysLog_S::GetInstance();
	m_syscfg = basicx::SysCfg_S::GetInstance();
	m_sysrtm = basicx::SysRtm_S::GetInstance();
	m_plugins = basicx::Plugins::GetInstance();
	m_risker = boost::make_shared<Risker>( this );
}

TraderVIP_P::~TraderVIP_P() {
}

void TraderVIP_P::SetGlobalPath() {
	m_location = m_plugins->GetPluginLocationByName( PLUGIN_NAME );
	m_cfg_file_path = m_plugins->GetPluginCfgFilePathByName( PLUGIN_NAME );
	m_info_file_path = m_plugins->GetPluginInfoFilePathByName( PLUGIN_NAME );
}

bool TraderVIP_P::ReadConfig( std::string file_path ) {
	std::string log_info;

	pugi::xml_document document;
	if( !document.load_file( file_path.c_str() ) ) {
		FormatLibrary::StandardLibrary::FormatTo( log_info, "�򿪲������������Ϣ�ļ�ʧ�ܣ�{0}", file_path );
		LogPrint( basicx::syslog_level::c_error, log_info );
		return false;
	}

	pugi::xml_node node_plugin = document.document_element();
	if( node_plugin.empty() ) {
		log_info = "��ȡ�������������Ϣ ���ڵ� ʧ�ܣ�";
		LogPrint( basicx::syslog_level::c_error, log_info );
		return false;
	}

	m_configs.m_address = node_plugin.child_value( "Address" );
	m_configs.m_time_out = atoi( node_plugin.child_value( "TimeOut" ) );
	m_configs.m_app_name = node_plugin.child_value( "AppName" );
	m_configs.m_version = node_plugin.child_value( "Version" );
	m_configs.m_node_info = node_plugin.child_value( "NodeInfo" );
	m_configs.m_sys_user_id = node_plugin.child_value( "SysUserID" );
	m_configs.m_wtfs = node_plugin.child_value( "WTFS" );
	m_configs.m_src_fbdm = node_plugin.child_value( "SrcFBDM" );
	m_configs.m_des_fbdm = node_plugin.child_value( "DesFBDM" );

	//FormatLibrary::StandardLibrary::FormatTo( log_info, "{0} {1} {2} {3} {4} {5} {6} {7} {8}", m_configs.m_address, m_configs.m_time_out, 
	//	m_configs.m_app_name, m_configs.m_version, m_configs.m_node_info, m_configs.m_sys_user_id, m_configs.m_wtfs, m_configs.m_src_fbdm, m_configs.m_des_fbdm );
	//LogPrint( basicx::syslog_level::c_debug, log_info );

	log_info = "�������������Ϣ��ȡ��ɡ�";
	LogPrint( basicx::syslog_level::c_info, log_info );

	return true;
}

void TraderVIP_P::LogPrint( basicx::syslog_level log_level, std::string& log_info, int32_t log_show/* = 0*/ ) {
	if( 0 == log_show ) {
		m_syslog->LogPrint( log_level, m_log_cate, "LOG>: VIP " + log_info ); // ����̨
		m_sysrtm->LogTrans( log_level, m_log_cate, log_info ); // Զ�̼��
	}
	m_syslog->LogWrite( log_level, m_log_cate, log_info );
}

bool TraderVIP_P::Initialize() {
	SetGlobalPath();
	ReadConfig( m_cfg_file_path );

	m_thread_service = boost::make_shared<boost::thread>( boost::bind( &TraderVIP_P::CreateService, this ) );
	m_thread_user_request = boost::make_shared<boost::thread>( boost::bind( &TraderVIP_P::HandleUserRequest, this ) );

	return true;
}

bool TraderVIP_P::InitializeExt() {
	return true;
}

bool TraderVIP_P::StartPlugin() {
	std::string log_info;

	try {
		log_info = "��ʼ���ò�� ....";
		LogPrint( basicx::syslog_level::c_info, log_info );

		m_risker->Start( m_cfg_file_path, m_syscfg->GetCfgBasic() ); //

		StartNetServer(); //

		if( !Fix_Initialize() ) {
			log_info = "����ӿ� ��ʼ�� ʧ�ܣ�";
			LogPrint( basicx::syslog_level::c_error, log_info );
			return false;
		}

		if( !InitFixInfo() ) {
			log_info = "�����̨ ��ʼ�� ʧ�ܣ�";
			LogPrint( basicx::syslog_level::c_error, log_info );
			return false;
		}

		// TODO����Ӹ����ʼ������

		log_info = "���������ɡ�";
		LogPrint( basicx::syslog_level::c_info, log_info );

		m_plugin_running = true; // ��Ҫ�ڴ����߳�ǰ��ֵΪ��

		return true;
	} // try
	catch( ... ) {
		log_info = "�������ʱ����δ֪����";
		LogPrint( basicx::syslog_level::c_error, log_info );
	}

	return false;
}

bool TraderVIP_P::IsPluginRun() {
	return m_plugin_running;
}

bool TraderVIP_P::StopPlugin() {
	std::string log_info;

	try {
		log_info = "��ʼֹͣ��� ....";
		LogPrint( basicx::syslog_level::c_info, log_info );

		if( !Fix_Uninitialize() ) {
			log_info = "����ӿ� ����ʼ�� ʧ�ܣ�";
			LogPrint( basicx::syslog_level::c_error, log_info );
			return false;
		}

		m_risker->Stop(); //

		for( auto it_s = m_map_session.begin(); it_s != m_map_session.end(); it_s++ ) {
			if( it_s->second != nullptr ) {
				delete it_s->second;
			}
		}
		m_map_session.clear();

		for( size_t i = 0; i < m_vec_useless_session.size(); i++ ) {
			if( m_vec_useless_session[i] != nullptr ) {
				delete m_vec_useless_session[i];
			}
		}
		m_vec_useless_session.clear();

		// TODO����Ӹ��෴��ʼ������

		log_info = "���ֹͣ��ɡ�";
		LogPrint( basicx::syslog_level::c_info, log_info );

		m_plugin_running = false;

		return true;
	} // try
	catch( ... ) {
		log_info = "���ֹͣʱ����δ֪����";
		LogPrint( basicx::syslog_level::c_error, log_info );
	}

	return false;
}

bool TraderVIP_P::UninitializeExt() {
	return true;
}

bool TraderVIP_P::Uninitialize() {
	if( true == m_service_running ) {
		m_service_running = false;
		m_service->stop();
	}
	
	return true;
}

bool TraderVIP_P::AssignTask( int32_t task_id, int32_t identity, int32_t code, std::string& data ) {
	try {
		m_task_list_lock.lock();
		bool write_in_progress = !m_list_task.empty();
		TaskItem task_item;
		m_list_task.push_back( task_item );
		TaskItem& task_item_ref = m_list_task.back();
		task_item_ref.m_task_id = task_id;
		task_item_ref.m_identity = identity;
		task_item_ref.m_code = code;
		task_item_ref.m_data = data;
		m_task_list_lock.unlock();
		
		if( !write_in_progress && true == m_service_running ) {
			m_service->post( boost::bind( &TraderVIP_P::HandleTaskMsg, this ) );
		}

		return true;
	}
	catch( std::exception& ex ) {
		std::string log_info;
		FormatLibrary::StandardLibrary::FormatTo( log_info, "��� TaskItem ��Ϣ �쳣��{0}", ex.what() );
		LogPrint( basicx::syslog_level::c_error, log_info );
	}

	return false;
}

void TraderVIP_P::CreateService() {
	std::string log_info;

	log_info = "����������������߳����, ��ʼ��������������� ...";
	LogPrint( basicx::syslog_level::c_info, log_info );

	try {
		try {
			m_service = boost::make_shared<boost::asio::io_service>();
			boost::asio::io_service::work work( *m_service );

			for( int32_t i = 0; i < m_work_thread_number; i++ ) {
				thread_ptr thread_service( new boost::thread( boost::bind( &boost::asio::io_service::run, m_service ) ) );
				m_vec_work_thread.push_back( thread_service );
			}

			m_service_running = true;

			for( size_t i = 0; i < m_vec_work_thread.size(); i++ ) { // �ȴ������߳��˳�
				m_vec_work_thread[i]->join();
			}
		}
		catch( std::exception& ex ) {
			FormatLibrary::StandardLibrary::FormatTo( log_info, "����������� ��ʼ�� �쳣��{0}", ex.what() );
			LogPrint( basicx::syslog_level::c_error, log_info );
		}
	} // try
	catch( ... ) {
		log_info = "������������̷߳���δ֪����";
		LogPrint( basicx::syslog_level::c_fatal, log_info );
	}

	if( true == m_service_running ) {
		m_service_running = false;
		m_service->stop();
	}

	log_info = "������������߳��˳���";
	LogPrint( basicx::syslog_level::c_warn, log_info );
}

void TraderVIP_P::HandleTaskMsg() {
	std::string log_info;
	
	try {
		std::string result_data = "";
		TaskItem* task_item = &m_list_task.front(); // �϶���

		//FormatLibrary::StandardLibrary::FormatTo( log_info, "��ʼ���� {0} ������ ...", task_item->m_task_id );
		//LogPrint( basicx::syslog_level::c_info, log_info );

		int32_t func_id = 0;
		int32_t task_id = 0;
		try {
			Request request;
			request.m_task_id = task_item->m_task_id;
			request.m_identity = task_item->m_identity;
			request.m_code = task_item->m_code;

			if( NW_MSG_CODE_JSON == task_item->m_code ) {
				LogPrint( basicx::syslog_level::c_debug, task_item->m_data, FILE_LOG_ONLY );

				std::string errors_json;
				if( m_json_reader_trader->parse( task_item->m_data.c_str(), task_item->m_data.c_str() + task_item->m_data.length(), &request.m_req_json, &errors_json ) ) { // �����ģ�std::string strUser = StringToGB2312( jsRootR["user"].asString() );
					func_id = request.m_req_json["function"].asInt();
					task_id = request.m_req_json["task_id"].asInt();
				}
				else {
					FormatLibrary::StandardLibrary::FormatTo( log_info, "�������� {0} ʱ���� JSON ����ʧ�ܣ�{1}", task_item->m_task_id, errors_json );
					result_data = OnErrorResult( func_id, -1, log_info, task_id, task_item->m_code );
				}
			}
			else {
				FormatLibrary::StandardLibrary::FormatTo( log_info, "�������� {0} ʱ���ݱ����ʽ�쳣��", task_item->m_task_id );
				task_item->m_code = NW_MSG_CODE_JSON; // �����ʽδ֪ʱĬ��ʹ��
				result_data = OnErrorResult( func_id, -1, log_info, task_id, task_item->m_code );
			}

			if( func_id > 0 ) {
				switch( func_id ) { // Ϊ���� break; ������������ if else ��ʽ
				case TD_FUNC_STOCK_LOGIN: // ��¼����
					m_user_request_list_lock.lock();
					m_list_user_request.push_back( request );
					m_user_request_list_lock.unlock();
					break;
				case TD_FUNC_STOCK_LOGOUT: // �ǳ�����
					m_user_request_list_lock.lock();
					m_list_user_request.push_back( request );
					m_user_request_list_lock.unlock();
					break;
				default: // �Ự�Դ�������
					int32_t session_id = 0;
					if( NW_MSG_CODE_JSON == request.m_code ) {
						session_id = request.m_req_json["session"].asInt();
					}
					if( session_id <= 0 ) {
						FormatLibrary::StandardLibrary::FormatTo( log_info, "�������� {0} ʱ�Ự�쳣��session��{1}", task_item->m_task_id, session_id );
						result_data = OnErrorResult( func_id, -1, log_info, task_id, request.m_code );
						break;
					}
					Session* session = nullptr;
					m_session_map_lock.lock();
					std::map<int32_t, Session*>::iterator it_s = m_map_session.find( session_id );
					if( it_s != m_map_session.end() ) {
						session = it_s->second;
					}
					m_session_map_lock.unlock();
					if( nullptr == session ) {
						FormatLibrary::StandardLibrary::FormatTo( log_info, "�������� {0} ʱ�ûỰ�����ڣ�session��{1}", task_item->m_task_id, session_id );
						result_data = OnErrorResult( func_id, -1, log_info, task_id, request.m_code );
						break;
					}
					// ����ʱֻʹ�ûỰ�Ŷ������˺�����ʱ����ü���¸������Ƿ�����֤����ȻֻҪ��ȡ�Ự�����ø��˺Ž���
					//bool is_authorized = false;
					//session->m_con_endpoint_map_lock.lock();
					//std::map<int32_t, int32_t>::iterator it_ce = session->m_map_con_endpoint.find( request.m_identity );
					//if( it_ce != session->m_map_con_endpoint.end() ) {
					//	is_authorized = true;
					//}
					//session->m_con_endpoint_map_lock.unlock();
					//if( false == is_authorized ) {
					//	FormatLibrary::StandardLibrary::FormatTo( log_info, "�������� {0} ʱ������δ��֤��session��{1} endpoint��{2}", task_item->m_task_id, session_id, request.m_identity );
					//	result_data = OnErrorResult( func_id, -1, log_info, task_id, request.m_code );
					//	break;
					//}
					session->m_request_list_lock.lock();
					bool write_in_progress = !session->m_list_request.empty();
					session->m_list_request.push_back( request );
					session->m_request_list_lock.unlock();
					if( !write_in_progress && true == session->m_service_user_running ) { // m_service_user_running
						session->m_service_user->post( boost::bind( &Session::HandleRequestMsg, session ) );
					}
				}
			}
			else {
				if( "" == result_data ) { // ���� result_data ����
					FormatLibrary::StandardLibrary::FormatTo( log_info, "�������� {0} ʱ���ܱ���쳣��function��{1}", task_item->m_task_id, func_id );
					result_data = OnErrorResult( func_id, -1, log_info, task_id, request.m_code );
				}
			}
		}
		catch( ... ) {
			FormatLibrary::StandardLibrary::FormatTo( log_info, "�������� {0} ʱ�������󣬿��ܱ����ʽ�쳣��", task_item->m_task_id );
			task_item->m_code = NW_MSG_CODE_JSON; // �����ʽδ֪ʱĬ��ʹ��
			result_data = OnErrorResult( func_id, -1, log_info, task_id, task_item->m_code );
		}

		if( result_data != "" ) { // ������ת��ǰ�ͷ���������
			CommitResult( task_item->m_task_id, task_item->m_identity, task_item->m_code, result_data );
		}

		//FormatLibrary::StandardLibrary::FormatTo( log_info, "���� {0} ��������ɡ�", task_item->m_task_id );
		//LogPrint( basicx::syslog_level::c_info, log_info );

		m_task_list_lock.lock();
		m_list_task.pop_front();
		bool write_on_progress = !m_list_task.empty();
		m_task_list_lock.unlock();

		if( write_on_progress && true == m_service_running ) {
			m_service->post( boost::bind( &TraderVIP_P::HandleTaskMsg, this ) );
		}
	}
	catch( std::exception& ex ) {
		FormatLibrary::StandardLibrary::FormatTo( log_info, "���� TaskItem ��Ϣ �쳣��{0}", ex.what() );
		LogPrint( basicx::syslog_level::c_error, log_info );
	}
}

// �Զ��� TraderVIP_P ����ʵ��

bool TraderVIP_P::InitFixInfo() {
	std::string log_info;

	BOOL ret = Fix_SetAppInfo( m_configs.m_app_name.c_str(), m_configs.m_version.c_str() );
	if( !ret ) {
		FormatLibrary::StandardLibrary::FormatTo( log_info, "ִ�� Fix_SetAppInfo �쳣��AppName��{0}��Version��{1}", m_configs.m_app_name, m_configs.m_version );
		LogPrint( basicx::syslog_level::c_error, log_info );
		return false;
	}

	ret = Fix_SetDefaultInfo( m_configs.m_sys_user_id.c_str(), m_configs.m_wtfs.c_str(), m_configs.m_src_fbdm.c_str(), m_configs.m_des_fbdm.c_str() );
	if( !ret ) {
		FormatLibrary::StandardLibrary::FormatTo( log_info, "ִ�� Fix_SetDefaultInfo �쳣��SysUserID��{0}��WTFS��{1}��SrcFBDM��{2}��DesFBDM��{3}", 
			m_configs.m_sys_user_id, m_configs.m_wtfs, m_configs.m_src_fbdm, m_configs.m_des_fbdm );
		LogPrint( basicx::syslog_level::c_error, log_info );
		return false;
	}

	return true;
}

void TraderVIP_P::StartNetServer() {
	basicx::CfgBasic* cfg_basic = m_syscfg->GetCfgBasic();

	m_net_server_trade = boost::make_shared<basicx::NetServer>();
	m_net_server_trade->ComponentInstance( this );
	basicx::NetServerCfg net_server_cfg_temp;
	net_server_cfg_temp.m_log_test = cfg_basic->m_debug_infos_server;
	net_server_cfg_temp.m_heart_check_time = cfg_basic->m_heart_check_server;
	net_server_cfg_temp.m_max_msg_cache_number = cfg_basic->m_max_msg_cache_server;
	net_server_cfg_temp.m_io_work_thread_number = cfg_basic->m_work_thread_server;
	net_server_cfg_temp.m_client_connect_timeout = cfg_basic->m_con_time_out_server;
	net_server_cfg_temp.m_max_connect_total_s = cfg_basic->m_con_max_server_server;
	net_server_cfg_temp.m_max_data_length_s = cfg_basic->m_data_length_server;
	m_net_server_trade->StartNetwork( net_server_cfg_temp );

	for( size_t i = 0; i < cfg_basic->m_vec_server_server.size(); i++ ) {
		if( "trade_vip" == cfg_basic->m_vec_server_server[i].m_type && 1 == cfg_basic->m_vec_server_server[i].m_work ) {
			m_net_server_trade->Server_AddListen( "0.0.0.0", cfg_basic->m_vec_server_server[i].m_port, cfg_basic->m_vec_server_server[i].m_type ); // 0.0.0.0
		}
	}
}

void TraderVIP_P::OnNetServerInfo( basicx::NetServerInfo& net_server_info_temp ) {
}

void TraderVIP_P::OnNetServerData( basicx::NetServerData& net_server_data_temp ) {
	try {
		if( "trade_vip" == net_server_data_temp.m_node_type ) { // �ڵ����ͱ���������ļ�һ��
			AssignTask( GetTaskId(), net_server_data_temp.m_identity, net_server_data_temp.m_code, net_server_data_temp.m_data );
		}
	}
	catch( ... ) {
		std::string log_info = "ת�� Server �������� ����δ֪����";
		LogPrint( basicx::syslog_level::c_error, log_info );
	}
}

int32_t TraderVIP_P::GetTaskId() {
	if( m_task_id > 2147483600 ) { // 2 ^ 31 = 2147483648
		m_task_id = 0;
	}
	m_task_id++;
	return m_task_id;
}

int32_t TraderVIP_P::GetSessionID() { // int32_t->2147483647��(0000001~2147482)*1000+(000~999)
	if( 2147482 == m_count_session_id ) {
		m_count_session_id = 0;
	}
	m_count_session_id++; //
	srand( (int32_t)time( 0 ) );
	int32_t random = rand() % 1000;
	return m_count_session_id * 1000 + random;
}

void TraderVIP_P::HandleUserRequest() {
	std::string log_info;

	log_info = "������¼�ǳ��������߳����, ��ʼ���е�¼�ǳ������� ...";
	LogPrint( basicx::syslog_level::c_info, log_info );

	try {
		while( 1 ) {
			if( !m_list_user_request.empty() ) {
				std::string result_data = "";
				Request* request = &m_list_user_request.front();

				int32_t func_id = 0;
				int32_t task_id = 0;
				try {
					if( NW_MSG_CODE_JSON == request->m_code ) {
						func_id = request->m_req_json["function"].asInt();
						task_id = request->m_req_json["task_id"].asInt();
					}

					switch( func_id ) {
					case TD_FUNC_STOCK_LOGIN: // ��¼����
						result_data = OnUserLogin( request );
						break;
					case TD_FUNC_STOCK_LOGOUT: // �ǳ�����
						result_data = OnUserLogout( request ); // �ǳ����뽻���첽���У���Ҫ���˺����һλ�û���֤�ǳ�ʱ���н��������
						break;
					}
				}
				catch( ... ) {
					FormatLibrary::StandardLibrary::FormatTo( log_info, "�������� {0} ��¼�ǳ�����ʱ����δ֪����", request->m_task_id );
					result_data = OnErrorResult( func_id, -1, log_info, task_id, request->m_code );
				}

				CommitResult( request->m_task_id, request->m_identity, request->m_code, result_data );

				m_user_request_list_lock.lock();
				m_list_user_request.pop_front();
				m_user_request_list_lock.unlock();
			}

			Sleep( 1 );
		}
	} // try
	catch( ... ) {
		log_info = "��¼�ǳ��������̷߳���δ֪����";
		LogPrint( basicx::syslog_level::c_fatal, log_info );
	}

	log_info = "��¼�ǳ��������߳��˳���";
	LogPrint( basicx::syslog_level::c_warn, log_info );
}

// ȷ�� Connect - Session - UserName ���߶�Ӧ
std::string TraderVIP_P::OnUserLogin( Request* request ) {
	std::string log_info;

	int32_t task_id = 0;
	std::string username = "";
	std::string password = "";
	if( NW_MSG_CODE_JSON == request->m_code ) {
		task_id = request->m_req_json["task_id"].asInt();
		username = request->m_req_json["username"].asString();
		password = request->m_req_json["password"].asString();
	}
	if( "" == username || "" == password ) {
		log_info = "�����û���¼ʱ �˺� �� ���� Ϊ�գ�";
		return OnErrorResult( TD_FUNC_STOCK_LOGIN, -1, log_info, task_id, request->m_code );
	}
	
	std::map<int32_t, Session*> map_session_temp;
	m_session_map_lock.lock();
	map_session_temp = m_map_session; // �Ựָ��
	m_session_map_lock.unlock();

	bool is_exist = false;
	Session* session = nullptr;
	for( auto it_s = map_session_temp.begin(); it_s != map_session_temp.end(); it_s++ ) { // �û�����ܶ࣬Ŀǰ������ȶ԰�
		if( ( it_s->second)->m_username == username ) { // �Ѵ��ڸ��û����˺�
			if( ( it_s->second)->m_password != password ) { // ���״ε�¼��̨�˺�����У��
				log_info = "�����û���¼ʱ �˺� �� ���� ��ƥ�䣡";
				return OnErrorResult( TD_FUNC_STOCK_LOGIN, -1, log_info, task_id, request->m_code );
			}
			session = it_s->second; // �Ựָ��
			is_exist = true;
			break;
		}
	}
	if( false == is_exist ) {
		try {
			session = new Session( this ); //
		}
		catch( ... ) {
		}
	}
	if( nullptr == session ) {
		log_info = "�����û���¼ʱ �����Ự���� ʧ�ܣ�";
		return OnErrorResult( TD_FUNC_STOCK_LOGIN, -1, log_info, task_id, request->m_code );
	}
	
	if( false == is_exist && false == session->m_connect_ok ) { // ֻ���½��Ựʱ���˺����������Ƹ���
		session->m_connect = Fix_Connect( m_configs.m_address.c_str(), username.c_str(), password.c_str(), m_configs.m_time_out ); // Ĭ�ϻ��Զ�����
		if( 0 == session->m_connect ) {
			delete session; // �½��Ựʱ�϶����� m_map_session ��
			FormatLibrary::StandardLibrary::FormatTo( log_info, "���� �����̨ �쳣��address��{0}��timeout��{1}", m_configs.m_address, m_configs.m_time_out );
			return OnErrorResult( TD_FUNC_STOCK_LOGIN, -1, log_info, task_id, request->m_code );
		}
		else { // ���ӳɹ�������������У������
			long api_session = Fix_AllocateSession( session->m_connect );

			Fix_SetNode( api_session, m_configs.m_node_info.c_str() ); //
			int32_t ret = Fix_CreateReq( api_session, 610301 ); // �ͻ���������У��
			if( ret < 0 ) {
				delete session; // �½��Ựʱ�϶����� m_map_session ��
				Fix_ReleaseSession( api_session );
				log_info = "�����û� ����У�� ʱ���� Fix_CreateReq ʧ�ܣ�";
				return OnErrorResult( TD_FUNC_STOCK_LOGIN, -1, log_info, task_id, request->m_code );
			}

			Fix_SetString( api_session, 605, username.c_str() ); // 605 FID_KHH �ͻ���
			char c_password[64] = { 0 };
			strcpy_s( c_password, 64, password.c_str() );
			Fix_Encode( c_password );
			Fix_SetString( api_session, 598, c_password ); // 598 FID_JYMM ��������
			Fix_SetString( api_session, 781, "0" ); // 781 FID_JMLX ��������

			long fid_code = 0;
			char fid_message[VIP_FID_MESSAGE_LENGTH];
			memset( &fid_message, 0, VIP_FID_MESSAGE_LENGTH );

			if( Fix_Run( api_session ) ) {
				fid_code = Fix_GetLong( api_session, 507, 0 );
				Fix_GetItem( api_session, 508, fid_message, VIP_FID_MESSAGE_LENGTH, 0 );
				if( fid_code < 0 ) {
					delete session; // �½��Ựʱ�϶����� m_map_session ��
					Fix_ReleaseSession( api_session );
					FormatLibrary::StandardLibrary::FormatTo( log_info, "�����û� ����У�� ʧ�ܣ�{0}", fid_message );
					return OnErrorResult( TD_FUNC_STOCK_LOGIN, fid_code, log_info, task_id, request->m_code );
				}
			}
			else {
				fid_code = Fix_GetCode( api_session );
				Fix_GetErrMsg( api_session, fid_message, VIP_FID_MESSAGE_LENGTH );

				delete session; // �½��Ựʱ�϶����� m_map_session ��
				Fix_ReleaseSession( api_session );
				FormatLibrary::StandardLibrary::FormatTo( log_info, "�����û� ����У�� ʱִ�� Fix_Run ʧ�ܣ�{0}", fid_message );
				return OnErrorResult( TD_FUNC_STOCK_LOGIN, fid_code, log_info, task_id, request->m_code );
			}

			Fix_ReleaseSession( api_session );
		}
		// �൱���״ε�¼��̨ʱУ�����˺ź�����
		session->m_username = username;
		session->m_password = password;
		session->m_node_info = m_configs.m_node_info;
		session->m_sys_user_id = m_configs.m_sys_user_id;
		session->m_connect_ok = true;
	}
	session->m_con_endpoint_map_lock.lock();
	session->m_map_con_endpoint[request->m_identity] = request->m_identity;
	session->m_con_endpoint_map_lock.unlock();

	if( false == is_exist ) {
		session->m_thread_service_user = boost::make_shared<boost::thread>( boost::bind( &Session::CreateServiceUser, session ) );

		session->m_session = GetSessionID();
		m_session_map_lock.lock();
		m_map_session[session->m_session] = session;
		m_session_map_lock.unlock();

		// ��Ϊ��¼��Ϣ����Ϣ������˳��ִ�У����Բ���Ҫ����
		// ��Ϊ����ȫ�춼Ҫ���˺Ž��з�أ����Է�ض������˺ŵǳ�������
		session->m_risker = m_risker; //
		m_risker->PrintSessionAssetAccount( session->m_username );
	}

	FormatLibrary::StandardLibrary::FormatTo( log_info, "�����û���¼�ɹ���session��{0}", session->m_session );
	LogPrint( basicx::syslog_level::c_info, log_info );

	if( NW_MSG_CODE_JSON == request->m_code ) {
		Json::Value ret_data_json;
		ret_data_json["session"] = session->m_session;

		Json::Value results_json;
		results_json["ret_func"] = TD_FUNC_STOCK_LOGIN;
		results_json["ret_code"] = 0;
		results_json["ret_info"] = basicx::StringToUTF8( log_info );
		results_json["ret_task"] = task_id;
		results_json["ret_last"] = true;
		results_json["ret_numb"] = 1;
		results_json["ret_data"].append( ret_data_json );
		return Json::writeString( m_json_writer_trader, results_json );
	}

	return "";
}

std::string TraderVIP_P::OnUserLogout( Request* request ) {
	std::string log_info;

	int32_t session_id = 0;
	int32_t task_id = 0;
	if( NW_MSG_CODE_JSON == request->m_code ) {
		session_id = request->m_req_json["session"].asInt();
		task_id = request->m_req_json["task_id"].asInt();
	}
	if( session_id <= 0 ) {
		FormatLibrary::StandardLibrary::FormatTo( log_info, "�����û��ǳ�ʱ �Ự �쳣��session��{0}", session_id );
		return OnErrorResult( TD_FUNC_STOCK_LOGOUT, -1, log_info, task_id, request->m_code );
	}

	Session* session = nullptr;
	m_session_map_lock.lock();
	std::map<int32_t, Session*>::iterator it_s = m_map_session.find( session_id );
	if( it_s != m_map_session.end() ) {
		session = it_s->second;
	}
	m_session_map_lock.unlock();
	if( nullptr == session ) {
		FormatLibrary::StandardLibrary::FormatTo( log_info, "�����û��ǳ�ʱ �Ự �����ڣ�session��{0}", session_id );
		return OnErrorResult( TD_FUNC_STOCK_LOGOUT, -1, log_info, task_id, request->m_code );
	}

	session->m_con_endpoint_map_lock.lock();
	session->m_map_con_endpoint.erase( request->m_identity );
	session->m_con_endpoint_map_lock.unlock();

	if( session->m_map_con_endpoint.empty() ) { // ���������û�
		session->m_sub_endpoint_map_lock.lock();
		session->m_map_sub_endpoint.clear(); // ���ķ��ӽ���
		session->m_sub_endpoint_map_lock.unlock();
		if( true == session->m_subscibe_ok ) {
			session->m_subscibe_ok = false;
			Fix_UnSubscibeByHandle( session->m_subscibe_cj );
			Fix_UnSubscibeByHandle( session->m_subscibe_sb );
			Fix_UnSubscibeByHandle( session->m_subscibe_cd );
		}
		m_session_map_lock.lock();
		m_map_session.erase( session->m_session );
		m_session_map_lock.unlock();
		if( true == session->m_connect_ok ) {
			session->m_connect_ok = false;
			Fix_Close( session->m_connect );
		}
		session->StopServiceUser(); //
		m_vec_useless_session.push_back( session ); // Ŀǰ�� delete session;
	}

	FormatLibrary::StandardLibrary::FormatTo( log_info, "�����û��ǳ��ɹ���session��{0}", session_id );
	LogPrint( basicx::syslog_level::c_info, log_info );

	if( NW_MSG_CODE_JSON == request->m_code ) {
		Json::Value results_json;
		results_json["ret_func"] = TD_FUNC_STOCK_LOGOUT;
		results_json["ret_code"] = 0;
		results_json["ret_info"] = basicx::StringToUTF8( log_info );
		results_json["ret_task"] = task_id;
		results_json["ret_last"] = true;
		results_json["ret_numb"] = 0;
		results_json["ret_data"] = "";
		return Json::writeString( m_json_writer_trader, results_json );
	}

	return "";
}

void TraderVIP_P::CommitResult( int32_t task_id, int32_t identity, int32_t code, std::string& data ) {
	basicx::ConnectInfo* connect_info = m_net_server_trade->Server_GetConnect( identity );
	if( connect_info != nullptr ) {
		m_net_server_trade->Server_SendData( connect_info, NW_MSG_TYPE_USER_DATA, code, data );
	}
}

std::string TraderVIP_P::OnErrorResult( int32_t func_id, int32_t ret_code, std::string ret_info, int32_t ret_task, int32_t encode ) {
	LogPrint( basicx::syslog_level::c_error, ret_info );

	if( NW_MSG_CODE_JSON == encode ) {
		Json::Value results_json;
		results_json["ret_func"] = func_id;
		results_json["ret_code"] = ret_code;
		results_json["ret_info"] = basicx::StringToUTF8( ret_info );
		results_json["ret_task"] = ret_task;
		results_json["ret_last"] = true;
		results_json["ret_numb"] = 0;
		results_json["ret_data"] = "";
		return Json::writeString( m_json_writer_trader, results_json );
	}

	return "";
}

// Ԥ�� TraderVIP ����ʵ��

TraderVIP::TraderVIP()
	: basicx::Plugins_X( PLUGIN_NAME )
	, m_trader_vip_p( nullptr ) {
	try {
		m_trader_vip_p = new TraderVIP_P();
	}
	catch( ... ) {
	}
}

TraderVIP::~TraderVIP() {
	if( m_trader_vip_p != nullptr ) {
		delete m_trader_vip_p;
		m_trader_vip_p = nullptr;
	}
}

bool TraderVIP::Initialize() {
	return m_trader_vip_p->Initialize();
}

bool TraderVIP::InitializeExt() {
	return m_trader_vip_p->InitializeExt();
}

bool TraderVIP::StartPlugin() {
	return m_trader_vip_p->StartPlugin();
}

bool TraderVIP::IsPluginRun() {
	return m_trader_vip_p->IsPluginRun();
}

bool TraderVIP::StopPlugin() {
	return m_trader_vip_p->StopPlugin();
}

bool TraderVIP::UninitializeExt() {
	return m_trader_vip_p->UninitializeExt();
}

bool TraderVIP::Uninitialize() {
	return m_trader_vip_p->Uninitialize();
}

bool TraderVIP::AssignTask( int32_t task_id, int32_t identity, int32_t code, std::string& data ) {
	return m_trader_vip_p->AssignTask( task_id, identity, code, data );
}

// �Զ��� TraderVIP ����ʵ��
