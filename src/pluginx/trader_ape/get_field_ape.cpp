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

#include "get_field_ape.h"

GetField::GetField() {
	m_syslog = basicx::SysLog_S::GetInstance();

	m_map_get_field_func[120001] = &GetField::GetField_120001_620001;
	m_map_get_field_func[120002] = &GetField::GetField_120002_620021;
	m_map_get_field_func[120003] = &GetField::GetField_120003_620002;
	m_map_get_field_func[120004] = &GetField::GetField_120004_620022;
	m_map_get_field_func[130002] = &GetField::GetField_130002_630002;
	m_map_get_field_func[130004] = &GetField::GetField_130004_630004;
	m_map_get_field_func[130005] = &GetField::GetField_130005_630005;
	m_map_get_field_func[130006] = &GetField::GetField_130006_630006;
	m_map_get_field_func[130008] = &GetField::GetField_130008_601410;
	m_map_get_field_func[130009] = &GetField::GetField_130009_601411;
	m_map_get_field_func[190001] = &GetField::GetField_190001_100065;
	m_map_get_field_func[190002] = &GetField::GetField_190002_100064;
	m_map_get_field_func[190003] = &GetField::GetField_190003_100066;
}

GetField::~GetField() {
}

void GetField::FillHead( Json::Value& results_json, int32_t ret_func, int32_t ret_numb, Request* request ) {
	results_json["ret_func"] = ret_func;
	results_json["ret_code"] = 0;
	results_json["ret_info"] = basicx::StringToUTF8( "ҵ���ύ�ɹ���" );
	results_json["ret_task"] = request->m_req_json["task_id"].asInt();
	results_json["ret_last"] = true;
	results_json["ret_numb"] = ret_numb == 0 ? 1 : ret_numb;
	//results_json["ret_data"] = "";
}

void GetField::FillHeadQuery( Json::Value& results_json, int32_t ret_func, int32_t ret_numb, Request* request ) {
	results_json["ret_func"] = ret_func;
	results_json["ret_code"] = 0;
	if( ret_numb > 0 ) {
		results_json["ret_info"] = basicx::StringToUTF8( "ҵ���ύ�ɹ���" );
	}
	else {
		results_json["ret_info"] = basicx::StringToUTF8( "ҵ���ύ�ɹ����޽�����ݣ�" );
	}
	results_json["ret_task"] = request->m_req_json["task_id"].asInt();
	results_json["ret_last"] = true;
	results_json["ret_numb"] = ret_numb;
	//results_json["ret_data"] = "";
}

bool GetField::GetField_120001_620001( int32_t api_session, Request* request, std::string& results ) { // ����ί���µ�
	try {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			Json::Value results_json;
			int32_t ret_numb = Fix_GetCount( api_session ); // ҵ��ִ�г���ʱ ret_numb == 0
			FillHead( results_json, 120001, ret_numb, request ); // 120001 // FillHead
			int32_t i = ret_numb == 0 ? -1 : 0; // ���ף�ҵ��ʧ����ȡ��һ��
			for( ; i < ret_numb; i++ ) {
				Json::Value ret_data_json;
				ret_data_json["otc_code"] = Fix_GetLong( api_session, 507, i ); // FID_CODE ������ Int
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["otc_info"] = Fix_GetItem( api_session, 508, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_MESSAGE ����˵�� Char 255
				ret_data_json["order_id"] = Fix_GetLong( api_session, 681, i ); // FID_WTH ί�к� Int
				// FID_WTPCH ί�����κ� Int
				results_json["ret_data"].append( ret_data_json );
			}
			results = m_json_writer.write( results_json );
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool GetField::GetField_120002_620021( int32_t api_session, Request* request, std::string& results ) { // ����ί�г���
	try {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			Json::Value results_json;
			int32_t ret_numb = Fix_GetCount( api_session ); // ҵ��ִ�г���ʱ ret_numb == 0
			FillHead( results_json, 120002, ret_numb, request ); // 120002 // FillHead
			int32_t i = ret_numb == 0 ? -1 : 0; // ���ף�ҵ��ʧ����ȡ��һ��
			for( ; i < ret_numb; i++ ) {
				Json::Value ret_data_json;
				ret_data_json["otc_code"] = Fix_GetLong( api_session, 507, i ); // FID_CODE ������ Int
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["otc_info"] = Fix_GetItem( api_session, 508, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_MESSAGE ����˵�� Char 255
				ret_data_json["order_id"] = Fix_GetLong( api_session, 681, i ); // FID_WTH ί�к�(����) Int
				results_json["ret_data"].append( ret_data_json );
			}
			results = m_json_writer.write( results_json );
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool GetField::GetField_120003_620002( int32_t api_session, Request* request, std::string& results ) { // ����ί���µ�
	try {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			Json::Value results_json;
			int32_t ret_numb = Fix_GetCount( api_session ); // ҵ��ִ�г���ʱ ret_numb == 0
			FillHead( results_json, 120003, ret_numb, request ); // 120003 // FillHead
			int32_t i = ret_numb == 0 ? -1 : 0; // ���ף�ҵ��ʧ����ȡ��һ��
			for( ; i < ret_numb; i++ ) {
				Json::Value ret_data_json;
				ret_data_json["otc_code"] = Fix_GetLong( api_session, 507, i ); // FID_CODE ������ Int
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["otc_info"] = Fix_GetItem( api_session, 508, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_MESSAGE ����˵�� Char 255
				ret_data_json["batch_id"] = Fix_GetLong( api_session, 1017, i ); // FID_WTPCH ί�����κ� Int
				ret_data_json["batch_ht"] = Fix_GetItem( api_session, 705, m_field_value_huge, FIELD_VALUE_HUGE, i ); // FID_EN_WTH ί�к�ͬ�� Char 6000
				// FID_COUNT ί�гɹ����� Int
				results_json["ret_data"].append( ret_data_json );
			}
			results = m_json_writer.write( results_json );
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool GetField::GetField_120004_620022( int32_t api_session, Request* request, std::string& results ) { // ����ί�г���
	try {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			Json::Value results_json;
			int32_t ret_numb = Fix_GetCount( api_session ); // ҵ��ִ�г���ʱ ret_numb == 0
			FillHead( results_json, 120004, ret_numb, request ); // 120004 // FillHead
			int32_t i = ret_numb == 0 ? -1 : 0; // ���ף�ҵ��ʧ����ȡ��һ��
			for( ; i < ret_numb; i++ ) {
				Json::Value ret_data_json;
				ret_data_json["otc_code"] = Fix_GetLong( api_session, 507, i ); // FID_CODE ������ Int
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["otc_info"] = Fix_GetItem( api_session, 508, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_MESSAGE ����˵�� Char 255
				results_json["ret_data"].append( ret_data_json );
			}
			results = m_json_writer.write( results_json );
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool GetField::GetField_130002_630002( int32_t api_session, Request* request, std::string& results ) { // ��ѯ�ͻ��ʽ�
	try {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			Json::Value results_json;
			int32_t ret_numb = Fix_GetCount( api_session ); // ҵ��ִ�г���ʱ ret_numb == 0
			FillHeadQuery( results_json, 130002, ret_numb, request ); // 130002 // FillHeadQuery
			int32_t i = 0; // ��ѯ��ҵ��ʧ����ȡ�������
			for( ; i < ret_numb; i++ ) {
				Json::Value ret_data_json;
				ret_data_json["otc_code"] = Fix_GetLong( api_session, 507, i ); // FID_CODE ������ Int
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["otc_info"] = Fix_GetItem( api_session, 508, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_MESSAGE ����˵�� Char 255
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["account"] = Fix_GetItem( api_session, 716, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_ZJZH �ʽ��˺� Char 20
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["currency"] = Fix_GetItem( api_session, 511, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_BZ ���� Char  3
				ret_data_json["available"] = Fix_GetDouble( api_session, 619, i ); // FID_KYZJ �����ʽ� Numric 16,2
				ret_data_json["balance"] = Fix_GetDouble( api_session, 709, i ); // FID_ZHYE �˻���� Numric 16,2
				ret_data_json["frozen"] = Fix_GetDouble( api_session, 826, i ); // FID_DJJE ������ Numric 16,2
				// FID_YJLX  Ԥ����Ϣ  Numric  16,2
				// FID_ZHZT  �˻�״̬  Int
				// FID_JGDM  ��������  Char    4
				results_json["ret_data"].append( ret_data_json );
			}
			results = m_json_writer.write( results_json );
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool GetField::GetField_130004_630004( int32_t api_session, Request* request, std::string& results ) { // ��ѯ�ͻ��ֲ�
	try {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			Json::Value results_json;
			int32_t ret_numb = Fix_GetCount( api_session ); // ҵ��ִ�г���ʱ ret_numb == 0
			FillHeadQuery( results_json, 130004, ret_numb, request ); // 130004 // FillHeadQuery
			int32_t i = 0; // ��ѯ��ҵ��ʧ����ȡ�������
			for( ; i < ret_numb; i++ ) {
				Json::Value ret_data_json;
				ret_data_json["otc_code"] = Fix_GetLong( api_session, 507, i ); // FID_CODE ������ Int
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["otc_info"] = Fix_GetItem( api_session, 508, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_MESSAGE ����˵�� Char 255
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["holder"] = Fix_GetItem( api_session, 571, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_GDH �ɶ��� Char 10
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["exchange"] = Fix_GetItem( api_session, 599, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_JYS ���������� Char 2
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["currency"] = Fix_GetItem( api_session, 511, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_BZ ���� Char 3
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["symbol"] = Fix_GetItem( api_session, 719, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_ZQDM ֤ȯ���� Char 6
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["security_type"] = Fix_GetItem( api_session, 720, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_ZQLB ֤ȯ��� Char 2
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["security_name"] = Fix_GetItem( api_session, 722, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_ZQMC ֤ȯ���� Char 8
				ret_data_json["security_qty"] = Fix_GetLong( api_session, 757, i ); // FID_JCCL ��ֲ��� Long //��Ҫ��֤ȯ����
				ret_data_json["can_sell"] = Fix_GetLong( api_session, 615, i ); // FID_KMCSL ���������� Long
				ret_data_json["can_sub"] = Fix_GetLong( api_session, 1179, i ); // FID_KSGSL ���깺���� Long
				ret_data_json["can_red"] = Fix_GetLong( api_session, 1184, i ); // FID_KSHSL ��������� Long
				ret_data_json["non_tradable"] = Fix_GetLong( api_session, 568, i ); // FID_FLTSL ����ͨ���� Long
				ret_data_json["frozen_qty"] = Fix_GetLong( api_session, 1235, i ); // FID_DJSL �������� Long
				ret_data_json["sell_qty"] = Fix_GetLong( api_session, 541, i ); // FID_DRMCCJSL ���������ɽ����� Long
				ret_data_json["sell_money"] = Fix_GetDouble( api_session, 540, i ); // FID_DRMCCJJE ���������ɽ���� Numric 16,2
				ret_data_json["buy_qty"] = Fix_GetLong( api_session, 545, i ); // FID_DRMRCJSL ��������ɽ����� Long
				ret_data_json["buy_money"] = Fix_GetDouble( api_session, 544, i ); // FID_DRMRCJJE ��������ɽ���� Numric 16,2
				ret_data_json["sub_qty"] = Fix_GetLong( api_session, 9299, i ); // FID_SGCJSL �깺�ɽ����� Long
				ret_data_json["red_qty"] = Fix_GetLong( api_session, 9300, i ); // FID_SHCJSL ��سɽ����� Long
				// FID_DRMCWTSL  ��������ί������  Long
				// FID_DRMRWTSL  ��������ί������  Long
				// FID_KCRQ      ��������          Int
				// FID_ZQSL      ֤ȯ����          Long
				// FID_WJSSL     δ��������        Long
				// FID_BDRQ      �䶯����          Int
				// FID_MCDXSL    ������������      Long
				// FID_MRDXSL    �����������      Long
				// FID_JYDW      ���׵�λ          Int
				// FID_MCSL      �ۼ���������      Long
				// FID_MRSL      �ۼ���������      Long
				// FID_PGSL      �������          Long
				// FID_SGSL      �깺����          Long
				// FID_TBFDYK    ̯������ӯ��      Numric  16,2
				// FID_TBBBJ     ̯��������        Numric  9,3
				// FID_TBCBJ     ̯���ɱ���        Numric  9,3
				// FID_CCJJ      �ֲ־���          Numric  9,3
				// FID_FDYK      ����ӯ��          Numric  16,2
				// FID_HLJE      �������          Numric  16,2
				// FID_LJYK      �ۼ�ӯ��          Numric  16,2
				// FID_MCJE      �������          Numric  16,2
				// FID_MRJE      ������          Numric  16,2
				// FID_MRJJ      �������          Numric  9,3
				// FID_PGJE      ��ɽ��          Numric  16,2
				// FID_ZXSZ      ������ֵ          Numric  16,2
				// FID_BBJ       ������            Numric  9,3
				// FID_ZXJ       ���¼�            Numric  9,3
				// FID_GPSZ      ����ͨ��ֵ        Numric  16,2
				// FID_LXBJ      ��Ϣ����          Numric  16,9
				results_json["ret_data"].append( ret_data_json );
			}
			results = m_json_writer.write( results_json );
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool GetField::GetField_130005_630005( int32_t api_session, Request* request, std::string& results ) { // ��ѯ�ͻ�����ί��
	try {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			Json::Value results_json;
			int32_t ret_numb = Fix_GetCount( api_session ); // ҵ��ִ�г���ʱ ret_numb == 0
			FillHeadQuery( results_json, 130005, ret_numb, request ); // 130005 // FillHeadQuery
			int32_t i = 0; // ��ѯ��ҵ��ʧ����ȡ�������
			for( ; i < ret_numb; i++ ) {
				Json::Value ret_data_json;
				ret_data_json["otc_code"] = Fix_GetLong( api_session, 507, i ); // FID_CODE ������ Int
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["otc_info"] = Fix_GetItem( api_session, 508, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_MESSAGE ����˵�� Char 255
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["holder"] = Fix_GetItem( api_session, 571, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_GDH �ɶ��� Char 10
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["exchange"] = Fix_GetItem( api_session, 599, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_JYS ���������� Char 2
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["currency"] = Fix_GetItem( api_session, 511, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_BZ ���� Char 3
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["symbol"] = Fix_GetItem( api_session, 719, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_ZQDM ֤ȯ���� Char 6
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["security_type"] = Fix_GetItem( api_session, 720, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_ZQLB ֤ȯ��� Char 2
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["security_name"] = Fix_GetItem( api_session, 722, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_ZQMC ֤ȯ���� Char 8
				ret_data_json["order_id"] = Fix_GetLong( api_session, 681, i ); // FID_WTH ί�к� Int
				int64_t entr_type = Fix_GetLong( api_session, 1013, i ); // FID_DDLX �������� Int
				if( 0 == entr_type ) { // �޼�
					ret_data_json["entr_type"] = 1; // ������֤����֤��Ϊ 0 �޼�
				}
				else if( 1 == entr_type || 104 == entr_type ) { //�м�
					ret_data_json["entr_type"] = 2; // ������֤Ϊ 1 �мۣ�������֤Ϊ 104 �м�
				}
				else {
					return false;
				}
				ret_data_json["exch_side"] = Fix_GetLong( api_session, 683, i ); // FID_WTLB ί����� Int
				ret_data_json["price"] = Fix_GetDouble( api_session, 682, i ); // FID_WTJG ί�м۸� Nurmic 9,3
				ret_data_json["amount"] = Fix_GetLong( api_session, 684, i ); // FID_WTSL ί������ Int
				ret_data_json["fill_price"] = Fix_GetDouble( api_session, 525, i ); // FID_CJJG �ɽ��۸� Numric 9,3
				ret_data_json["fill_amount"] = Fix_GetLong( api_session, 528, i ); // FID_CJSL �ɽ����� Int
				ret_data_json["fill_money"] = Fix_GetDouble( api_session, 524, i ); // FID_CJJE �ɽ���� Numric 16,2
				ret_data_json["report_ret"] = Fix_GetLong( api_session, 753, i ); // FID_SBJG �걨��� Int
				ret_data_json["cxl_qty"] = Fix_GetLong( api_session, 886, i ); // FID_CDSL �������� Int
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["cxl_flag"] = Fix_GetItem( api_session, 755, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_CXBZ ������־ Char 1
				ret_data_json["frozen"] = Fix_GetDouble( api_session, 764, i ); // FID_DJZJ �����ʽ� Numric 16,2
				ret_data_json["settlement"] = Fix_GetDouble( api_session, 647, i ); // FID_QSZJ �����ʽ� Numric 16,2
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["brow_index"] = Fix_GetItem( api_session, 763, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_BROWINDEX ������ѯ����ֵ Char 128
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["report_time"] = Fix_GetItem( api_session, 751, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_SBSJ �걨ʱ�� Char 8
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["order_time"] = Fix_GetItem( api_session, 750, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_WTSJ ί��ʱ�� Char 8
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["fill_time"] = Fix_GetItem( api_session, 527, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_CJSJ �ɽ�ʱ�� Char 8
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["account"] = Fix_GetItem( api_session, 716, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_ZJZH �ʽ��˺� Char 20
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["message"] = Fix_GetItem( api_session, 830, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_JGSM ���˵�� Char 64
				// FID_BPGDH     ���̹ɶ���      Char  10
				// FID_WTFS      ί�з�ʽ        Int
				// FID_SBWTH     �걨ί�к�      Char  10
				// FID_WTPCH     ί�����κ�      Int
				// FID_ZJDJLSH   �ʽ𶳽���ˮ��  Int
				// FID_ZQDJLSH   ֤ȯ������ˮ��  Int
				// FID_SBRQ      ί���걨����    Int
				// FID_SBJLH     �걨��¼��      Int
				// FID_WTRQ      ί������        Int
				// FID_ADDR_IP   IP ��ַ         Char  16
				// FID_ADDR_MAC  MAC ��ַ        Char  12
				results_json["ret_data"].append( ret_data_json );
			}
			results = m_json_writer.write( results_json );
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool GetField::GetField_130006_630006( int32_t api_session, Request* request, std::string& results ) { // ��ѯ�ͻ����ճɽ�
	try {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			Json::Value results_json;
			int32_t ret_numb = Fix_GetCount( api_session ); // ҵ��ִ�г���ʱ ret_numb == 0
			FillHeadQuery( results_json, 130006, ret_numb, request ); // 130006 // FillHeadQuery
			int32_t i = 0; // ��ѯ��ҵ��ʧ����ȡ�������
			for( ; i < ret_numb; i++ ) {
				Json::Value ret_data_json;
				ret_data_json["otc_code"] = Fix_GetLong( api_session, 507, i ); // FID_CODE ������ Int
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["otc_info"] = Fix_GetItem( api_session, 508, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_MESSAGE ����˵�� Char 255
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["holder"] = Fix_GetItem( api_session, 571, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_GDH �ɶ��� Char 10
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["exchange"] = Fix_GetItem( api_session, 599, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_JYS ���������� Char 2
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["currency"] = Fix_GetItem( api_session, 511, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_BZ ���� Char 3
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["symbol"] = Fix_GetItem( api_session, 719, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_ZQDM ֤ȯ���� Char 6
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["security_type"] = Fix_GetItem( api_session, 720, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_ZQLB ֤ȯ��� Char 2
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["security_name"] = Fix_GetItem( api_session, 722, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_ZQMC ֤ȯ���� Char 8
				ret_data_json["order_id"] = Fix_GetLong( api_session, 681, i ); // FID_WTH ί�к� Int
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["trans_id"] = Fix_GetItem( api_session, 522, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_CJBH �ɽ���� Char 16
				ret_data_json["exch_side"] = Fix_GetLong( api_session, 683, i ); // FID_WTLB ί����� Int
				ret_data_json["fill_price"] = Fix_GetDouble( api_session, 525, i ); // FID_CJJG �ɽ��۸� Numric 9,3
				ret_data_json["fill_amount"] = Fix_GetLong( api_session, 528, i ); // FID_CJSL �ɽ����� Int
				ret_data_json["fill_money"] = Fix_GetDouble( api_session, 524, i ); // FID_CJJE �ɽ���� Numric 16,2
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["cxl_flag"] = Fix_GetItem( api_session, 755, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_CXBZ ������־ Char 1
				ret_data_json["settlement"] = Fix_GetDouble( api_session, 646, i ); // FID_QSJE ������ Numric 16,2
				ret_data_json["commission"] = Fix_GetDouble( api_session, 766, i ); // FID_S1 Ӷ�� Numric 16,2
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["brow_index"] = Fix_GetItem( api_session, 763, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_BROWINDEX ������ѯ����ֵ Char 128
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["account"] = Fix_GetItem( api_session, 716, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_ZJZH �ʽ��˺� Char 20
				// FID_HBXH   �ر����    Int
				// FID_LXBJ   ��Ϣ����    Numric  16,9
				// FID_SBWTH  �걨ί�к�  Char    10
				// FID_LX     ��Ϣ        Numric  16,2
				results_json["ret_data"].append( ret_data_json );
			}
			results = m_json_writer.write( results_json );
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool GetField::GetField_130008_601410( int32_t api_session, Request* request, std::string& results ) { // ��ѯETF������Ϣ
	try {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			Json::Value results_json;
			int32_t ret_numb = Fix_GetCount( api_session ); // ҵ��ִ�г���ʱ ret_numb == 0
			FillHeadQuery( results_json, 130008, ret_numb, request ); // 130008 // FillHeadQuery
			int32_t i = 0; // ��ѯ��ҵ��ʧ����ȡ�������
			for( ; i < ret_numb; i++ ) {
				Json::Value ret_data_json;
				ret_data_json["otc_code"] = Fix_GetLong( api_session, 507, i ); // FID_CODE ������ Int
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["otc_info"] = Fix_GetItem( api_session, 508, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_MESSAGE ����˵�� Char 255
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["fund_name"] = Fix_GetItem( api_session, 9130, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_JJMC ETF�������� Char 8
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["fund_id_1"] = Fix_GetItem( api_session, 1079, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_SGDM ETF������� Char 6
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["fund_id_2"] = Fix_GetItem( api_session, 9129, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_JJDM ETF������� Char 6
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["exchange"] = Fix_GetItem( api_session, 599, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_JYS ������ Char 2
				ret_data_json["count"] = Fix_GetLong( api_session, 788, i ); // FID_COUNT ��Ʊ��¼�� Int
				ret_data_json["status"] = Fix_GetLong( api_session, 9133, i ); // FID_SGSHZT ��������״̬ Int
				ret_data_json["pub_iopv"] = Fix_GetLong( api_session, 739, i ); // FID_LOGICAL �Ƿ񷢲�IOPV Int
				ret_data_json["unit"] = Fix_GetLong( api_session, 1061, i ); // FID_TZDW ��С�깺��ص�λ Int
				ret_data_json["cash_ratio"] = Fix_GetDouble( api_session, 9131, i ); // FID_XJTDBL ����ֽ�������� Numeric 7,5
				ret_data_json["cash_diff"] = Fix_GetDouble( api_session, 9136, i ); // FID_XJCE T���ֽ��� Numeric 11,2
				ret_data_json["iopv"] = Fix_GetDouble( api_session, 9138, i ); // FID_DWJZ T-1�ջ���λ��ֵ Numeric 8,4
				ret_data_json["trade_iopv"] = Fix_GetDouble( api_session, 9139, i ); // FID_SGSHDWJZ T-1�����굥λ��ֵ Numeric 12,2
				// FID_KSRQ    �����ֽ��Ϲ���ʼ����  Int
				// FID_RGDM    �Ϲ�����              Char 6
				// FID_RGQRDM  �Ϲ�ȷ�ϴ���          Char 6
				// FID_RQ      ETF�����Ϲ�����       Int
				// FID_XJDM    �ֽ����(�ʽ����)    Char 6
				// FID_JSRQ    �����ֽ��Ϲ���������  Int
				// FID_WJLJ    �ļ�·��              Char 255
				results_json["ret_data"].append( ret_data_json );
			}
			results = m_json_writer.write( results_json );
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool GetField::GetField_130009_601411( int32_t api_session, Request* request, std::string& results ) { // ��ѯETF�ɷֹ���Ϣ
	try {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			Json::Value results_json;
			int32_t ret_numb = Fix_GetCount( api_session ); // ҵ��ִ�г���ʱ ret_numb == 0
			FillHeadQuery( results_json, 130009, ret_numb, request ); // 130009 // FillHeadQuery
			int32_t i = 0; // ��ѯ��ҵ��ʧ����ȡ�������
			for( ; i < ret_numb; i++ ) {
				Json::Value ret_data_json;
				ret_data_json["otc_code"] = Fix_GetLong( api_session, 507, i ); // FID_CODE ������ Int
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["otc_info"] = Fix_GetItem( api_session, 508, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_MESSAGE ����˵�� Char 255
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["fund_name"] = Fix_GetItem( api_session, 9129, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_JJDM ETF������� Char 6
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["stock_code"] = Fix_GetItem( api_session, 719, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_ZQDM ETF�ɷݹɴ��� Char 6
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["stock_name"] = Fix_GetItem( api_session, 722, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_ZQMC ETF�ɷݹ����� Char 8
				ret_data_json["stock_qty"] = Fix_GetLong( api_session, 724, i ); // FID_ZQSL ETF�ɷݹ����� Int
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["exchange"] = Fix_GetItem( api_session, 599, m_field_value_short, FIELD_VALUE_SHORT, i ); // FID_JYS ������ Char 2
				ret_data_json["replace_flag"] = Fix_GetLong( api_session, 9134, i ); // FID_TDBZ �����־ Int
				ret_data_json["replace_money"] = Fix_GetDouble( api_session, 9137, i ); // FID_TDJE ������  Numeric 16,2
				ret_data_json["up_px_ratio"] = Fix_GetDouble( api_session, 1603, i ); // FID_YJBL ��۱���  Numeric 16,2
				results_json["ret_data"].append( ret_data_json );
			}
			results = m_json_writer.write( results_json );
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool GetField::GetField_190001_100065( int32_t api_session, Request* request, std::string& results ) { // �걨�ر�
	try {
		Json::Value results_json; // �ر�ͳһ�� NW_MSG_CODE_JSON ����
		results_json["ret_func"] = 190001;
		results_json["task_id"] = request->m_req_json["task_id"].asInt();
		results_json["order_id"] = Fix_GetLong( api_session, 681 ); // FID_WTH ί�к� Int
		results_json["exch_side"] = Fix_GetLong( api_session, 683 ); // FID_WTLB ί����� Int
		memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
		results_json["symbol"] = Fix_GetItem( api_session, 719, m_field_value_short, FIELD_VALUE_SHORT ); // FID_ZQDM ֤ȯ���� Char 6
		memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
		results_json["security_type"] = Fix_GetItem( api_session, 720, m_field_value_short, FIELD_VALUE_SHORT ); // FID_ZQLB ֤ȯ��� Char 2
		memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
		results_json["exchange"] = Fix_GetItem( api_session, 599, m_field_value_short, FIELD_VALUE_SHORT ); // FID_JYS ������ Char 2
		results_json["cxl_qty"] = Fix_GetLong( api_session, 886 ); // FID_CDSL �������� Int
		results_json["commit_ret"] = Fix_GetLong( api_session, 753 ); // FID_SBJG �걨��� Int
		memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
		results_json["commit_msg"] = Fix_GetItem( api_session, 830, m_field_value_short, FIELD_VALUE_SHORT ); // FID_JGSM ���˵�� Char 64
		// FID_QRBZ   ȷ�ϱ�־        Int
		// FID_GDH    �ɶ���          Char     10
		// FID_BZ     ����            Char     3
		// FID_CXBZ   ������־        Char     1
		// FID_DJZJ   �����ʽ�        Numeric  16,2
		results = m_json_writer_sb.write( results_json );
		return true;
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool GetField::GetField_190002_100064( int32_t api_session, Request* request, std::string& results ) { // �ɽ��ر�
	try {
		Json::Value results_json; // �ر�ͳһ�� NW_MSG_CODE_JSON ����
		results_json["ret_func"] = 190002;
		results_json["task_id"] = request->m_req_json["task_id"].asInt();
		results_json["order_id"] = Fix_GetLong( api_session, 681 ); // FID_WTH ί�к� Int
		results_json["exch_side"] = Fix_GetLong( api_session, 683 ); // FID_WTLB ί����� Int
		memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
		results_json["trans_id"] = Fix_GetItem( api_session, 522, m_field_value_short, FIELD_VALUE_SHORT ); // FID_CJBH �ɽ���� Char 16
		memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
		results_json["symbol"] = Fix_GetItem( api_session, 719, m_field_value_short, FIELD_VALUE_SHORT ); // FID_ZQDM ֤ȯ���� Char 6
		memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
		results_json["security_type"] = Fix_GetItem( api_session, 720, m_field_value_short, FIELD_VALUE_SHORT ); // FID_ZQLB ֤ȯ��� Char 2
		memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
		results_json["exchange"] = Fix_GetItem( api_session, 599, m_field_value_short, FIELD_VALUE_SHORT ); // FID_JYS ������ Char 2
		results_json["fill_qty"] = Fix_GetLong( api_session, 528 ); // FID_CJSL ���γɽ����� Int
		results_json["fill_price"] = Fix_GetDouble( api_session, 525 ); // FID_CJJG ���γɽ��۸� Numeric 9,3
		memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
		results_json["fill_time"] = Fix_GetItem( api_session, 527, m_field_value_short, FIELD_VALUE_SHORT ); // FID_CJSJ �ɽ�ʱ�� Char 8
		results_json["cxl_qty"] = Fix_GetLong( api_session, 886 ); // FID_CDSL �������� Int
		// FID_GDH    �ɶ���          Char     10
		// FID_BZ     ����            Char     3
		// FID_CXBZ   ������־        Char     1
		// FID_QSZJ   �����ʽ�        Numeric  16,2
		// FID_ZCJSL  ί���ܳɽ�����  Int
		// FID_ZCJJE  ί���ܳɽ����  Numeric  16,2
		// FID_CJJE   ���γɽ����    Numeric  16,2
		results = m_json_writer_cj.write( results_json );
		return true;
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool GetField::GetField_190003_100066( int32_t api_session, Request* request, std::string& results ) { // �����ر�
	try {
		Json::Value results_json; // �ر�ͳһ�� NW_MSG_CODE_JSON ����
		results_json["ret_func"] = 190003;
		results_json["task_id"] = request->m_req_json["task_id"].asInt();
		results_json["order_id"] = Fix_GetLong( api_session, 681 ); // FID_WTH ί�к� Int
		results_json["exch_side"] = Fix_GetLong( api_session, 683 ); // FID_WTLB ί����� Int
		memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
		results_json["symbol"] = Fix_GetItem( api_session, 719, m_field_value_short, FIELD_VALUE_SHORT ); // FID_ZQDM ֤ȯ���� Char 6
		memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
		results_json["security_type"] = Fix_GetItem( api_session, 720, m_field_value_short, FIELD_VALUE_SHORT ); // FID_ZQLB ֤ȯ��� Char 2
		memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
		results_json["exchange"] = Fix_GetItem( api_session, 599, m_field_value_short, FIELD_VALUE_SHORT ); // FID_JYS ������ Char 2
		results_json["cxl_qty"] = Fix_GetLong( api_session, 886 ); // FID_CDSL �������� Int
		results_json["total_fill_qty"] = Fix_GetLong( api_session, 528 ); // FID_CJSL �ɽ����� Int
		// FID_GDH    �ɶ���          Char     10
		// FID_BZ     ����            Char     3
		// FID_CXBZ   ������־        Char     1
		// FID_DJZJ   �����ʽ�        Numeric  16,2
		results = m_json_writer_cd.write( results_json );
		return true;
	}
	catch( ... ) {
		return false;
	}
	return false;
}
