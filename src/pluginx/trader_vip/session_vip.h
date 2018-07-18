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

#ifndef TRADER_VIP_SESSION_VIP_H
#define TRADER_VIP_SESSION_VIP_H

#include "../../global/compile.h"

#ifdef TRADER_VIP_COMMUNITY
    #include "risker_vip.h"
#endif
#ifdef TRADER_VIP_PROFESSIONAL
    #include "risker_vip_pro.h"
#endif

#include "set_field_vip.h"
#include "get_field_vip.h"

#define FILE_LOG_ONLY 1

#define VIP_FID_MESSAGE_LENGTH 256 // 255 + 1

class TraderVIP_P;

// һ���Ự��һ��������һ���˻����߹�����ÿ���Ự�����ж���ͻ�����ͬһ�˺Ž��н���
// �ر����� m_map_sub_endpoint �е����ӱ�ʶ���й㲥���ѶϿ������ӽ��ڷ�������ʱȡ�ÿ�����ָ��
// ���׸��� request �����ӱ�ʶ���н�����ͣ��ͻ��˷������ǳ��� m_map_con_endpoint �����ӱ�ʶ��������
// ������ڷ������ǳ��ͻ��ˣ���ʹ�ûỰ������Ч���ӣ���Ϊ m_map_con_endpoint �ǿոûỰ��������
// ��δ���������ӵĻỰ��֤ʱ���������ǳ��������ǳ��Ŀͻ��˾���ʹ��ԭ�Ự�����Ӻ�ֱ�ӽ���
// ���������ӵĻỰ��֤ʱ����Ϊÿ�����ӱ�ʶ��ͬ�����Ա������µ�¼�����ӱ�ʶ����� m_map_con_endpoint ����ܽ���
// ��Ϊ�ر����� m_map_sub_endpoint �����ӱ�ʶ�㲥�����Է������˶��������˶�����Ҫ�������Ӳ����Ĳ����յ��ر�����
// ���׺ͻر�����ʹ��ͬһ���ӣ�����ʱ�ͻ�����߱��첽�����׽��й����д���Ļر����ݵ�����

class Session
{
public:
	Session( TraderVIP_P* trader_vip_p );
	~Session();

public:
	void CreateServiceUser();
	void HandleRequestMsg();
	void StopServiceUser();

	std::string OnSubscibe( Request* request );
	std::string OnUnsubscibe( Request* request );
	std::string OnTradeRequest( Request* request, HANDLE_SESSION api_session );
	std::string OnTradeRequest_Simulate( Request* request, HANDLE_SESSION api_session );
	Request* GetRequestByOrderRef( int32_t order_ref );

	bool CallBackEvent( HANDLE_CONN api_connect, HANDLE_SESSION api_session, long subscibe, int32_t func_id );

public:
	int32_t m_session;
	risker_ptr m_risker;
	std::string m_username;
	std::string m_password;
	std::string m_node_info;
	std::string m_sys_user_id;
	boost::mutex m_con_endpoint_map_lock;
	std::map<int32_t, int32_t> m_map_con_endpoint;
	boost::mutex m_sub_endpoint_map_lock;
	std::map<int32_t, int32_t> m_map_sub_endpoint;
	long m_connect; // ���Ӿ��
	bool m_connect_ok; // ���ӿ���
	bool m_subscibe_ok; // ���Ŀ���
	long m_subscibe_cj; // ί�гɽ����ľ��
	long m_subscibe_sb; // ί���걨���ľ��
	long m_subscibe_cd; // ί�г������ľ��
	boost::mutex m_call_back_event_lock_cj;
	boost::mutex m_call_back_event_lock_sb;
	boost::mutex m_call_back_event_lock_cd;

	service_ptr m_service_user;
	bool m_service_user_running;
	thread_ptr m_thread_service_user;
	thread_ptr m_work_thread_user;

	boost::mutex m_request_list_lock;
	std::list<Request> m_list_request;

	long m_fid_code;
	char m_fid_message[VIP_FID_MESSAGE_LENGTH];

	Json::StreamWriterBuilder m_json_writer;

	SetField m_set_field;
	GetField m_get_field;
	std::unordered_map<int32_t, SetField::SetFieldFunc>* m_map_set_field_func;
	std::unordered_map<int32_t, GetField::GetFieldFunc>* m_map_get_field_func;

	boost::mutex m_order_ref_request_map_lock;
	std::map<int32_t, Request> m_map_order_ref_request;

private:
	std::string m_log_cate;
	TraderVIP_P* m_trader_vip_p;
};

#endif // TRADER_VIP_SESSION_VIP_H
