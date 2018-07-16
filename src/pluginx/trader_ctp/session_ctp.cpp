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

#include "session_ctp.h"
#include "trader_ctp_.h"

Session::Session( TraderCTP_P* trader_ctp_p )
	: m_session( 0 )
	, m_username( "" )
	, m_password( "" )
	, m_broker_id( "" )
	, m_query_only( false )
	, m_service_user_running( false )
	, m_user_api( nullptr )
	, m_last_rsp_is_error( false )
	, m_last_error_msg( "�޴�����Ϣ��" )
	, m_request_id( 0 )
	, m_connect_ok( false )
	, m_login_ok( false )
	, m_logout_ok( false )
	, m_settle_ok( false )
	, m_front_id( 0 )
	, m_session_id( 0 )
	, m_log_cate( "<TRADER_CTP>" ) {
	m_syscfg = basicx::SysCfg_S::GetInstance();
	m_trader_ctp_p = trader_ctp_p; // ������Ƿ��ָ��
}

Session::~Session() {
	StopServiceUser();
}

void Session::OnFrontConnected() {
	m_connect_ok = true; // ֻ�����״����ӳɹ����
	std::string log_info = "����ǰ�����ӳɹ���";
	m_trader_ctp_p->LogPrint( basicx::syslog_level::c_info, log_info );
}

void Session::OnFrontDisconnected( int nReason ) { // CTP �� API ���Զ���������
	m_connect_ok = false;
	m_login_ok = false;
	m_settle_ok = false;

	std::string log_info;
	FormatLibrary::StandardLibrary::FormatTo( log_info, "����ǰ�������жϣ�{0}", nReason );
	m_trader_ctp_p->LogPrint( basicx::syslog_level::c_warn, log_info );
}

void Session::OnHeartBeatWarning( int nTimeLapse ) {
	std::string log_info;
	FormatLibrary::StandardLibrary::FormatTo( log_info, "������ʱ���棡{0}", nTimeLapse );
	m_trader_ctp_p->LogPrint( basicx::syslog_level::c_warn, log_info );
}

int32_t Session::ReqUserLogin( std::string broker_id, std::string user_id, std::string password ) {
	CThostFtdcReqUserLoginField req;
	memset( &req, 0, sizeof( req ) );
	strcpy_s( req.BrokerID, const_cast<char*>( broker_id.c_str()) ); // ���͹�˾���� char 11
	strcpy_s( req.UserID, const_cast<char*>( user_id.c_str()) ); // �û����� char 16
	strcpy_s( req.Password, const_cast<char*>( password.c_str()) ); // ���� char 41
	// req.TradingDay; // ������ char 9
	// req.UserProductInfo; // �û��˲�Ʒ��Ϣ char 11
	// req.InterfaceProductInfo; // �ӿڶ˲�Ʒ��Ϣ char 11
	// req.ProtocolInfo; // Э����Ϣ char 11
	// req.MacAddress; // Mac��ַ char 21
	// req.OneTimePassword; // ��̬���� char 41
	// req.ClientIPAddress; // �ն�IP��ַ char 16
	m_login_ok = false; // ֻ�����״ε�¼�ɹ����
	m_last_rsp_is_error = false;
	int32_t request_id = GetRequestID();
	return m_user_api->ReqUserLogin( &req, request_id );
}

void Session::OnRspUserLogin( CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast ) {
	std::string log_info;
	if( ( pRspInfo && pRspInfo->ErrorID != 0 ) || !pRspUserLogin ) {
		m_last_error_msg = pRspInfo->ErrorMsg;
		m_last_rsp_is_error = true;
		FormatLibrary::StandardLibrary::FormatTo( log_info, "�û���¼ʧ�ܣ�{0}", m_last_error_msg );
		m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );
	}
	else {
		m_login_ok = true; // ֻ�����״ε�¼�ɹ����
		m_front_id = pRspUserLogin->FrontID; // ǰ�ñ�� int
		m_session_id = pRspUserLogin->SessionID; // �Ự��� int
		// m_trader_ctp_p->m_order_ref = atoi( pRspUserLogin->MaxOrderRef ); // ��󱨵����� char 13 // ÿ���½��Ự CTP �������� OrderRef Ϊ 1 �����������Լ�����Ϊ׼
		// pRspUserLogin->TradingDay; // ������ char 9
		// pRspUserLogin->LoginTime; // ��¼�ɹ�ʱ�� char 9
		// pRspUserLogin->BrokerID; // ���͹�˾���� char 11
		// pRspUserLogin->UserID; // �û����� char 16
		// pRspUserLogin->SystemName; // ����ϵͳ���� char 41
		// pRspUserLogin->SHFETime; // ������ʱ�� char 9
		// pRspUserLogin->DCETime; // ������ʱ�� char 9
		// pRspUserLogin->CZCETime; // ֣����ʱ�� char 9
		// pRspUserLogin->FFEXTime; // �н���ʱ�� char 9
		FormatLibrary::StandardLibrary::FormatTo( log_info, "�û���¼�ɹ�����ǰ�����գ�{0}", pRspUserLogin->TradingDay );
		m_trader_ctp_p->LogPrint( basicx::syslog_level::c_info, log_info );
	}
}

int32_t Session::ReqUserLogout() {
	CThostFtdcUserLogoutField req;
	memset( &req, 0, sizeof( req ) );
	strcpy_s( req.BrokerID, const_cast<char*>(m_broker_id.c_str()) ); // ���͹�˾���� char 11
	strcpy_s( req.UserID, const_cast<char*>(m_username.c_str()) ); // �û����� char 16
	m_logout_ok = false; // ֻ����ĩ�εǳ��ɹ����
	m_last_rsp_is_error = false;
	int32_t request_id = GetRequestID();
	return m_user_api->ReqUserLogout( &req, request_id );
}

void Session::OnRspUserLogout( CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast ) {
	std::string log_info;
	if( ( pRspInfo && pRspInfo->ErrorID != 0 ) || !pUserLogout ) {
		m_last_error_msg = pRspInfo->ErrorMsg;
		m_last_rsp_is_error = true;
		FormatLibrary::StandardLibrary::FormatTo( log_info, "�û��ǳ�ʧ�ܣ�{0}", m_last_error_msg );
		m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );
	}
	else {
		m_logout_ok = true; // ֻ����ĩ�εǳ��ɹ����
		// pUserLogout->BrokerID; // ���͹�˾���� char 11
		// pUserLogout->UserID; // �û����� char 16
		FormatLibrary::StandardLibrary::FormatTo( log_info, "�û��ǳ��ɹ���{0} {1}", pUserLogout->BrokerID, pUserLogout->UserID );
		m_trader_ctp_p->LogPrint( basicx::syslog_level::c_info, log_info );
	}
}

int32_t Session::ReqSettlementInfoConfirm() {
	CThostFtdcSettlementInfoConfirmField req;
	memset( &req, 0, sizeof( req ) );
	strcpy_s( req.BrokerID, const_cast<char*>(m_broker_id.c_str()) ); // ���͹�˾���� char 11
	strcpy_s( req.InvestorID, const_cast<char*>(m_username.c_str()) ); // Ͷ���ߴ��� char 13
	// req.ConfirmDate; // ȷ������ char 9
	// req.ConfirmTime; // ȷ��ʱ�� char 9
	m_settle_ok = false; // ֻ�����״ν���ȷ�ϳɹ����
	m_last_rsp_is_error = false;
	int32_t request_id = GetRequestID();
	return m_user_api->ReqSettlementInfoConfirm( &req, request_id );
}

void Session::OnRspSettlementInfoConfirm( CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast ) {
	std::string log_info;
	if( ( pRspInfo && pRspInfo->ErrorID != 0 ) || !pSettlementInfoConfirm ) {
		m_last_error_msg = pRspInfo->ErrorMsg;
		m_last_rsp_is_error = true;
		FormatLibrary::StandardLibrary::FormatTo( log_info, "���㵥ȷ��ʧ�ܣ�{0}", m_last_error_msg );
		m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );
	}
	else {
		m_settle_ok = true; // ֻ�����״ν���ȷ�ϳɹ����
		// pSettlementInfoConfirm->BrokerID; // ���͹�˾���� char 11
		// pSettlementInfoConfirm->InvestorID; // Ͷ���ߴ��� char 13
		// pSettlementInfoConfirm->ConfirmDate; // ȷ������ char 9
		// pSettlementInfoConfirm->ConfirmTime; // ȷ��ʱ�� char 9
		FormatLibrary::StandardLibrary::FormatTo( log_info, "���㵥ȷ�ϳɹ���{0} {1} {2} {3}", 
			pSettlementInfoConfirm->BrokerID, pSettlementInfoConfirm->InvestorID, pSettlementInfoConfirm->ConfirmDate, pSettlementInfoConfirm->ConfirmTime );
		m_trader_ctp_p->LogPrint( basicx::syslog_level::c_info, log_info );
	}
}

// Ŀǰ��ʵ�ִ��������ܣ�һ��ͨ����������ִ�У���Ҫʱ�ټӰ�
// ��������ContingentCondition(�û��趨��������)��StopPrice(�û��趨ֹ���)��OrderPriceType(THOST_FTDC_OPT_Li mitPrice)��LimitPrice(�û��趨�۸�)��TimeCondition(THOST_FTDC_TC_GFD)
// ��������ֹӮ�Ǹ����ĸ��۸������StopPrice���������Ƿ�ͨ�� ReqParkedOrderInsert �� ReqParkedOrderAction ���У�
// ����ʱ���� Q7 �ͻ����µ�ί���� OrderRef ֱ���ǳ��� 12 ���ַ�����ǰ�������ֵĲ��ֲ��ո��
int32_t Session::ReqOrderInsert( Request* request, bool is_arbitrage ) {
	try {
		std::string instrument = "";
		double price = 0.0;
		int32_t amount = 0;
		int32_t entr_type = 0;
		int32_t exch_side = 0;
		int32_t offset = 0;
		int32_t hedge = 0;

		if( NW_MSG_CODE_JSON == request->m_code ) {
			instrument = request->m_req_json["instrument"].asCString();
			price = request->m_req_json["price"].asDouble();
			amount = request->m_req_json["amount"].asInt();
			entr_type = request->m_req_json["entr_type"].asInt();
			exch_side = request->m_req_json["exch_side"].asInt();
			offset = request->m_req_json["offset"].asInt();
			hedge = request->m_req_json["hedge"].asInt();
		}

		CThostFtdcInputOrderField req;
		memset( &req, 0, sizeof( req ) );
		strcpy_s( req.BrokerID, const_cast<char*>(m_broker_id.c_str()) ); // ���͹�˾���� char 11
		strcpy_s( req.InvestorID, const_cast<char*>(m_username.c_str()) ); // Ͷ���ߴ��� char 13
		std::string order_ref = "";
		FormatLibrary::StandardLibrary::FormatTo( order_ref, "{0}", m_trader_ctp_p->GetOrderRef() );
		strcpy_s( req.OrderRef, const_cast<char*>( order_ref.c_str()) ); // �������� char 13 // �������� CTP �Զ�����
		strcpy_s( req.InstrumentID, const_cast<char*>(instrument.c_str()) ); // ��Լ���� char 31
		req.VolumeTotalOriginal = amount; // ���� int
		if( false == is_arbitrage ) { // ������Լ
			if( 1 == entr_type ) { // ί�з�ʽ���޼�
				req.LimitPrice = price; // ί�м۸� double
				req.OrderPriceType = THOST_FTDC_OPT_LimitPrice; // �����۸����� char // �޼�
				req.TimeCondition = THOST_FTDC_TC_GFD; // ��Ч������ char // ������Ч
			}
			else if( 2 == entr_type ) { // ί�з�ʽ���м�
				req.LimitPrice = 0.0; // ί�м۸� double
				req.OrderPriceType = THOST_FTDC_OPT_AnyPrice; // �����۸����� char // �м�
				req.TimeCondition = THOST_FTDC_TC_IOC; // ��Ч������ char // ������ɣ�������
			}
			req.IsSwapOrder = 0; // ��������־ int
		}
		else { // ��Ϻ�Լ // ֻ���޼�
			req.LimitPrice = price; // ί�м۸� double
			req.OrderPriceType = THOST_FTDC_OPT_LimitPrice; // �����۸����� char // �޼�
			req.TimeCondition = THOST_FTDC_TC_GFD; // ��Ч������ char // ������Ч
			req.IsSwapOrder = ( 7 == entr_type ? 1 : 0 ); // ��������־ int // entr_type��֣������8 ����������9 ��Ʒ����������������2 ����������7 ��������
		}
		// �������ͣ����롢����
		char c_direct[] = { THOST_FTDC_D_Buy, THOST_FTDC_D_Sell };
		if( exch_side >= 1 && exch_side <= 2 ) {
			req.Direction = c_direct[exch_side - 1]; // �������� char
		}
		// �� CTP ���棬SR505&SR509 �� SPC y1505&p1505 �����ĺ�Լ����ƽ�֣�CombOffsetFlag �ֶ�Ҳ�ǵ������ȵ���д��ֻ�ǻ���ʱ��Ҫָ��������־������ʱ��������ȫһ��
		// ��ƽ���򣺿��֡�ƽ�֡�ǿƽ��ƽ��ƽ��ǿ��������ǿƽ��������ƽ�����Ϊ��THOST_FTDC_OF_CloseToday��������Ϊ��THOST_FTDC_OF_Close
		char c_offset[] = { THOST_FTDC_OF_Open, THOST_FTDC_OF_Close, THOST_FTDC_OF_ForceClose, THOST_FTDC_OF_CloseToday, THOST_FTDC_OF_CloseYesterday, THOST_FTDC_OF_ForceOff, THOST_FTDC_OF_LocalForceClose };
		if( offset >= 1 && offset <= 7 ) {
			req.CombOffsetFlag[0] = c_offset[offset - 1]; // ��Ͽ�ƽ��־ char 5
		}
		// Ͷ���ױ���Ͷ�����������ױ�
		char c_hedge[] = { THOST_FTDC_HF_Speculation, THOST_FTDC_HF_Arbitrage, THOST_FTDC_HF_Hedge };
		if( hedge >= 1 && hedge <= 3 ) {
			req.CombHedgeFlag[0] = c_hedge[hedge - 1]; // ���Ͷ���ױ���־ char 5
		}
		req.VolumeCondition = THOST_FTDC_VC_AV; // �ɽ������� char // �κ�����
		req.MinVolume = 1; // ��С�ɽ��� int // 1
		req.ContingentCondition = THOST_FTDC_CC_Immediately; // �������� char // ����
		req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose; // ǿƽԭ�� char // ��ǿƽ
		req.IsAutoSuspend = 0; // �Զ������־ int // ��
		req.UserForceClose = 0; // �û�ǿƽ��־ int // ��
		// req.UserID; // �û����� char 16
		// req.StopPrice; // ֹ��� double
		// req.GTDDate; // GTD���� char 9
		// req.BusinessUnit; // ҵ��Ԫ char 21
		int32_t request_id = GetRequestID();
		req.RequestID = request_id; // ������ int // ��һ�°ɣ�ȷ�������ر��д��ڴ�ֵ
		m_request_map_lock.lock();
		m_map_request[request_id] = *request;
		m_request_map_lock.unlock();
		return m_user_api->ReqOrderInsert( &req, request_id );
	}
	catch( ... ) {
		return -4;
	}
}

// �û�����ʱ����� CTP ����У����ȷ���򲻻��յ� OnRspOrderInsert ��Ϣ������ֱ�� OnRtnOrder �ر���ֻ�б����� CTP �ܾ��Ż��յ� OnRspOrderInsert ��Ϣ
// �����������Ϊ���������û��ͻ��յ� OnErrRtnOrderInsert ��Ϣ
void Session::OnRspOrderInsert( CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast ) {
	std::string log_info;
	std::string result_data = "";
	Request* request = GetRequestByID( nRequestID );

	if( request != nullptr ) {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			Json::Value results_json;
			results_json["ret_func"] = request->m_req_json["function"].asInt();
			results_json["ret_task"] = request->m_req_json["task_id"].asInt();
			results_json["ret_last"] = bIsLast;
			//results_json["ret_data"] = "";

			if( ( pRspInfo && pRspInfo->ErrorID != 0 ) || !pInputOrder ) {
				if( pRspInfo ) {
					FormatLibrary::StandardLibrary::FormatTo( log_info, "�����ύʧ�ܣ�{0}", pRspInfo->ErrorMsg );
				}
				else {
					log_info = "�����ύʧ�ܣ�ԭ��δ֪��";
				}
				m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );

				results_json["ret_code"] = -1;
				results_json["ret_info"] = basicx::StringToUTF8( log_info );
				results_json["ret_numb"] = 0;
			}
			else {
				FormatLibrary::StandardLibrary::FormatTo( log_info, "�����ύ�ɹ���{0}", pInputOrder->OrderRef );
				m_trader_ctp_p->LogPrint( basicx::syslog_level::c_info, log_info );

				results_json["ret_code"] = 0;
				results_json["ret_info"] = basicx::StringToUTF8( log_info );
				results_json["ret_numb"] = 1;

				Json::Value ret_data_json;
				ret_data_json["otc_code"] = 0;
				ret_data_json["otc_info"] = basicx::StringToUTF8( log_info );
				ret_data_json["order_id"] = pInputOrder->OrderRef; // �������� char 13
				// pInputOrder->BrokerID; // ���͹�˾���� char 11
				// pInputOrder->InvestorID; // Ͷ���ߴ��� char 13
				// pInputOrder->InstrumentID; // ��Լ���� char 31
				// pInputOrder->UserID; // �û����� char 16
				// pInputOrder->OrderPriceType; // �����۸����� char
				// pInputOrder->Direction; // �������� char
				// pInputOrder->CombOffsetFlag; // ��Ͽ�ƽ��־ char 5
				// pInputOrder->CombHedgeFlag; // ���Ͷ���ױ���־ char 5
				// pInputOrder->LimitPrice; // �۸� double
				// pInputOrder->VolumeTotalOriginal; // ���� int
				// pInputOrder->TimeCondition; // ��Ч������ char
				// pInputOrder->GTDDate; // GTD���� char 9
				// pInputOrder->VolumeCondition; // �ɽ������� char
				// pInputOrder->MinVolume; // ��С�ɽ��� int
				// pInputOrder->ContingentCondition; // �������� char
				// pInputOrder->StopPrice; // ֹ��� double
				// pInputOrder->ForceCloseReason; // ǿƽԭ�� char
				// pInputOrder->IsAutoSuspend; // �Զ������־ int
				// pInputOrder->BusinessUnit; // ҵ��Ԫ char 21
				// pInputOrder->RequestID; // ������ int
				// pInputOrder->UserForceClose; // �û�ǿ����־ int
				// pInputOrder->IsSwapOrder; // ��������־ int
				results_json["ret_data"].append( ret_data_json );
			}

			result_data = Json::writeString( m_json_writer, results_json );
		}

		m_trader_ctp_p->CommitResult( request->m_task_id, request->m_identity, request->m_code, result_data );
	}
	else {
		FormatLibrary::StandardLibrary::FormatTo( log_info, "�����ص�����ʧ�ܣ�{0}", nRequestID );
		m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );
	}
}

void Session::OnErrRtnOrderInsert( CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo ) { // ����������������У�����Ӧ�ü��ٷ���
	std::string log_info;

	if( ( pRspInfo && pRspInfo->ErrorID != 0 ) || !pInputOrder ) {
		if( pRspInfo ) {
			FormatLibrary::StandardLibrary::FormatTo( log_info, "�����ύ������ʧ�ܣ�{0}", pRspInfo->ErrorMsg );
		}
		else {
			log_info = "�����ύ������ʧ�ܣ�ԭ��δ֪��";
		}
		m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );
	}
	else {
		std::string result_data = "";
		Request* request = GetRequestByID( pInputOrder->RequestID );

		if( request != nullptr ) {
			if( NW_MSG_CODE_JSON == request->m_code ) {
				Json::Value results_json;
				results_json["ret_func"] = request->m_req_json["function"].asInt();
				results_json["ret_task"] = request->m_req_json["task_id"].asInt();
				results_json["ret_last"] = true;
				results_json["ret_code"] = -1;
				results_json["ret_info"] = basicx::StringToUTF8( "�����ύ������ʧ�ܣ�" );
				results_json["ret_numb"] = 0;
				//results_json["ret_data"] = "";

				if( pRspInfo && pRspInfo->ErrorID != 0 ) {
					if( pRspInfo ) {
						FormatLibrary::StandardLibrary::FormatTo( log_info, "�����ύ������ʧ�ܣ�{0}", pRspInfo->ErrorMsg );
					}
					else {
						log_info = "�����ύ������ʧ�ܣ�ԭ��δ֪��";
					}
					m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );

					results_json["ret_info"] = basicx::StringToUTF8( log_info );
				}

				result_data = Json::writeString( m_json_writer, results_json );
			}

			m_trader_ctp_p->CommitResult( request->m_task_id, request->m_identity, request->m_code, result_data );
		}
		else {
			FormatLibrary::StandardLibrary::FormatTo( log_info, "��������������ص�����ʧ�ܣ�{0}", pInputOrder->RequestID );
			m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );
		}
	}
}

// �µ��Ժ��� CTP ��ί�б�����������ǰ������ͨ�� FrontID��SessionID��OrderRef ���ߴ� CTP ����
// �� CTP ��ί�б���������֮����Ҫ���� ExchangeID��OrderSysID ������Ϣ����������ᱨ�Ҳ���Ҫ����ί�еĴ���
// ���Ҫ��� CTP ��������Ϊ�ͻ�����δ�յ������ر����� OrderRef ��Ϣ������Ҫ������������µ�ʱ�� task_id ���µ�ʱ�� RequestID ��Ϣ������˾ݴ��ҳ�����Ķ�Ӧ OrderRef ȥ����
// ����ί�б���������ǰ�ͳ���������������µ��󱨵��ر�����ǰ�ͳ��������Ҳ���٣���Ŀǰ��ʵ�����ֳ������ܣ���Ҫʱ�ټӰ�
int32_t Session::ReqOrderAction( Request* request ) {
	try {
		std::string order_ref = "";

		if( NW_MSG_CODE_JSON == request->m_code ) {
			order_ref = request->m_req_json["order_id"].asCString();
		}

		CThostFtdcOrderField* order_item = GetOrderItemByID( atoi( order_ref.c_str() ) );

		CThostFtdcInputOrderActionField req;
		memset( &req, 0, sizeof( req ) );
		strcpy_s( req.BrokerID, const_cast<char*>(m_broker_id.c_str()) ); // ���͹�˾���� char 11
		strcpy_s( req.InvestorID, const_cast<char*>(m_username.c_str()) ); // Ͷ���ߴ��� char 13
		req.OrderActionRef = m_trader_ctp_p->GetOrderRef(); // ������������ int // �������� CTP �Զ�����
		strcpy_s( req.OrderRef, const_cast<char*>( order_ref.c_str()) ); // �������� char 13
		req.FrontID = m_front_id; // ǰ�ñ�� int
		req.SessionID = m_session_id; // �Ự��� int
		if( order_item != nullptr ) { // ��� order_item == nullptr ���� CTP ���Ҳ����Ĵ���
			strcpy_s( req.ExchangeID, order_item->ExchangeID ); // ���������� char 9
			strcpy_s( req.OrderSysID, order_item->OrderSysID ); // ������� char 21
		}
		req.ActionFlag = THOST_FTDC_AF_Delete; // ������־ char
		// req.LimitPrice; // �۸� double
		// req.VolumeChange; // �����仯 int
		// req.UserID; // �û����� char 16
		// req.InstrumentID; // ��Լ���� char 31
		int32_t request_id = GetRequestID();
		req.RequestID = request_id; // ������ int // ��һ�°ɣ�ȷ�������ر��д��ڴ�ֵ
		m_request_map_lock.lock();
		m_map_request[request_id] = *request;
		m_request_map_lock.unlock();
		return m_user_api->ReqOrderAction( &req, request_id );
	}
	catch( ... ) {
		return -4;
	}
}

// �û�����ʱ����� CTP ����У����ȷ���򲻻��յ� OnRspOrderAction ��Ϣ������ֱ�� OnRtnOrder �ر���ֻ�г����� CTP �ܾ��Ż��յ� OnRspOrderAction ��Ϣ
// �����������Ϊ���������û��ͻ��յ� OnErrRtnOrderAction ��Ϣ
void Session::OnRspOrderAction( CThostFtdcInputOrderActionField* pInputOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast ) {
	std::string log_info;
	std::string result_data = "";
	Request* request = GetRequestByID( nRequestID );

	if( request != nullptr ) {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			Json::Value results_json;
			results_json["ret_func"] = request->m_req_json["function"].asInt();
			results_json["ret_task"] = request->m_req_json["task_id"].asInt();
			results_json["ret_last"] = bIsLast;
			//results_json["ret_data"] = "";

			if( ( pRspInfo && pRspInfo->ErrorID != 0 ) || !pInputOrderAction ) {
				if( pRspInfo ) {
					FormatLibrary::StandardLibrary::FormatTo( log_info, "�����ύʧ�ܣ�{0}", pRspInfo->ErrorMsg );
				}
				else {
					log_info = "�����ύʧ�ܣ�ԭ��δ֪��";
				}
				m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );

				results_json["ret_code"] = -1;
				results_json["ret_info"] = basicx::StringToUTF8( log_info );
				results_json["ret_numb"] = 0;
			}
			else {
				FormatLibrary::StandardLibrary::FormatTo( log_info, "�����ύ�ɹ���{0}", pInputOrderAction->OrderActionRef );
				m_trader_ctp_p->LogPrint( basicx::syslog_level::c_info, log_info );

				results_json["ret_code"] = 0;
				results_json["ret_info"] = basicx::StringToUTF8( log_info );
				results_json["ret_numb"] = 1;

				Json::Value ret_data_json;
				ret_data_json["otc_code"] = 0;
				ret_data_json["otc_info"] = basicx::StringToUTF8( log_info );
				std::string order_action_ref;
				FormatLibrary::StandardLibrary::FormatTo( order_action_ref, "{0}", pInputOrderAction->OrderActionRef ); // ������������ int
				ret_data_json["order_id"] = order_action_ref; // ���ַ�����ʽ��
				// pInputOrderAction->BrokerID; // ���͹�˾���� char 11
				// pInputOrderAction->InvestorID; // Ͷ���ߴ��� char 13
				// pInputOrderAction->OrderRef; // �������� char 13
				// pInputOrderAction->RequestID; // ������ int
				// pInputOrderAction->FrontID; // ǰ�ñ�� int
				// pInputOrderAction->SessionID; // �Ự��� int
				// pInputOrderAction->ExchangeID; // ���������� char 9
				// pInputOrderAction->OrderSysID; // ������� char 21
				// pInputOrderAction->ActionFlag; // ������־ char
				// pInputOrderAction->LimitPrice; // �۸� double
				// pInputOrderAction->VolumeChange; // �����仯 int
				// pInputOrderAction->UserID; // �û����� char 16
				// pInputOrderAction->InstrumentID; // ��Լ���� char 31
				results_json["ret_data"].append( ret_data_json );
			}

			result_data = Json::writeString( m_json_writer, results_json );
		}

		m_trader_ctp_p->CommitResult( request->m_task_id, request->m_identity, request->m_code, result_data );
	}
	else {
		FormatLibrary::StandardLibrary::FormatTo( log_info, "�����ص�����ʧ�ܣ�{0}", nRequestID );
		m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );
	}
}

void Session::OnErrRtnOrderAction( CThostFtdcOrderActionField* pOrderAction, CThostFtdcRspInfoField* pRspInfo ) { // ����������������У�����Ӧ�ü��ٷ���
	std::string log_info;

	if( ( pRspInfo && pRspInfo->ErrorID != 0 ) || !pOrderAction ) {
		if( pRspInfo ) {
			FormatLibrary::StandardLibrary::FormatTo( log_info, "�����ύ������ʧ�ܣ�{0}", pRspInfo->ErrorMsg );
		}
		else {
			log_info = "�����ύ������ʧ�ܣ�ԭ��δ֪��";
		}
		m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );
	}
	else {
		std::string result_data = "";
		Request* request = GetRequestByID( pOrderAction->RequestID );

		if( request != nullptr ) {
			if( NW_MSG_CODE_JSON == request->m_code ) {
				Json::Value results_json;
				results_json["ret_func"] = request->m_req_json["function"].asInt();
				results_json["ret_task"] = request->m_req_json["task_id"].asInt();
				results_json["ret_last"] = true;
				results_json["ret_code"] = -1;
				results_json["ret_info"] = basicx::StringToUTF8( "�����ύ������ʧ�ܣ�" );
				results_json["ret_numb"] = 0;
				//results_json["ret_data"] = "";

				if( pRspInfo && pRspInfo->ErrorID != 0 ) {
					if( pRspInfo ) {
						FormatLibrary::StandardLibrary::FormatTo( log_info, "�����ύ������ʧ�ܣ�{0}", pRspInfo->ErrorMsg );
					}
					else {
						log_info = "�����ύ������ʧ�ܣ�ԭ��δ֪��";
					}
					m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );

					results_json["ret_info"] = basicx::StringToUTF8( log_info );
				}

				result_data = Json::writeString( m_json_writer, results_json );
			}

			m_trader_ctp_p->CommitResult( request->m_task_id, request->m_identity, request->m_code, result_data );
		}
		else {
			FormatLibrary::StandardLibrary::FormatTo( log_info, "��������������ص�����ʧ�ܣ�{0}", pOrderAction->RequestID );
			m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );
		}
	}
}

int32_t Session::ReqQryTradingAccount( Request* request ) {
	try {
		CThostFtdcQryTradingAccountField req;
		memset( &req, 0, sizeof( req ) );
		strcpy_s( req.BrokerID, const_cast<char*>(m_broker_id.c_str()) ); // ���͹�˾���� char 11
		strcpy_s( req.InvestorID, const_cast<char*>(m_username.c_str()) ); // Ͷ���ߴ��� char 13
		int32_t request_id = GetRequestID();
		m_request_map_lock.lock();
		m_map_request[request_id] = *request;
		m_request_map_lock.unlock();
		return m_user_api->ReqQryTradingAccount( &req, request_id );
	}
	catch( ... ) {
		return -4;
	}
}

void Session::OnRspQryTradingAccount( CThostFtdcTradingAccountField* pTradingAccount, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast ) {
	std::string log_info;
	std::string result_data = "";
	Request* request = GetRequestByID( nRequestID );

	if( request != nullptr ) {
		bool no_error_and_is_last = false; // ����Ƿ���Ҫ������־��Ϣ

		if( NW_MSG_CODE_JSON == request->m_code ) {
			Json::Value results_json;
			results_json["ret_func"] = request->m_req_json["function"].asInt();
			results_json["ret_task"] = request->m_req_json["task_id"].asInt();
			//results_json["ret_data"] = "";

			if( pRspInfo && pRspInfo->ErrorID != 0 ) {
				FormatLibrary::StandardLibrary::FormatTo( log_info, "�ͻ��ʽ��ѯ�ύʧ�ܣ�{0}", pRspInfo->ErrorMsg );
				m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );

				results_json["ret_last"] = true;
				results_json["ret_code"] = -1;
				results_json["ret_info"] = basicx::StringToUTF8( log_info );
				results_json["ret_numb"] = 0;
			}
			else if( !pTradingAccount ) { // ��ѯ�޽������
				log_info = "�ͻ��ʽ��ѯ�ύ�ɹ����޽�����ݡ�";
				m_trader_ctp_p->LogPrint( basicx::syslog_level::c_info, log_info );

				results_json["ret_last"] = true;
				results_json["ret_code"] = 0;
				results_json["ret_info"] = basicx::StringToUTF8( "�ͻ��ʽ��ѯӦ�������ǡ�" );
				results_json["ret_numb"] = 0;
			}
			else {
				if( true == bIsLast ) { // ��Ҫ����������־��Ϣ
					no_error_and_is_last = true;
				}

				results_json["ret_last"] = false;
				results_json["ret_code"] = 0;
				results_json["ret_info"] = basicx::StringToUTF8( log_info );
				results_json["ret_numb"] = 1;

				Json::Value ret_data_json;
				ret_data_json["otc_code"] = 0;
				ret_data_json["otc_info"] = basicx::StringToUTF8( log_info );
				ret_data_json["account"] = pTradingAccount->AccountID; // Ͷ�����ʺ� char 13
				ret_data_json["currency"] = "RMB"; // ����
				ret_data_json["available"] = pTradingAccount->Available; // �����ʽ� double
				ret_data_json["profit"] = pTradingAccount->CloseProfit; // ƽ��ӯ�� double
				ret_data_json["float_profit"] = pTradingAccount->PositionProfit; // �ֲ�ӯ�� double
				ret_data_json["margin"] = pTradingAccount->CurrMargin; // ��ǰ��֤���ܶ� double
				ret_data_json["frozen_margin"] = pTradingAccount->FrozenMargin; // ����ı�֤�� double
				ret_data_json["fee"] = pTradingAccount->Commission; // ������ double
				ret_data_json["frozen_fee"] = pTradingAccount->FrozenCommission; // ����������� double
				// pTradingAccount->BrokerID; // ���͹�˾���� char 11
				// pTradingAccount->PreMortgage; // �ϴ���Ѻ��� double
				// pTradingAccount->PreCredit; // �ϴ����ö�� double
				// pTradingAccount->PreDeposit; // �ϴδ��� double
				// pTradingAccount->PreBalance; // �ϴν���׼���� double
				// pTradingAccount->PreMargin; // �ϴ�ռ�õı�֤�� double
				// pTradingAccount->InterestBase; // ��Ϣ���� double
				// pTradingAccount->Interest; // ��Ϣ���� double
				// pTradingAccount->Deposit; // ����� double
				// pTradingAccount->Withdraw; // ������ double
				// pTradingAccount->FrozenCash; // ������ʽ� double
				// pTradingAccount->CashIn; // �ʽ��� double
				// pTradingAccount->Balance; // �ڻ�����׼���� double
				// pTradingAccount->WithdrawQuota; // ��ȡ�ʽ� double
				// pTradingAccount->Reserve; // ����׼���� double
				// pTradingAccount->TradingDay; // ������ char 9
				// pTradingAccount->SettlementID; // ������ int
				// pTradingAccount->Credit; // ���ö�� double
				// pTradingAccount->Mortgage; // ��Ѻ��� double
				// pTradingAccount->ExchangeMargin; // ��������֤�� double
				// pTradingAccount->DeliveryMargin; // Ͷ���߽��֤�� double
				// pTradingAccount->ExchangeDeliveryMargin; // ���������֤�� double
				results_json["ret_data"].append( ret_data_json );
			}

			result_data = Json::writeString( m_json_writer, results_json );
		}

		m_trader_ctp_p->CommitResult( request->m_task_id, request->m_identity, request->m_code, result_data );

		if( true == no_error_and_is_last ) { // ����������־��Ϣ
			FormatLibrary::StandardLibrary::FormatTo( log_info, "�ͻ��ʽ��ѯ�ύ�ɹ���{0}", pTradingAccount->AccountID );
			m_trader_ctp_p->LogPrint( basicx::syslog_level::c_info, log_info );

			if( NW_MSG_CODE_JSON == request->m_code ) {
				Json::Value results_json;
				results_json["ret_func"] = request->m_req_json["function"].asInt();
				results_json["ret_task"] = request->m_req_json["task_id"].asInt();
				results_json["ret_last"] = true;
				results_json["ret_code"] = 0;
				results_json["ret_info"] = basicx::StringToUTF8( "�ͻ��ʽ��ѯӦ�������ǡ�" );
				results_json["ret_numb"] = 0;
				//results_json["ret_data"] = "";
				result_data = Json::writeString( m_json_writer, results_json );
			}

			m_trader_ctp_p->CommitResult( request->m_task_id, request->m_identity, request->m_code, result_data );
		}
	}
	else {
		FormatLibrary::StandardLibrary::FormatTo( log_info, "�ͻ��ʽ��ѯ�ص�����ʧ�ܣ�{0}", nRequestID );
		m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );
	}
}

int32_t Session::ReqQryInvestorPosition( Request* request ) {
	try {
		std::string instrument = "";

		if( NW_MSG_CODE_JSON == request->m_code ) {
			instrument = request->m_req_json["instrument"].asCString();
		}

		CThostFtdcQryInvestorPositionField req;
		memset( &req, 0, sizeof( req ) );
		strcpy_s( req.BrokerID, const_cast<char*>(m_broker_id.c_str()) ); // ���͹�˾���� char 11
		strcpy_s( req.InvestorID, const_cast<char*>(m_username.c_str()) ); // Ͷ���ߴ��� char 13
		strcpy_s( req.InstrumentID, const_cast<char*>(instrument.c_str()) ); // ��Լ���� char 31
		int32_t request_id = GetRequestID();
		m_request_map_lock.lock();
		m_map_request[request_id] = *request;
		m_request_map_lock.unlock();
		return m_user_api->ReqQryInvestorPosition( &req, request_id );
	}
	catch( ... ) {
		return -4;
	}
}

void Session::OnRspQryInvestorPosition( CThostFtdcInvestorPositionField* pInvestorPosition, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast ) {
	std::string log_info;
	std::string result_data = "";
	Request* request = GetRequestByID( nRequestID );

	if( request != nullptr ) {
		bool no_error_and_is_last = false; // ����Ƿ���Ҫ������־��Ϣ

		if( NW_MSG_CODE_JSON == request->m_code ) {
			Json::Value results_json;
			results_json["ret_func"] = request->m_req_json["function"].asInt();
			results_json["ret_task"] = request->m_req_json["task_id"].asInt();
			//results_json["ret_data"] = "";

			if( pRspInfo && pRspInfo->ErrorID != 0 ) {
				FormatLibrary::StandardLibrary::FormatTo( log_info, "�ͻ��ֲֲ�ѯ�ύʧ�ܣ�{0}", pRspInfo->ErrorMsg );
				m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );

				results_json["ret_last"] = true;
				results_json["ret_code"] = -1;
				results_json["ret_info"] = basicx::StringToUTF8( log_info );
				results_json["ret_numb"] = 0;
			}
			else if( !pInvestorPosition ) { // ��ѯ�޽������
				log_info = "�ͻ��ֲֲ�ѯ�ύ�ɹ����޽�����ݡ�";
				m_trader_ctp_p->LogPrint( basicx::syslog_level::c_info, log_info );

				results_json["ret_last"] = true;
				results_json["ret_code"] = 0;
				results_json["ret_info"] = basicx::StringToUTF8( "�ͻ��ֲֲ�ѯӦ�������ǡ�" );
				results_json["ret_numb"] = 0;
			}
			else {
				if( true == bIsLast ) { // ��Ҫ����������־��Ϣ
					no_error_and_is_last = true;
				}

				results_json["ret_last"] = false;
				results_json["ret_code"] = 0;
				results_json["ret_info"] = basicx::StringToUTF8( log_info );
				results_json["ret_numb"] = 1;

				Json::Value ret_data_json;
				ret_data_json["otc_code"] = 0;
				ret_data_json["otc_info"] = basicx::StringToUTF8( log_info );
				ret_data_json["instrument"] = pInvestorPosition->InstrumentID; // ��Լ���� char 31
				std::string direction( 1, pInvestorPosition->PosiDirection );
				ret_data_json["exch_side"] = atoi( direction.c_str() ) - 1; // �ֲֶ�շ��� char // Ϊ TSgitFtdcPosiDirectionType ���ͣ�'2' �࣬'3' �գ���Ϊ - 1
				ret_data_json["position"] = pInvestorPosition->Position; // �ֲܳ� int
				ret_data_json["tod_position"] = pInvestorPosition->TodayPosition; // ���ճֲ� int
				ret_data_json["pre_position"] = pInvestorPosition->YdPosition; // ���ճֲ� int
				ret_data_json["open_volume"] = pInvestorPosition->OpenVolume; // ������ int
				ret_data_json["close_volume"] = pInvestorPosition->CloseVolume; // ƽ���� int
				// pInvestorPosition->BrokerID; // ���͹�˾���� char 11
				// pInvestorPosition->InvestorID; // Ͷ���ߴ��� char 13
				// pInvestorPosition->HedgeFlag; // Ͷ���ױ���־ char
				// pInvestorPosition->PositionDate; // �ֲ����� char
				// pInvestorPosition->LongFrozen; // ��ͷ���� int
				// pInvestorPosition->ShortFrozen; // ��ͷ���� int
				// pInvestorPosition->LongFrozenAmount; // ��ͷ������ double
				// pInvestorPosition->ShortFrozenAmount; // ��ͷ������ double
				// pInvestorPosition->OpenAmount; // ���ֽ�� double
				// pInvestorPosition->CloseAmount; // ƽ�ֽ�� double
				// pInvestorPosition->PositionCost; // �ֲֳɱ� double
				// pInvestorPosition->PreMargin; // �ϴ�ռ�õı�֤�� double
				// pInvestorPosition->UseMargin; // ռ�õı�֤�� double
				// pInvestorPosition->FrozenMargin; // ����ı�֤�� double
				// pInvestorPosition->FrozenCash; // ������ʽ� double
				// pInvestorPosition->FrozenCommission; // ����������� double
				// pInvestorPosition->CashIn; // �ʽ��� double
				// pInvestorPosition->Commission; // ������ double
				// pInvestorPosition->CloseProfit; // ƽ��ӯ�� double
				// pInvestorPosition->PositionProfit; // �ֲ�ӯ�� double
				// pInvestorPosition->PreSettlementPrice; // �ϴν���� double
				// pInvestorPosition->SettlementPrice; // ���ν���� double
				// pInvestorPosition->TradingDay; // ������ char 9
				// pInvestorPosition->SettlementID; // ������ int
				// pInvestorPosition->OpenCost; // ���ֳɱ� double
				// pInvestorPosition->ExchangeMargin; // ��������֤�� double
				// pInvestorPosition->CombPosition; // ��ϳɽ��γɵĳֲ� int
				// pInvestorPosition->CombLongFrozen; // ��϶�ͷ���� int
				// pInvestorPosition->CombShortFrozen; // ��Ͽ�ͷ���� int
				// pInvestorPosition->CloseProfitByDate; // ���ն���ƽ��ӯ�� double
				// pInvestorPosition->CloseProfitByTrade; // ��ʶԳ�ƽ��ӯ�� double
				// pInvestorPosition->MarginRateByMoney; // ��֤���� double
				// pInvestorPosition->MarginRateByVolume; // ��֤����(������) double
				results_json["ret_data"].append( ret_data_json );
			}

			result_data = Json::writeString( m_json_writer, results_json );
		}

		m_trader_ctp_p->CommitResult( request->m_task_id, request->m_identity, request->m_code, result_data );

		if( true == no_error_and_is_last ) { // ����������־��Ϣ
			FormatLibrary::StandardLibrary::FormatTo( log_info, "�ͻ��ֲֲ�ѯ�ύ�ɹ���{0}", pInvestorPosition->InstrumentID );
			m_trader_ctp_p->LogPrint( basicx::syslog_level::c_info, log_info );

			if( NW_MSG_CODE_JSON == request->m_code ) {
				Json::Value results_json;
				results_json["ret_func"] = request->m_req_json["function"].asInt();
				results_json["ret_task"] = request->m_req_json["task_id"].asInt();
				results_json["ret_last"] = true;
				results_json["ret_code"] = 0;
				results_json["ret_info"] = basicx::StringToUTF8( "�ͻ��ֲֲ�ѯӦ�������ǡ�" );
				results_json["ret_numb"] = 0;
				//results_json["ret_data"] = "";
				result_data = Json::writeString( m_json_writer, results_json );
			}

			m_trader_ctp_p->CommitResult( request->m_task_id, request->m_identity, request->m_code, result_data );
		}
	}
	else {
		FormatLibrary::StandardLibrary::FormatTo( log_info, "�ͻ��ֲֲ�ѯ�ص�����ʧ�ܣ�{0}", nRequestID );
		m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );
	}
}

// �û�δָ��ί�кţ����� ExchangeID��OrderSysID ��Ϣ����ѯ���У���������
// �û�ָ��ί�кţ����Դӻ�����ȡ�ø�ί�кŶ�Ӧ�� ExchangeID��OrderSysID �Ӷ���ѯ�õ���Ӧί����Ϣ
// ���������û�и�ί�кţ�����ϵͳ���׻�ϵͳ�������������� ExchangeID��OrderSysID ��Ϣ����ѯ���У����� OnRspQryOrder ��ͨ��ί�кŹ���
// ���Կ������ָ���ڻ���Լ��
int32_t Session::ReqQryOrder( Request* request ) {
	try {
		std::string order_ref = "";

		if( NW_MSG_CODE_JSON == request->m_code ) {
			order_ref = request->m_req_json["order_id"].asCString();
		}

		CThostFtdcOrderField* order_item = nullptr;
		if( order_ref != "" ) { // �û�ָ��ί�к�
			order_item = GetOrderItemByID( atoi( order_ref.c_str() ) );
		}

		CThostFtdcQryOrderField req;
		memset( &req, 0, sizeof( req ) );
		strcpy_s( req.BrokerID, const_cast<char*>(m_broker_id.c_str()) ); // ���͹�˾���� char 11
		strcpy_s( req.InvestorID, const_cast<char*>(m_username.c_str()) ); // Ͷ���ߴ��� char 13
		//strcpy_s( req.InstrumentID, const_cast<char*>(instrument.c_str()) ); // ��Լ���� char 31
		if( order_item != nullptr ) { // ��� order_item == nullptr ���ѯ����ί�У������ѯ����ί��
			std::string instrument = order_item->InstrumentID;
			strcpy_s( req.InstrumentID, const_cast<char*>(instrument.c_str()) ); // ��Լ���� char 31 // ���𵽹������ü��ٷ�������
			strcpy_s( req.ExchangeID, order_item->ExchangeID ); // ���������� char 9
			strcpy_s( req.OrderSysID, order_item->OrderSysID ); // ������� char 21
		}
		// req.InsertTimeStart; // ��ʼʱ�� char 9
		// req.InsertTimeEnd; // ����ʱ�� char 9
		int32_t request_id = GetRequestID();
		m_request_map_lock.lock();
		m_map_request[request_id] = *request;
		m_request_map_lock.unlock();
		return m_user_api->ReqQryOrder( &req, request_id );
	}
	catch( ... ) {
		return -4;
	}
}

void Session::OnRspQryOrder( CThostFtdcOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast ) {
	std::string log_info;
	std::string result_data = "";
	Request* request = GetRequestByID( nRequestID );

	if( request != nullptr ) {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			if( pRspInfo && pRspInfo->ErrorID != 0 ) {
				FormatLibrary::StandardLibrary::FormatTo( log_info, "�ͻ�����ί�в�ѯ�ύʧ�ܣ�{0}", pRspInfo->ErrorMsg );
				m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );

				Json::Value results_json;
				results_json["ret_func"] = request->m_req_json["function"].asInt();
				results_json["ret_task"] = request->m_req_json["task_id"].asInt();
				results_json["ret_last"] = true;
				results_json["ret_code"] = -1;
				results_json["ret_info"] = basicx::StringToUTF8( log_info );
				results_json["ret_numb"] = 0;
				//results_json["ret_data"] = "";

				result_data = Json::writeString( m_json_writer, results_json );
				m_trader_ctp_p->CommitResult( request->m_task_id, request->m_identity, request->m_code, result_data );
			}
			else if( !pOrder ) { // ��ѯ�޽������
				log_info = "�ͻ�����ί�в�ѯ�ύ�ɹ����޽�����ݡ�";
				m_trader_ctp_p->LogPrint( basicx::syslog_level::c_info, log_info );

				Json::Value results_json;
				results_json["ret_func"] = request->m_req_json["function"].asInt();
				results_json["ret_task"] = request->m_req_json["task_id"].asInt();
				results_json["ret_last"] = true;
				results_json["ret_code"] = 0;
				results_json["ret_info"] = basicx::StringToUTF8( "�ͻ�����ί�в�ѯӦ�������ǡ�" );
				results_json["ret_numb"] = 0;
				//results_json["ret_data"] = "";

				result_data = Json::writeString( m_json_writer, results_json );
				m_trader_ctp_p->CommitResult( request->m_task_id, request->m_identity, request->m_code, result_data );
			}
			else {
				std::string order_ref_req = request->m_req_json["order_id"].asCString();
				std::string order_ref_rsp = pOrder->OrderRef;
				// ������ Q7 �ȿͻ����� OrderRef ��ȫ�ַ�����˲��ո�ģ���Ȼ��ֵ������ȣ�������ͻᱻ���˵�
				if( "" == order_ref_req || order_ref_rsp == order_ref_req ) { // ���˳��û������
					Json::Value results_json;
					results_json["ret_func"] = request->m_req_json["function"].asInt();
					results_json["ret_task"] = request->m_req_json["task_id"].asInt();
					results_json["ret_last"] = false;
					results_json["ret_code"] = 0;
					results_json["ret_info"] = basicx::StringToUTF8( log_info );
					results_json["ret_numb"] = 1;
					//results_json["ret_data"] = "";

					Json::Value ret_data_json;
					ret_data_json["otc_code"] = 0;
					ret_data_json["otc_info"] = basicx::StringToUTF8( log_info );
					ret_data_json["order_id"] = pOrder->OrderRef; // �������� char 13
					ret_data_json["order_sys_id"] = pOrder->OrderSysID; // ������� char 21
					ret_data_json["instrument"] = pOrder->InstrumentID; // ��Լ���� char 31
					ret_data_json["exchange"] = pOrder->ExchangeID; // ���������� char 9
					std::string direction( 1, pOrder->Direction );
					ret_data_json["exch_side"] = atoi( direction.c_str() ) + 1; // �������� char
					ret_data_json["fill_qty"] = pOrder->VolumeTraded; // �ɽ����� int
					// ������ȡ n_status �Ļ� "status" �ֶ�һ������
					if( pOrder->OrderStatus != 'a' && pOrder->OrderStatus != 'b' && pOrder->OrderStatus != 'c' ) {
						// 0��ȫ���ɽ���1�����ֳɽ���2�����ɲ�����3���ѱ�δ�ɣ�4���Զ�����5��ȫ������
						int32_t status_temp[] = { 5, 4, 7, 3, 13, 8 };
						std::string s_status( 1, pOrder->OrderStatus ); // ����״̬ char
						int32_t n_status = atoi( s_status.c_str() );
						if( n_status >= 0 && n_status <= 5 ) {
							ret_data_json["status"] = status_temp[n_status];
						}
					}
					else if( 'a' == pOrder->OrderStatus ) { // δ֪
						ret_data_json["status"] = 14;
					}
					else if( 'b' == pOrder->OrderStatus ) { // ��δ����
						ret_data_json["status"] = 11;
					}
					else if( 'c' == pOrder->OrderStatus ) { // �Ѿ�����
						ret_data_json["status"] = 12;
					}
					ret_data_json["status_msg"] = basicx::StringToUTF8( pOrder->StatusMsg ); // ״̬��Ϣ char 81
					// pOrder->RequestID; // ������ int
					// pOrder->BrokerID; // ���͹�˾���� char 11
					// pOrder->InvestorID; // Ͷ���ߴ��� char 13
					// pOrder->UserID; // �û����� char 16
					// pOrder->OrderPriceType; // �����۸����� char
					// pOrder->CombOffsetFlag; // ��Ͽ�ƽ��־ char 5
					// pOrder->CombHedgeFlag; // ���Ͷ���ױ���־ char 5
					// pOrder->LimitPrice; // �۸� double
					// pOrder->VolumeTotalOriginal; // ���� int
					// pOrder->TimeCondition; // ��Ч������ char
					// pOrder->GTDDate; // GTD���� char 9
					// pOrder->VolumeCondition; // �ɽ������� char
					// pOrder->MinVolume; // ��С�ɽ��� int
					// pOrder->ContingentCondition; // �������� char
					// pOrder->StopPrice; // ֹ��� double
					// pOrder->ForceCloseReason; // ǿƽԭ�� char
					// pOrder->IsAutoSuspend; // �Զ������־ int
					// pOrder->BusinessUnit; // ҵ��Ԫ char 21
					// pOrder->OrderLocalID; // ���ر������ char 13
					// pOrder->ParticipantID; // ��Ա���� char 11
					// pOrder->ClientID; // �ͻ����� char 11
					// pOrder->ExchangeInstID; // ��Լ�ڽ������Ĵ��� char 31
					// pOrder->TraderID; // ����������Ա���� char 21
					// pOrder->InstallID; // ��װ��� int
					// pOrder->OrderSubmitStatus; // �����ύ״̬ char
					// pOrder->NotifySequence; // ������ʾ��� int
					// pOrder->TradingDay; // ������ char 9
					// pOrder->SettlementID; // ������ int
					// pOrder->OrderSource; // ������Դ char
					// pOrder->OrderType; // �������� char
					// pOrder->VolumeTotal; // ʣ������ int
					// pOrder->InsertDate; // �������� char 9
					// pOrder->InsertTime; // ί��ʱ�� char 9
					// pOrder->ActiveTime; // ����ʱ�� char 9
					// pOrder->SuspendTime; // ����ʱ�� char 9
					// pOrder->UpdateTime; // ����޸�ʱ�� char 9
					// pOrder->CancelTime; // ����ʱ�� char 9
					// pOrder->ActiveTraderID; // ����޸Ľ���������Ա���� char 21
					// pOrder->ClearingPartID; // �����Ա��� char 11
					// pOrder->SequenceNo; // ��� int
					// pOrder->FrontID; // ǰ�ñ�� int
					// pOrder->SessionID; // �Ự��� int
					// pOrder->UserProductInfo; // �û��˲�Ʒ��Ϣ char 11
					// pOrder->UserForceClose; // �û�ǿ����־ int
					// pOrder->ActiveUserID; // �����û����� char 16
					// pOrder->BrokerOrderSeq; // ���͹�˾������� int
					// pOrder->RelativeOrderSysID; // ��ر��� char 21
					// pOrder->ZCETotalTradedVolume; // ֣�����ɽ����� int
					// pOrder->IsSwapOrder; // ��������־ int
					results_json["ret_data"].append( ret_data_json );

					result_data = Json::writeString( m_json_writer, results_json );
					m_trader_ctp_p->CommitResult( request->m_task_id, request->m_identity, request->m_code, result_data );
				}

				if( true == bIsLast ) { // ����������־��Ϣ
					log_info = "�ͻ�����ί�в�ѯ�ύ�ɹ���";
					m_trader_ctp_p->LogPrint( basicx::syslog_level::c_info, log_info );

					Json::Value results_json;
					results_json["ret_func"] = request->m_req_json["function"].asInt();
					results_json["ret_task"] = request->m_req_json["task_id"].asInt();
					results_json["ret_last"] = true;
					results_json["ret_code"] = 0;
					results_json["ret_info"] = basicx::StringToUTF8( "�ͻ�����ί�в�ѯӦ�������ǡ�" );
					results_json["ret_numb"] = 0;
					//results_json["ret_data"] = "";
					result_data = Json::writeString( m_json_writer, results_json );
					m_trader_ctp_p->CommitResult( request->m_task_id, request->m_identity, request->m_code, result_data );
				}
			}
		}
	}
	else {
		FormatLibrary::StandardLibrary::FormatTo( log_info, "�ͻ�����ί�в�ѯ�ص�����ʧ�ܣ�{0}", nRequestID );
		m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );
	}
}

// ��Ϊ CThostFtdcQryTradeField ���޷��� OrderRef �ֶΣ�ָ�� TradeID ���岻���������ӿڲ�ͳһ����ֻ��ȫ����ѯ�ٸ��� OrderRef ������
// ���Կ������ָ���ڻ���Լ��
int32_t Session::ReqQryTrade( Request* request ) {
	try {
		std::string order_ref = "";

		if( NW_MSG_CODE_JSON == request->m_code ) {
			order_ref = request->m_req_json["order_id"].asCString();
		}

		CThostFtdcOrderField* order_item = nullptr;
		if( order_ref != "" ) { // �û�ָ��ί�кţ�������ʵ��̫�����壬��Ϊ CThostFtdcQryTradeField ���޷��� OrderRef �ֶ�
			order_item = GetOrderItemByID( atoi( order_ref.c_str() ) );
		}

		CThostFtdcQryTradeField req;
		memset( &req, 0, sizeof( req ) );
		strcpy_s( req.BrokerID, const_cast<char*>(m_broker_id.c_str()) ); // ���͹�˾���� char 11
		strcpy_s( req.InvestorID, const_cast<char*>(m_username.c_str()) ); // Ͷ���ߴ��� char 13
		//strcpy_s( req.InstrumentID, const_cast<char*>(instrument.c_str()) ); // ��Լ���� char 31
		if( order_item != nullptr ) { // ��� order_item == nullptr ���ѯ����ί�У������ѯ����ί�У�����ˣ�
			std::string instrument = order_item->InstrumentID;
			strcpy_s( req.InstrumentID, const_cast<char*>(instrument.c_str()) ); // ��Լ���� char 31 // ���𵽹������ü��ٷ�������
			strcpy_s( req.ExchangeID, order_item->ExchangeID ); // ���������� char 9
			// req.TradeID; // �ɽ���� char 21 // ����Ͳ�����
		}
		// req.TradeTimeStart; // ��ʼʱ�� char 9
		// req.TradeTimeEnd; // ����ʱ�� char 9
		int32_t request_id = GetRequestID();
		m_request_map_lock.lock();
		m_map_request[request_id] = *request;
		m_request_map_lock.unlock();
		return m_user_api->ReqQryTrade( &req, request_id );
	}
	catch( ... ) {
		return -4;
	}
}

void Session::OnRspQryTrade( CThostFtdcTradeField* pTrade, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast ) {
	std::string log_info;
	std::string result_data = "";
	Request* request = GetRequestByID( nRequestID );

	if( request != nullptr ) {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			if( pRspInfo && pRspInfo->ErrorID != 0 ) {
				FormatLibrary::StandardLibrary::FormatTo( log_info, "�ͻ����ճɽ���ѯ�ύʧ�ܣ�{0}", pRspInfo->ErrorMsg );
				m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );

				Json::Value results_json;
				results_json["ret_func"] = request->m_req_json["function"].asInt();
				results_json["ret_task"] = request->m_req_json["task_id"].asInt();
				results_json["ret_last"] = true;
				results_json["ret_code"] = -1;
				results_json["ret_info"] = basicx::StringToUTF8( log_info );
				results_json["ret_numb"] = 0;
				//results_json["ret_data"] = "";

				result_data = Json::writeString( m_json_writer, results_json );
				m_trader_ctp_p->CommitResult( request->m_task_id, request->m_identity, request->m_code, result_data );
			}
			else if( !pTrade ) { // ��ѯ�޽������
				log_info = "�ͻ����ճɽ���ѯ�ύ�ɹ����޽�����ݡ�";
				m_trader_ctp_p->LogPrint( basicx::syslog_level::c_info, log_info );

				Json::Value results_json;
				results_json["ret_func"] = request->m_req_json["function"].asInt();
				results_json["ret_task"] = request->m_req_json["task_id"].asInt();
				results_json["ret_last"] = true;
				results_json["ret_code"] = 0;
				results_json["ret_info"] = basicx::StringToUTF8( "�ͻ����ճɽ���ѯӦ�������ǡ�" );
				results_json["ret_numb"] = 0;
				//results_json["ret_data"] = "";

				result_data = Json::writeString( m_json_writer, results_json );
				m_trader_ctp_p->CommitResult( request->m_task_id, request->m_identity, request->m_code, result_data );
			}
			else {
				std::string order_ref_req = request->m_req_json["order_id"].asCString();
				std::string order_ref_rsp = pTrade->OrderRef;
				// ������ Q7 �ȿͻ����� OrderRef ��ȫ�ַ�����˲��ո�ģ���Ȼ��ֵ������ȣ�������ͻᱻ���˵�
				if( "" == order_ref_req || order_ref_rsp == order_ref_req ) { // ���˳��û������
					Json::Value results_json;
					results_json["ret_func"] = request->m_req_json["function"].asInt();
					results_json["ret_task"] = request->m_req_json["task_id"].asInt();
					results_json["ret_last"] = false;
					results_json["ret_code"] = 0;
					results_json["ret_info"] = basicx::StringToUTF8( log_info );
					results_json["ret_numb"] = 1;
					//results_json["ret_data"] = "";

					Json::Value ret_data_json;
					ret_data_json["otc_code"] = 0;
					ret_data_json["otc_info"] = basicx::StringToUTF8( log_info );
					ret_data_json["order_id"] = pTrade->OrderRef; // �������� char 13
					ret_data_json["trans_id"] = pTrade->TradeID; // �ɽ���� char 21
					ret_data_json["instrument"] = pTrade->InstrumentID; // ��Լ���� char 31
					ret_data_json["exchange"] = pTrade->ExchangeID; // ���������� char 9
					std::string direction( 1, pTrade->Direction );
					ret_data_json["exch_side"] = atoi( direction.c_str() ) + 1; // �������� char
					ret_data_json["fill_qty"] = pTrade->Volume; // �ɽ����� int
					ret_data_json["fill_price"] = pTrade->Price; // �ɽ��۸� double
					ret_data_json["fill_time"] = pTrade->TradeTime; // �ɽ�ʱ�� char 9
					// pTrade->BrokerID; // ���͹�˾���� char 11
					// pTrade->InvestorID; // Ͷ���ߴ��� char 13
					// pTrade->UserID; // �û����� char 16
					// pTrade->OrderSysID; // ������� char 21
					// pTrade->ParticipantID; // ��Ա���� char 11
					// pTrade->ClientID; // �ͻ����� char 11
					// pTrade->TradingRole; // ���׽�ɫ char
					// pTrade->ExchangeInstID; // ��Լ�ڽ������Ĵ��� char 31
					// pTrade->OffsetFlag; // ��ƽ��־ char
					// pTrade->HedgeFlag; // Ͷ���ױ���־ char
					// pTrade->TradeDate; // �ɽ�ʱ�� char 9
					// pTrade->TradeType; // �ɽ����� char
					// pTrade->PriceSource; // �ɽ�����Դ char
					// pTrade->TraderID; // ����������Ա���� char 21
					// pTrade->OrderLocalID; // ���ر������ char 13
					// pTrade->ClearingPartID; // �����Ա��� char 11
					// pTrade->BusinessUnit; // ҵ��Ԫ char 21
					// pTrade->SequenceNo; // ��� int
					// pTrade->TradingDay; // ������ char 9
					// pTrade->SettlementID; // ������ int
					// pTrade->BrokerOrderSeq; // ���͹�˾������� int
					// pTrade->TradeSource; // �ɽ���Դ char
					results_json["ret_data"].append( ret_data_json );

					result_data = Json::writeString( m_json_writer, results_json );
					m_trader_ctp_p->CommitResult( request->m_task_id, request->m_identity, request->m_code, result_data );
				}

				if( true == bIsLast ) { // ����������־��Ϣ
					log_info = "�ͻ����ճɽ���ѯ�ύ�ɹ���";
					m_trader_ctp_p->LogPrint( basicx::syslog_level::c_info, log_info );

					Json::Value results_json;
					results_json["ret_func"] = request->m_req_json["function"].asInt();
					results_json["ret_task"] = request->m_req_json["task_id"].asInt();
					results_json["ret_last"] = true;
					results_json["ret_code"] = 0;
					results_json["ret_info"] = basicx::StringToUTF8( "�ͻ����ճɽ���ѯӦ�������ǡ�" );
					results_json["ret_numb"] = 0;
					//results_json["ret_data"] = "";
					result_data = Json::writeString( m_json_writer, results_json );
					m_trader_ctp_p->CommitResult( request->m_task_id, request->m_identity, request->m_code, result_data );
				}
			}
		}
	}
	else {
		FormatLibrary::StandardLibrary::FormatTo( log_info, "�ͻ����ճɽ���ѯ�ص�����ʧ�ܣ�{0}", nRequestID );
		m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );
	}
}

int32_t Session::ReqQryInstrument( Request* request ) {
	try {
		std::string instrument = "";

		if( NW_MSG_CODE_JSON == request->m_code ) {
			instrument = request->m_req_json["instrument"].asCString();
		}
		//request->m_req_json["category"].asCString();

		CThostFtdcQryInstrumentField req;
		memset( &req, 0, sizeof( req ) );
		strcpy_s( req.InstrumentID, const_cast<char*>(instrument.c_str()) ); // ��Լ���� char 31 // Ϊ�ձ�ʾ��ѯ���к�Լ
		// req.ExchangeID; // ���������� char 9
		// req.ExchangeInstID; // ��Լ�ڽ������Ĵ��� char 31
		// req.ProductID; // ��Ʒ���� char 31
		int32_t request_id = GetRequestID();
		m_request_map_lock.lock();
		m_map_request[request_id] = *request;
		m_request_map_lock.unlock();
		return m_user_api->ReqQryInstrument( &req, request_id );
	}
	catch( ... ) {
		return -4;
	}
}

void Session::OnRspQryInstrument( CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast ) {
	std::string log_info;
	std::string result_data = "";
	Request* request = GetRequestByID( nRequestID );

	if( request != nullptr ) {
		bool no_error_and_is_last = false; // ����Ƿ���Ҫ������־��Ϣ

		if( NW_MSG_CODE_JSON == request->m_code ) {
			Json::Value results_json;
			results_json["ret_func"] = request->m_req_json["function"].asInt();
			results_json["ret_task"] = request->m_req_json["task_id"].asInt();
			//results_json["ret_data"] = "";

			if( pRspInfo && pRspInfo->ErrorID != 0 ) {
				FormatLibrary::StandardLibrary::FormatTo( log_info, "�ڻ���Լ��ѯ�ύʧ�ܣ�{0}", pRspInfo->ErrorMsg );
				m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );

				results_json["ret_last"] = true;
				results_json["ret_code"] = -1;
				results_json["ret_info"] = basicx::StringToUTF8( log_info );
				results_json["ret_numb"] = 0;
			}
			else if( !pInstrument ) { // ��ѯ�޽������
				log_info = "�ڻ���Լ��ѯ�ύ�ɹ����޽�����ݡ�";
				m_trader_ctp_p->LogPrint( basicx::syslog_level::c_info, log_info );

				results_json["ret_last"] = true;
				results_json["ret_code"] = 0;
				results_json["ret_info"] = basicx::StringToUTF8( "�ڻ���Լ��ѯӦ�������ǡ�" );
				results_json["ret_numb"] = 0;
			}
			else {
				if( true == bIsLast ) { // ��Ҫ����������־��Ϣ
					no_error_and_is_last = true;
				}

				results_json["ret_last"] = false;
				results_json["ret_code"] = 0;
				results_json["ret_info"] = basicx::StringToUTF8( log_info );
				results_json["ret_numb"] = 1;

				Json::Value ret_data_json;
				ret_data_json["otc_code"] = 0;
				ret_data_json["otc_info"] = basicx::StringToUTF8( log_info );
				ret_data_json["instrument"] = pInstrument->InstrumentID; // ��Լ���� char 31
				ret_data_json["exchange"] = pInstrument->ExchangeID; // ���������� char 9
				ret_data_json["delivery_y"] = pInstrument->DeliveryYear; // ������� int
				ret_data_json["delivery_m"] = pInstrument->DeliveryMonth; // ������ int
				ret_data_json["long_margin"] = pInstrument->LongMarginRatio; // ��ͷ��֤���� double
				ret_data_json["short_margin"] = pInstrument->ShortMarginRatio; // ��ͷ��֤���� double
				// pInstrument->InstrumentName; // ��Լ���� char 21
				// pInstrument->ExchangeInstID; // ��Լ�ڽ������Ĵ��� char 31
				// pInstrument->ProductID; // ��Ʒ���� char 31
				// pInstrument->ProductClass; // ��Ʒ���� char
				// pInstrument->MaxMarketOrderVolume; // �м۵�����µ��� int
				// pInstrument->MinMarketOrderVolume; // �м۵���С�µ��� int
				// pInstrument->MaxLimitOrderVolume; // �޼۵�����µ��� int
				// pInstrument->MinLimitOrderVolume; // �޼۵���С�µ��� int
				// pInstrument->VolumeMultiple; // ��Լ�������� int
				// pInstrument->PriceTick; // ��С�䶯��λ double
				// pInstrument->CreateDate; // ������ char 9
				// pInstrument->OpenDate; // ������ char 9
				// pInstrument->ExpireDate; // ������ char 9
				// pInstrument->StartDelivDate; // ��ʼ������ char 9
				// pInstrument->EndDelivDate; // ���������� char 9
				// pInstrument->InstLifePhase; // ��Լ��������״̬ char
				// pInstrument->IsTrading; // ��ǰ�Ƿ��� int
				// pInstrument->PositionType; // �ֲ����� char
				// pInstrument->PositionDateType; // �ֲ��������� char
				results_json["ret_data"].append( ret_data_json );
			}

			result_data = Json::writeString( m_json_writer, results_json );
		}

		m_trader_ctp_p->CommitResult( request->m_task_id, request->m_identity, request->m_code, result_data );

		if( true == no_error_and_is_last ) { // ����������־��Ϣ
			FormatLibrary::StandardLibrary::FormatTo( log_info, "�ڻ���Լ��ѯ�ύ�ɹ���{0}", pInstrument->InstrumentID );
			m_trader_ctp_p->LogPrint( basicx::syslog_level::c_info, log_info );

			if( NW_MSG_CODE_JSON == request->m_code ) {
				Json::Value results_json;
				results_json["ret_func"] = request->m_req_json["function"].asInt();
				results_json["ret_task"] = request->m_req_json["task_id"].asInt();
				results_json["ret_last"] = true;
				results_json["ret_code"] = 0;
				results_json["ret_info"] = basicx::StringToUTF8( "�ڻ���Լ��ѯӦ�������ǡ�" );
				results_json["ret_numb"] = 0;
				//results_json["ret_data"] = "";
				result_data = Json::writeString( m_json_writer, results_json );
			}

			m_trader_ctp_p->CommitResult( request->m_task_id, request->m_identity, request->m_code, result_data );
		}
	}
	else {
		FormatLibrary::StandardLibrary::FormatTo( log_info, "�ڻ���Լ��ѯ�ص�����ʧ�ܣ�{0}", nRequestID );
		m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );
	}
}

// FrontID��SessionID��OrderRef����û�еõ�������Ӧǰ���û�����ʹ�����齻�����кŽ��г�������������Ψһ��ʶ�κ�һ��ί��
// BrokerID��BrokerOrderSeq���յ��û�������Ϊÿ�����͹�˾�ı������ɵĽ������к�
// exchangeID��traderID��OrderLocalID������ϯλ������������ʱ���������齻�����кţ���ʾÿһ�ʷ����������ı���
// exchangeID��OrderSysID��������������Ͷ���߱������������齻�����кţ���ʾÿһ���յ��ı���
// TThostFtdcOrderStatusType������״̬�����δ֪����ʾ CTP �Ѿ������û���ί��ָ���û��ת����������
// ȫ���ɽ� = ȫ���ɽ������ֳɽ����ڶ����� = ���ֳɽ������ֳɽ����ڶ����� = ���ɲ�����δ�ɽ����ڶ����� = �ѱ�δ�ɣ�δ�ɽ����ڶ����� = �Զ����𣻳��� = ȫ������

void Session::OnRtnOrder( CThostFtdcOrderField* pOrder ) {
	if( true == m_query_only ) { // �ӿڽ�����ѯʱ�ر�ί�кͳɽ��ر�����
		return;
	}

	std::string log_info;
	std::string result_risk = "";
	std::string result_data = "";
	std::string asset_account = "";

	// ί�г�����ί�в�ѯӦ�𡢳ɽ���ѯӦ�𡢳ɽ��ر�����Ҫ
	m_order_map_lock.lock();
	m_map_order[atoi( pOrder->OrderRef )] = *pOrder;
	m_order_map_lock.unlock();

	std::string direction( 1, pOrder->Direction );

	Request* request = GetRequestByID( pOrder->RequestID );
	if( request != nullptr ) {
		try {
			Json::Value results_json; // �ر�ͳһ�� NW_MSG_CODE_JSON ����
			results_json["ret_func"] = 290001;
			//results_json["task_id"] = pOrder->RequestID; // ������ int
			results_json["task_id"] = request->m_req_json["task_id"].asInt(); // ע�⣺����Ӧ��ԭΪ�û��� task_id ����ֱ���÷���˵�������
			results_json["order_id"] = pOrder->OrderRef; // �������� char 13
			results_json["order_sys_id"] = pOrder->OrderSysID; // ������� char 21
			results_json["instrument"] = pOrder->InstrumentID; // ��Լ���� char 31
			results_json["exchange"] = pOrder->ExchangeID; // ���������� char 9
			results_json["exch_side"] = atoi( direction.c_str() ) + 1; // �������� char
			results_json["fill_qty"] = pOrder->VolumeTraded; // �ɽ����� int
			// ������ȡ n_status �Ļ� "status" �ֶ�һ������
			if( pOrder->OrderStatus != 'a' && pOrder->OrderStatus != 'b' && pOrder->OrderStatus != 'c' ) {
				// 0��ȫ���ɽ���1�����ֳɽ���2�����ɲ�����3���ѱ�δ�ɣ�4���Զ�����5��ȫ������
				int32_t status_temp[] = { 5, 4, 7, 3, 13, 8 };
				std::string s_status( 1, pOrder->OrderStatus ); // ����״̬ char
				int32_t n_status = atoi( s_status.c_str() );
				if( n_status >= 0 && n_status <= 5 ) {
					results_json["status"] = status_temp[n_status];
				}
			}
			else if( 'a' == pOrder->OrderStatus ) { // δ֪
				results_json["status"] = 14;
			}
			else if( 'b' == pOrder->OrderStatus ) { // ��δ����
				results_json["status"] = 11;
			}
			else if( 'c' == pOrder->OrderStatus ) { // �Ѿ�����
				results_json["status"] = 12;
			}
			results_json["status_msg"] = basicx::StringToUTF8( pOrder->StatusMsg ); // ״̬��Ϣ char 81
			// pOrder->BrokerID; // ���͹�˾���� char 11
			// pOrder->InvestorID; // Ͷ���ߴ��� char 13
			// pOrder->UserID; // �û����� char 16
			// pOrder->OrderPriceType; // �����۸����� char
			// pOrder->CombOffsetFlag; // ��Ͽ�ƽ��־ char 5
			// pOrder->CombHedgeFlag; // ���Ͷ���ױ���־ char 5
			// pOrder->LimitPrice; // �۸� double
			// pOrder->VolumeTotalOriginal; // ���� int
			// pOrder->TimeCondition; // ��Ч������ char
			// pOrder->GTDDate; // GTD���� char 9
			// pOrder->VolumeCondition; // �ɽ������� char
			// pOrder->MinVolume; // ��С�ɽ��� int
			// pOrder->ContingentCondition; // �������� char
			// pOrder->StopPrice; // ֹ��� double
			// pOrder->ForceCloseReason; // ǿƽԭ�� char
			// pOrder->IsAutoSuspend; // �Զ������־ int
			// pOrder->BusinessUnit; // ҵ��Ԫ char 21
			// pOrder->OrderLocalID; // ���ر������ char 13
			// pOrder->ParticipantID; // ��Ա���� char 11
			// pOrder->ClientID; // �ͻ����� char 11
			// pOrder->ExchangeInstID; // ��Լ�ڽ������Ĵ��� char 31
			// pOrder->TraderID; // ����������Ա���� char 21
			// pOrder->InstallID; // ��װ��� int
			// pOrder->OrderSubmitStatus; // �����ύ״̬ char
			// pOrder->NotifySequence; // ������ʾ��� int
			// pOrder->TradingDay; // ������ char 9
			// pOrder->SettlementID; // ������ int
			// pOrder->OrderSource; // ������Դ char
			// pOrder->OrderType; // �������� char
			// pOrder->VolumeTotal; // ʣ������ int
			// pOrder->InsertDate; // �������� char 9
			// pOrder->InsertTime; // ί��ʱ�� char 9
			// pOrder->ActiveTime; // ����ʱ�� char 9
			// pOrder->SuspendTime; // ����ʱ�� char 9
			// pOrder->UpdateTime; // ����޸�ʱ�� char 9
			// pOrder->CancelTime; // ����ʱ�� char 9
			// pOrder->ActiveTraderID; // ����޸Ľ���������Ա���� char 21
			// pOrder->ClearingPartID; // �����Ա��� char 11
			// pOrder->SequenceNo; // ��� int
			// pOrder->FrontID; // ǰ�ñ�� int
			// pOrder->SessionID; // �Ự��� int
			// pOrder->UserProductInfo; // �û��˲�Ʒ��Ϣ char 11
			// pOrder->UserForceClose; // �û�ǿ����־ int
			// pOrder->ActiveUserID; // �����û����� char 16
			// pOrder->BrokerOrderSeq; // ���͹�˾������� int
			// pOrder->RelativeOrderSysID; // ��ر��� char 21
			// pOrder->ZCETotalTradedVolume; // ֣�����ɽ����� int
			// pOrder->IsSwapOrder; // ��������־ int
			result_data = Json::writeString( m_json_writer, results_json );
		}
		catch( ... ) {
			log_info = "��ȡ OnRtnOrder ��Ϣ�쳣��";
			m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );
		}

		if( result_data != "" ) {
			m_trader_ctp_p->CommitResult( 1, request->m_identity, NW_MSG_CODE_JSON, result_data ); // �ر�ͳһ�� NW_MSG_CODE_JSON ���� //Trade��1��Risks��2
		}

		asset_account = request->m_req_json["asset_account"].asString();
	}
	else {
		FormatLibrary::StandardLibrary::FormatTo( log_info, "�����ر��ص�����ʧ�ܣ�{0}", pOrder->RequestID );
		m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );
	}

	//////////////////// ���͸���ط���� ////////////////////
	try {
		Json::Value results_json; // �ر�ͳһ�� NW_MSG_CODE_JSON ����
		results_json["ret_func"] = TD_FUNC_RISKS_ORDER_REPORT_FUE;
		results_json["task_id"] = 0;
		results_json["asset_account"] = asset_account; // ��Ʒ�˺�
		results_json["account"] = m_username; // �����˺�
		results_json["order_id"] = pOrder->OrderRef; // �������� char 13
		results_json["order_sys_id"] = pOrder->OrderSysID; // ������� char 21
		results_json["instrument"] = pOrder->InstrumentID; // ��Լ���� char 31
		results_json["exchange"] = pOrder->ExchangeID; // ���������� char 9
		results_json["exch_side"] = atoi( direction.c_str() ) + 1; // �������� char
		results_json["fill_qty"] = pOrder->VolumeTraded; // �ɽ����� int
		// ������ȡ n_status �Ļ� "status" �ֶ�һ������
		if( pOrder->OrderStatus != 'a' && pOrder->OrderStatus != 'b' && pOrder->OrderStatus != 'c' ) {
			// 0��ȫ���ɽ���1�����ֳɽ���2�����ɲ�����3���ѱ�δ�ɣ�4���Զ�����5��ȫ������
			int32_t status_temp[] = { 5, 4, 7, 3, 13, 8 };
			std::string s_status( 1, pOrder->OrderStatus ); // ����״̬ char
			int32_t n_status = atoi( s_status.c_str() );
			if( n_status >= 0 && n_status <= 5 ) {
				results_json["status"] = status_temp[n_status];
			}
		}
		else if( 'a' == pOrder->OrderStatus ) { // δ֪
			results_json["status"] = 14;
		}
		else if( 'b' == pOrder->OrderStatus ) { // ��δ����
			results_json["status"] = 11;
		}
		else if( 'c' == pOrder->OrderStatus ) { // �Ѿ�����
			results_json["status"] = 12;
		}
		results_json["status_msg"] = basicx::StringToUTF8( pOrder->StatusMsg ); // ״̬��Ϣ char 81
		result_risk = Json::writeString( m_json_writer, results_json );
	}
	catch( ... ) {
		log_info = "��أ���ȡ OnRtnOrder ��Ϣ�쳣��";
		m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );
	}

	if( result_risk != "" ) {
		m_risker->CommitResult( NW_MSG_CODE_JSON, result_risk ); // �ر�ͳһ�� NW_MSG_CODE_JSON ����
	}
	//////////////////// ���͸���ط���� ////////////////////

	FormatLibrary::StandardLibrary::FormatTo( log_info, "�����ر���UserID��{0}, OrderRef��{1}, OrderSysID��{2}, Instrument��{3}, Exchange��{4}, ExchSide��{5}, FillQty��{6}, Status��{7}, StatusMsg��{8}", 
		pOrder->UserID, pOrder->OrderRef, pOrder->OrderSysID, pOrder->InstrumentID, pOrder->ExchangeID, atoi( direction.c_str() ) + 1, pOrder->VolumeTraded, std::string( 1, pOrder->OrderStatus ), pOrder->StatusMsg );
	m_trader_ctp_p->LogPrint( basicx::syslog_level::c_debug, log_info, FILE_LOG_ONLY );
}

void Session::OnRtnTrade( CThostFtdcTradeField* pTrade ) {
	if( true == m_query_only ) { // �ӿڽ�����ѯʱ�ر�ί�кͳɽ��ر�����
		return;
	}

	std::string log_info;
	std::string result_risk = "";
	std::string result_data = "";
	std::string asset_account = "";

	std::string direction( 1, pTrade->Direction );

	CThostFtdcOrderField* order_item = GetOrderItemByID( atoi( pTrade->OrderRef ) );
	if( nullptr == order_item ) {
		FormatLibrary::StandardLibrary::FormatTo( log_info, "�ɽ��ر� ί�� ����ʧ�ܣ�{0}", pTrade->OrderRef );
		m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );
		return;
	}

	Request* request = GetRequestByID( order_item->RequestID );
	if( request != nullptr ) {
		try {
			Json::Value results_json; // �ر�ͳһ�� NW_MSG_CODE_JSON ����
			results_json["ret_func"] = 290002;
			results_json["task_id"] = request->m_req_json["task_id"].asInt(); // ע�⣺����Ӧ��ԭΪ�û��� task_id ����ֱ���÷���˵�������
			results_json["order_id"] = pTrade->OrderRef; // �������� char 13
			results_json["trans_id"] = pTrade->TradeID; // �ɽ���� char 21
			results_json["instrument"] = pTrade->InstrumentID; // ��Լ���� char 31
			results_json["exchange"] = pTrade->ExchangeID; // ���������� char 9
			results_json["exch_side"] = atoi( direction.c_str() ) + 1; // �������� char
			results_json["fill_qty"] = pTrade->Volume; // �ɽ����� int
			results_json["fill_price"] = pTrade->Price; // �ɽ��۸� double
			results_json["fill_time"] = pTrade->TradeTime; // �ɽ�ʱ�� char 9
			// pTrade->BrokerID; // ���͹�˾���� char 11
			// pTrade->InvestorID; // Ͷ���ߴ��� char 13
			// pTrade->UserID; // �û����� char 16
			// pTrade->OrderSysID; // ������� char 21
			// pTrade->ParticipantID; // ��Ա���� char 11
			// pTrade->ClientID; // �ͻ����� char 11
			// pTrade->TradingRole; // ���׽�ɫ char
			// pTrade->ExchangeInstID; // ��Լ�ڽ������Ĵ��� char 31
			// pTrade->OffsetFlag; // ��ƽ��־ char
			// pTrade->HedgeFlag; // Ͷ���ױ���־ char
			// pTrade->TradeDate; // �ɽ�ʱ�� char 9
			// pTrade->TradeType; // �ɽ����� char
			// pTrade->PriceSource; // �ɽ�����Դ char
			// pTrade->TraderID; // ����������Ա���� char 21
			// pTrade->OrderLocalID; // ���ر������ char 13
			// pTrade->ClearingPartID; // �����Ա��� char 11
			// pTrade->BusinessUnit; // ҵ��Ԫ char 21
			// pTrade->SequenceNo; // ��� int
			// pTrade->TradingDay; // ������ char 9
			// pTrade->SettlementID; // ������ int
			// pTrade->BrokerOrderSeq; // ���͹�˾������� int
			// pTrade->TradeSource; // �ɽ���Դ char
			result_data = Json::writeString( m_json_writer, results_json );
		}
		catch( ... ) {
			log_info = "��ȡ OnRtnTrade ��Ϣ�쳣��";
			m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );
		}

		if( result_data != "" ) {
			m_trader_ctp_p->CommitResult( 1, request->m_identity, NW_MSG_CODE_JSON, result_data ); // �ر�ͳһ�� NW_MSG_CODE_JSON ���� // Trade��1��Risks��2
		}

		asset_account = request->m_req_json["asset_account"].asString();
	}
	else {
		FormatLibrary::StandardLibrary::FormatTo( log_info, "�ɽ��ر��ص�����ʧ�ܣ�{0}", order_item->RequestID );
		m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );
	}

	//////////////////// ���͸���ط���� ////////////////////
	try {
		Json::Value results_json; // �ر�ͳһ�� NW_MSG_CODE_JSON ����
		results_json["ret_func"] = TD_FUNC_RISKS_TRANSACTION_REPORT_FUE;
		results_json["task_id"] = 0;
		results_json["asset_account"] = asset_account; // ��Ʒ�˺�
		results_json["account"] = m_username; // �����˺�
		results_json["order_id"] = pTrade->OrderRef; // �������� char 13
		results_json["trans_id"] = pTrade->TradeID; // �ɽ���� char 21
		results_json["instrument"] = pTrade->InstrumentID; // ��Լ���� char 31
		results_json["exchange"] = pTrade->ExchangeID; // ���������� char 9
		results_json["exch_side"] = atoi( direction.c_str() ) + 1; // �������� char
		results_json["fill_qty"] = pTrade->Volume; // �ɽ����� int
		results_json["fill_price"] = pTrade->Price; // �ɽ��۸� double
		results_json["fill_time"] = pTrade->TradeTime; // �ɽ�ʱ�� char 9
		result_risk = Json::writeString( m_json_writer, results_json );
	}
	catch( ... ) {
		log_info = "��أ���ȡ OnRtnTrade ��Ϣ�쳣��";
		m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );
	}

	if( result_risk != "" ) {
		m_risker->CommitResult( NW_MSG_CODE_JSON, result_risk ); // �ر�ͳһ�� NW_MSG_CODE_JSON ����
	}
	//////////////////// ���͸���ط���� ////////////////////

	FormatLibrary::StandardLibrary::FormatTo( log_info, "�ɽ��ر���UserID��{0}, OrderRef��{1}, TradeID��{2}, Instrument��{3}, Exchange��{4}, ExchSide��{5}, FillQty��{6}, FillPrice��{7}", 
		pTrade->UserID, pTrade->OrderRef, pTrade->TradeID, pTrade->InstrumentID, pTrade->ExchangeID, atoi( direction.c_str() ) + 1, pTrade->Volume, pTrade->Price );
	m_trader_ctp_p->LogPrint( basicx::syslog_level::c_debug, log_info, FILE_LOG_ONLY );
}

void Session::OnRspError( CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast ) {
	std::string log_info;
	std::string result_data = "";
	Request* request = GetRequestByID( nRequestID );

	if( request != nullptr ) {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			Json::Value results_json;
			results_json["ret_func"] = request->m_req_json["function"].asInt();
			results_json["ret_task"] = request->m_req_json["task_id"].asInt();
			results_json["ret_last"] = bIsLast;
			//results_json["ret_data"] = "";

			if( pRspInfo && pRspInfo->ErrorID != 0 ) {
				if( pRspInfo ) {
					FormatLibrary::StandardLibrary::FormatTo( log_info, "�û��������֪ͨ��{0}", pRspInfo->ErrorMsg );
				}
				else {
					log_info = "�û��������֪ͨ��ԭ��δ֪��";
				}
				m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );

				results_json["ret_code"] = -1;
				results_json["ret_info"] = basicx::StringToUTF8( log_info );
				results_json["ret_numb"] = 0;
			}

			result_data = Json::writeString( m_json_writer, results_json );
		}

		m_trader_ctp_p->CommitResult( request->m_task_id, request->m_identity, request->m_code, result_data );
	}
	else {
		FormatLibrary::StandardLibrary::FormatTo( log_info, "�쳣�ص�����ʧ�ܣ�{0}", nRequestID );
		m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );
	}
}

std::string Session::GetLastErrorMsg() {
	std::string last_error_msg = m_last_error_msg;
	m_last_error_msg = "�޴�����Ϣ��";
	return last_error_msg;
}

int32_t Session::GetRequestID() {
	m_request_id++;
	if( m_request_id > 2147483600 ) { // 2147483648
		m_request_id = 1;
	}
	return m_request_id;
}

Request* Session::GetRequestByID( int32_t request_id ) {
	Request* request = nullptr;
	m_request_map_lock.lock();
	std::map<int32_t, Request>::iterator it_r = m_map_request.find( request_id );
	if( it_r != m_map_request.end() ) {
		request = &( it_r->second);
	}
	m_request_map_lock.unlock();
	return request;
}

CThostFtdcOrderField* Session::GetOrderItemByID( int32_t order_id ) {
	CThostFtdcOrderField* order_item = nullptr;
	m_order_map_lock.lock();
	std::map<int32_t, CThostFtdcOrderField>::iterator it_of = m_map_order.find( order_id );
	if( it_of != m_map_order.end() ) {
		order_item = &( it_of->second);
	}
	m_order_map_lock.unlock();
	return order_item;
}

void Session::CreateCtpTradeApi( std::string trade_front ) {
	std::string ext_folder = m_syscfg->GetPath_ExtFolder() + "\\";
	m_user_api = CThostFtdcTraderApi::CreateFtdcTraderApi( ext_folder.c_str() );
	m_user_api->RegisterSpi( (CThostFtdcTraderSpi*)this ); // ע���¼���
	m_user_api->RegisterFront( const_cast<char*>( trade_front.c_str()) ); // ע�ύ��ǰ�õ�ַ
	m_user_api->SubscribePublicTopic( THOST_TERT_QUICK ); // ע�ṫ���� // TERT_RESTART��TERT_RESUME��TERT_QUICK
	m_user_api->SubscribePrivateTopic( THOST_TERT_QUICK ); // ע��˽���� // TERT_RESTART��TERT_RESUME��TERT_QUICK
	m_user_api->Init();
	m_user_api->Join();
}

void Session::CreateServiceUser() {
	std::string log_info;

	FormatLibrary::StandardLibrary::FormatTo( log_info, "���� �Ự {0} ������������߳����, ��ʼ��������������� ...", m_session );
	m_trader_ctp_p->LogPrint( basicx::syslog_level::c_info, log_info );

	try {
		try {
			m_service_user = boost::make_shared<boost::asio::io_service>();
			boost::asio::io_service::work Work( *m_service_user );
			m_work_thread_user = boost::make_shared<boost::thread>( boost::bind( &boost::asio::io_service::run, m_service_user ) );
			m_service_user_running = true;
			m_work_thread_user->join();
		}
		catch( std::exception& ex ) {
			FormatLibrary::StandardLibrary::FormatTo( log_info, "�Ự {0} ����������� ��ʼ�� �쳣��{1}", m_session, ex.what() );
			m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );
		}
	} // try
	catch( ... ) {
		FormatLibrary::StandardLibrary::FormatTo( log_info, "�Ự {0} ������������̷߳���δ֪����", m_session );
		m_trader_ctp_p->LogPrint( basicx::syslog_level::c_fatal, log_info );
	}

	StopServiceUser();

	FormatLibrary::StandardLibrary::FormatTo( log_info, "�Ự {0} ������������߳��˳���", m_session );
	m_trader_ctp_p->LogPrint( basicx::syslog_level::c_warn, log_info );
}

void Session::HandleRequestMsg() {
	std::string log_info;

	try {
		std::string result_data = "";
		Request* request = &m_list_request.front(); // �϶���

		//FormatLibrary::StandardLibrary::FormatTo( log_info, "�Ự {0} ��ʼ���� {1} ������ ...", m_session, request->m_task_id );
		//m_trader_ctp_p->LogPrint( basicx::syslog_level::c_info, log_info );

		int32_t func_id = 0;
		int32_t task_id = 0;
		try {
			if( NW_MSG_CODE_JSON == request->m_code ) {
				func_id = request->m_req_json["function"].asInt();
				task_id = request->m_req_json["task_id"].asInt();
			}
			// func_id > 0 ���� HandleTaskMsg() �б�֤

			switch( func_id ) {
			case TD_FUNC_FUTURE_ADDSUB:
				result_data = OnSubscibe( request );
				break;
			case TD_FUNC_FUTURE_DELSUB:
				result_data = OnUnsubscibe( request );
				break;
			default: // ���������๦�ܱ��
				result_data = OnTradeRequest( request );
			}
		}
		catch( ... ) {
			FormatLibrary::StandardLibrary::FormatTo( log_info, "�Ự {0} �������� {1} ʱ����δ֪����", m_session, request->m_task_id );
			result_data = m_trader_ctp_p->OnErrorResult( func_id, -1, log_info, task_id, request->m_code );
		}

		if( result_data != "" ) { // ��Ϊ��˵���д������ɻص����ظ��û�
			m_trader_ctp_p->CommitResult( request->m_task_id, request->m_identity, request->m_code, result_data );
			if( 220001 == func_id || 220002 == func_id || 220003 == func_id ) { // ���֪ͨ
				std::string asset_account = request->m_req_json["asset_account"].asString();
				m_risker->CheckTradeResultForRisk( asset_account, func_id, task_id, result_data ); // ���� result_data Ϊ���� JSON ��䣬�����ֻʹ�ô�����Ϣ
			}
		}

		//FormatLibrary::StandardLibrary::FormatTo( log_info, "�Ự {0} ���� {1} ��������ɡ�", m_session, request->m_task_id );
		//m_trader_ctp_p->LogPrint( basicx::syslog_level::c_info, log_info );

		m_request_list_lock.lock();
		m_list_request.pop_front();
		bool write_on_progress = !m_list_request.empty();
		m_request_list_lock.unlock();

		if( write_on_progress && true == m_service_user_running ) { // m_service_user_running
			m_service_user->post( boost::bind( &Session::HandleRequestMsg, this ) );
		}
	}
	catch( std::exception& ex ) {
		FormatLibrary::StandardLibrary::FormatTo( log_info, "�Ự {0} ���� Request ��Ϣ �쳣��{1}", m_session, ex.what() );
		m_trader_ctp_p->LogPrint( basicx::syslog_level::c_error, log_info );
	}
}

void Session::StopServiceUser() {
	if( true == m_service_user_running ) {
		m_service_user_running = false;
		m_service_user->stop();
	}
}

std::string Session::OnSubscibe( Request* request ) {
	std::string log_info;

	int32_t task_id = 0;
	std::string password = "";
	if( NW_MSG_CODE_JSON == request->m_code ) {
		task_id = request->m_req_json["task_id"].asInt();
		password = request->m_req_json["password"].asString();
	}
	if( "" == password ) {
		FormatLibrary::StandardLibrary::FormatTo( log_info, "�û�����ʱ ���� Ϊ�գ�session��{0}", m_session );
		return m_trader_ctp_p->OnErrorResult( TD_FUNC_FUTURE_ADDSUB, -1, log_info, task_id, request->m_code );
	}
	if( m_password != password ) { // ��Ҫ������ܶ���
		FormatLibrary::StandardLibrary::FormatTo( log_info, "�û�����ʱ ���� ����session��{0}", m_session );
		return m_trader_ctp_p->OnErrorResult( TD_FUNC_FUTURE_ADDSUB, -1, log_info, task_id, request->m_code );
	}

	// ���� CTP ����Ҫ������ʵ ����
	
	m_sub_endpoint_map_lock.lock();
	m_map_sub_endpoint[request->m_identity] = request->m_identity;
	m_sub_endpoint_map_lock.unlock();

	FormatLibrary::StandardLibrary::FormatTo( log_info, "�û����ĳɹ���session��{0}", m_session );
	m_trader_ctp_p->LogPrint( basicx::syslog_level::c_info, log_info );

	if( NW_MSG_CODE_JSON == request->m_code ) {
		Json::Value results_json;
		results_json["ret_func"] = TD_FUNC_FUTURE_ADDSUB;
		results_json["ret_code"] = 0;
		results_json["ret_info"] = basicx::StringToUTF8( log_info );
		results_json["ret_task"] = task_id;
		results_json["ret_last"] = true;
		results_json["ret_numb"] = 0;
		results_json["ret_data"] = "";
		return Json::writeString( m_json_writer, results_json );
	}

	return "";
}

std::string Session::OnUnsubscibe( Request* request ) {
	std::string log_info;

	int32_t task_id = 0;
	if( NW_MSG_CODE_JSON == request->m_code ) {
		task_id = request->m_req_json["task_id"].asInt();
	}

	m_sub_endpoint_map_lock.lock();
	m_map_sub_endpoint.erase( request->m_identity );
	m_sub_endpoint_map_lock.unlock();

	if( m_map_sub_endpoint.empty() ) { // ���������û�
		// ���� CTP ����Ҫ������ʵ �˶�
	}

	FormatLibrary::StandardLibrary::FormatTo( log_info, "�û��˶��ɹ���session��{0}", m_session );
	m_trader_ctp_p->LogPrint( basicx::syslog_level::c_info, log_info );

	if( NW_MSG_CODE_JSON == request->m_code ) {
		Json::Value results_json;
		results_json["ret_func"] = TD_FUNC_FUTURE_DELSUB;
		results_json["ret_code"] = 0;
		results_json["ret_info"] = basicx::StringToUTF8( log_info );
		results_json["ret_task"] = task_id;
		results_json["ret_last"] = true;
		results_json["ret_numb"] = 0;
		results_json["ret_data"] = "";
		return Json::writeString( m_json_writer, results_json );
	}

	return "";
}

std::string Session::OnTradeRequest( Request* request ) {
	std::string log_info = "";

	int32_t func_id = 0;
	int32_t task_id = 0;
	if( NW_MSG_CODE_JSON == request->m_code ) {
		func_id = request->m_req_json["function"].asInt();
		task_id = request->m_req_json["task_id"].asInt();
	}

	if( 220001 == func_id || 220002 == func_id || 220003 == func_id ) { // ��ؼ��
		std::string asset_account = request->m_req_json["asset_account"].asString();
		int32_t risk_ret = m_risker->HandleRiskCtlCheck( asset_account, func_id, task_id, request, log_info );
		if( risk_ret < 0 ) {
			return m_trader_ctp_p->OnErrorResult( func_id, risk_ret, log_info, task_id, request->m_code );
		}
	}

	int32_t ret = 0;
	switch( func_id ) {
	case 220001: // �����ڻ�ί���µ�
		ret = ReqOrderInsert( request, false );
		return m_trader_ctp_p->TransErrorResult( func_id, ret, "�����ڻ� �µ� ʱ����ʧ�ܣ�", task_id, request->m_code ); // �д�ʱ�����ﷵ�ظ��û����޴�ʱ���ؿգ����ɻص����ظ��û�
	case 220002: // �����ڻ�ί�г���
		ret = ReqOrderAction( request );
		return m_trader_ctp_p->TransErrorResult( func_id, ret, "�����ڻ� ���� ʱ����ʧ�ܣ�", task_id, request->m_code ); // �д�ʱ�����ﷵ�ظ��û����޴�ʱ���ؿգ����ɻص����ظ��û�
	case 220003: // �����ڻ�ί���µ�
		ret = ReqOrderInsert( request, true );
		return m_trader_ctp_p->TransErrorResult( func_id, ret, "�����ڻ� �µ� ʱ����ʧ�ܣ�", task_id, request->m_code ); // �д�ʱ�����ﷵ�ظ��û����޴�ʱ���ؿգ����ɻص����ظ��û�
	case 230002: // ��ѯ�ͻ��ʽ�
		ret = ReqQryTradingAccount( request );
		return m_trader_ctp_p->TransErrorResult( func_id, ret, "��ѯ �ͻ��ʽ� ʱ����ʧ�ܣ�", task_id, request->m_code ); // �д�ʱ�����ﷵ�ظ��û����޴�ʱ���ؿգ����ɻص����ظ��û�
	case 230004: // ��ѯ�ͻ��ֲ�
		ret = ReqQryInvestorPosition( request );
		return m_trader_ctp_p->TransErrorResult( func_id, ret, "��ѯ �ͻ��ֲ� ʱ����ʧ�ܣ�", task_id, request->m_code ); // �д�ʱ�����ﷵ�ظ��û����޴�ʱ���ؿգ����ɻص����ظ��û�
	case 230005: // ��ѯ�ͻ�����ί��
		ret = ReqQryOrder( request );
		return m_trader_ctp_p->TransErrorResult( func_id, ret, "��ѯ �ͻ�����ί�� ʱ����ʧ�ܣ�", task_id, request->m_code ); // �д�ʱ�����ﷵ�ظ��û����޴�ʱ���ؿգ����ɻص����ظ��û�
	case 230006: // ��ѯ�ͻ����ճɽ�
		ret = ReqQryTrade( request );
		return m_trader_ctp_p->TransErrorResult( func_id, ret, "��ѯ �ͻ����ճɽ� ʱ����ʧ�ܣ�", task_id, request->m_code ); // �д�ʱ�����ﷵ�ظ��û����޴�ʱ���ؿգ����ɻص����ظ��û�
	case 230009: // ��ѯ�ڻ���Լ
		ret = ReqQryInstrument( request );
		return m_trader_ctp_p->TransErrorResult( func_id, ret, "��ѯ �ڻ���Լ ʱ����ʧ�ܣ�", task_id, request->m_code ); // �д�ʱ�����ﷵ�ظ��û����޴�ʱ���ؿգ����ɻص����ظ��û�
	default:
		FormatLibrary::StandardLibrary::FormatTo( log_info, "ҵ�� {0} δ֪��", func_id );
		return m_trader_ctp_p->OnErrorResult( func_id, -1, log_info, task_id, request->m_code );
	}

	return "";
}
