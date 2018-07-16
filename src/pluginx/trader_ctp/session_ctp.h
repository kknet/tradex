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

#ifndef TRADER_CTP_SESSION_CTP_H
#define TRADER_CTP_SESSION_CTP_H

#include "../../global/compile.h"

#ifdef TRADER_CTP_COMMUNITY
    #include "risker_ctp.h"
#endif
#ifdef TRADER_CTP_PROFESSIONAL
    #include "risker_ctp_pro.h"
#endif

#define FILE_LOG_ONLY 1

class TraderCTP_P;

// һ���Ự��һ��������һ���˻����߹�����ÿ���Ự�����ж���ͻ�����ͬһ�˺Ž��н���
// �ر����� m_map_sub_endpoint �е����ӱ�ʶ���й㲥���ѶϿ������ӽ��ڷ�������ʱȡ�ÿ�����ָ��
// ���׸��� request �����ӱ�ʶ���н�����ͣ��ͻ��˷������ǳ��� m_map_con_endpoint �����ӱ�ʶ��������
// ������ڷ������ǳ��ͻ��ˣ���ʹ�ûỰ������Ч���ӣ���Ϊ m_map_con_endpoint �ǿոûỰ��������
// ��δ���������ӵĻỰ��֤ʱ���������ǳ��������ǳ��Ŀͻ��˾���ʹ��ԭ�Ự�����Ӻ�ֱ�ӽ���
// ���������ӵĻỰ��֤ʱ����Ϊÿ�����ӱ�ʶ��ͬ�����Ա������µ�¼�����ӱ�ʶ����� m_map_con_endpoint ����ܽ���
// ��Ϊ�ر����� m_map_sub_endpoint �����ӱ�ʶ�㲥�����Է������˶��������˶�����Ҫ�������Ӳ����Ĳ����յ��ر�����
// ���׺ͻر�����ʹ��ͬһ���ӣ�����ʱ�ͻ�����߱��첽�����׽��й����д���Ļر����ݵ�����

// ���� CTP �� OrderRef �� OrderActionRef ��ʵ�ͻ����ڱ����µ��򳷵�ʱ�ǿ����Լ�ָ���ģ����������Ƚ� Order �����ڴ������µ�Ȼ��ȴ��ر�
// ���ܶ�ͬ���Ľ��׽ӿ����ǲ���ָ���ģ�ֻ�ܴ�ί�����󷵻ص������л�ã��� CTP ���൱��Ҫ�ȴ� OnRspOrderInsert ����
// ��������û�������ȷ�������ᡰ�����յ�������Ӧ OnRspOrderInsert���� ֻ�б����� CTP �ܾ��Ż��յ�
// ����Ŀǰ OrderRef �� OrderActionRef ͳһ�ɷ�������ɣ�Ҳ�������ͻ��˵�¼ͬһ�˺�ʱί�кŵķ�������
// �ͻ����µ������յ� OnRtnOrder ʱ������ task_id ��ʶ�Ƿ����Լ���ί�У��ٽ� Order �����ڴ���й�������Ҳһ�����᲻���гɽ��ر����ڱ����ر������⣿��
// �鿴���� Patsystems �Ľӿڣ����µ���Ҳ��һ����ʱ��ί�кţ��ڱ����������Ժ�Ż�����ʽ��ί�кţ������ task_id ��������ʱί�к�

class Session : public CThostFtdcTraderSpi
{
public:
	Session( TraderCTP_P* trader_ctp_p );
	~Session();

public:
	// ���ͻ����뽻�׺�̨������ͨ������ʱ����δ��¼ǰ�����÷��������á�
	void OnFrontConnected() override;
	// ���ͻ����뽻�׺�̨ͨ�����ӶϿ�ʱ���÷��������á���������������API���Զ��������ӣ��ͻ��˿ɲ�������
	void OnFrontDisconnected( int nReason ) override;
	// ������ʱ���档����ʱ��δ�յ�����ʱ���÷��������á�
	void OnHeartBeatWarning( int nTimeLapse ) override;
	// ��¼������Ӧ��
	void OnRspUserLogin( CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast ) override;
	// �ǳ�������Ӧ��
	void OnRspUserLogout( CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast ) override;
	// Ͷ���߽�����ȷ����Ӧ��
	void OnRspSettlementInfoConfirm( CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast ) override;
	// ����¼��������Ӧ��
	void OnRspOrderInsert( CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast ) override;
	// ����������¼�����ر���
	void OnErrRtnOrderInsert( CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo ) override;
	// ��������������Ӧ��
	void OnRspOrderAction( CThostFtdcInputOrderActionField* pInputOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast ) override;
	// ������������������ر���
	void OnErrRtnOrderAction( CThostFtdcOrderActionField* pOrderAction, CThostFtdcRspInfoField* pRspInfo ) override;
	// �����ѯ�ʽ��˻���Ӧ��
	void OnRspQryTradingAccount( CThostFtdcTradingAccountField* pTradingAccount, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast ) override;
	// �����ѯͶ���ֲ߳���Ӧ��
	void OnRspQryInvestorPosition( CThostFtdcInvestorPositionField* pInvestorPosition, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast ) override;
	// �����ѯ������Ӧ��
	void OnRspQryOrder( CThostFtdcOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast ) override;
	// �����ѯ�ɽ���Ӧ��
	void OnRspQryTrade( CThostFtdcTradeField* pTrade, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast ) override;
	// �����ѯ��Լ��Ӧ��
	void OnRspQryInstrument( CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast ) override;

	// ����֪ͨ��
	void OnRtnOrder( CThostFtdcOrderField* pOrder ) override;
	// �ɽ�֪ͨ��
	void OnRtnTrade( CThostFtdcTradeField* pTrade ) override;
	// ����Ӧ��
	void OnRspError( CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast ) override;

public:
	// �û���¼����
	int32_t ReqUserLogin( std::string broker_id, std::string user_id, std::string password );
	// �û��ǳ�����
	int32_t ReqUserLogout();
	// Ͷ���߽�����ȷ�ϡ�
	int32_t ReqSettlementInfoConfirm();
	// ����¼������
	int32_t ReqOrderInsert( Request* request, bool is_arbitrage );
	// ������������
	int32_t ReqOrderAction( Request* request );
	// �����ѯ�ʽ��˻���
	int32_t ReqQryTradingAccount( Request* request );
	// �����ѯͶ���ֲ߳֡�
	int32_t ReqQryInvestorPosition( Request* request );
	// �����ѯ������
	int32_t ReqQryOrder( Request* request );
	// �����ѯ�ɽ���
	int32_t ReqQryTrade( Request* request );
	// �����ѯ��Լ��
	int32_t ReqQryInstrument( Request* request );

public:
	std::string GetLastErrorMsg(); // ���ú� m_last_error_msg ��������
	int32_t GetRequestID();
	Request* GetRequestByID( int32_t request_id );
	CThostFtdcOrderField* GetOrderItemByID( int32_t order_id );
	void CreateCtpTradeApi( std::string trade_front );
	void CreateServiceUser();
	void HandleRequestMsg();
	void StopServiceUser();

	std::string OnSubscibe( Request* request );
	std::string OnUnsubscibe( Request* request );
	std::string OnTradeRequest( Request* request );
	
public:
	int32_t m_session;
	risker_ptr m_risker;
	std::string m_username;
	std::string m_password;
	std::string m_broker_id;
	bool m_query_only; // �ӿ��Ƿ������ѯ
	boost::mutex m_con_endpoint_map_lock;
	std::map<int32_t, int32_t> m_map_con_endpoint;
	boost::mutex m_sub_endpoint_map_lock;
	std::map<int32_t, int32_t> m_map_sub_endpoint;

	service_ptr m_service_user;
	bool m_service_user_running;
	thread_ptr m_thread_service_user;
	thread_ptr m_work_thread_user;

	CThostFtdcTraderApi* m_user_api;
	thread_ptr m_init_api_thread_user;
	bool m_last_rsp_is_error; // ��������첽����
	std::string m_last_error_msg; // �������������Ϣ
	int32_t m_request_id; // �����ʶ���
	bool m_connect_ok; // �����ѳɹ�
	bool m_login_ok; // ��¼�ѳɹ�
	bool m_logout_ok; // �ǳ��ѳɹ�
	bool m_settle_ok; // ����ȷ���ѳɹ�
	int32_t m_front_id; // ǰ�ñ��
	int32_t m_session_id; // �Ự���

	boost::mutex m_request_list_lock;
	std::list<Request> m_list_request;

	boost::mutex m_request_map_lock;
	std::map<int32_t, Request> m_map_request; // �ȴ��첽�ص�
	boost::mutex m_order_map_lock;
	std::map<int32_t, CThostFtdcOrderField> m_map_order; // ��Ȼ OrderRef Ϊ�ַ��������ﻹ��ת����������߲���Ч�ʰ�

	Json::StreamWriterBuilder m_json_writer;

private:
	std::string m_log_cate;
	basicx::SysCfg_S* m_syscfg;
	TraderCTP_P* m_trader_ctp_p;
};

#endif // TRADER_CTP_SESSION_CTP_H
