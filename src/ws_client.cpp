/*
 * wb_client.cpp
 *
 *  Created on: Sep 13, 2017
 *      Author: root
 */

#include "main.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <mutex>
#include <condition_variable>

#include <uWS/uWS.h>

#include <log4z/log4z.h>

#include "ws_command.h"
#include "ws_chat_protocol.pb.h"
#include "TransportData.h"
#include "ClientBorkerMessage.h"
#include "UserSession.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

using namespace rapidjson;

struct THREAD_SHARED_DATA {
		uWS::WebSocket<uWS::CLIENT> *ws;
		int connected;
};

static std::mutex shared_m;
static std::condition_variable shared_cv; // 全局条件变量
static THREAD_SHARED_DATA shared_data = { NULL, 0 };

void __run_server_binary__(void* self) {

	THREAD_SHARED_DATA* csws = (THREAD_SHARED_DATA*) self;
	if (csws == nullptr) {
		LOGD("csws == nullptr");
		return;
	}

	{
		std::unique_lock<std::mutex> l(shared_m);
		if (csws->connected != 1) {
			shared_cv.wait(l);
		}
	}

	UserSession<uWS::CLIENT>* session =
			(UserSession<uWS::CLIENT>*) csws->ws->getUserData();

	while (1) {
		ws_chat_protocol::ws_msg_chat_request chat;
		chat.mutable_head()->set_time(time(NULL));
		chat.mutable_head()->set_rid(session->rid);
		chat.mutable_head()->set_from(session->uid);
		chat.mutable_head()->set_to(10000);
		chat.set_msg("hello");

		int len = chat.ByteSize();
		char* buff = new char[len];
		bool r = chat.SerializeToArray(buff, len);
		if (r) {
			TransportData trans(TransportData::TYPE::encode);
			int c = trans.build(MESSAGE_CMD_CHAT, buff, len);
			if (c == 0) {
				{
					std::unique_lock<std::mutex> l(shared_m);
					csws->ws->send(trans.getBuffer(), trans.getBufferLength(),
							uWS::OpCode::BINARY);
				}
			}
		}
		delete buff;

		std::this_thread::sleep_for(std::chrono::seconds(1));
//		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	};
}

void __run_server_text__(void* self) {

	THREAD_SHARED_DATA* csws = (THREAD_SHARED_DATA*) self;
	if (csws == nullptr) {
		LOGD("csws == nullptr");
		return;
	}

	{
		std::unique_lock<std::mutex> l(shared_m);
		if (csws->connected != 1) {
			shared_cv.wait(l);
		}
	}

	UserSession<uWS::CLIENT>* session =
			(UserSession<uWS::CLIENT>*) csws->ws->getUserData();

	while (1) {
		Document d;
		d.SetObject();
		Document::AllocatorType& allocator = d.GetAllocator();
		d.AddMember("rid", session->rid, allocator);
		d.AddMember("uid", session->uid, allocator);
		d.AddMember("name", session->name.c_str(), allocator);
		StringBuffer buffer;
		Writer<StringBuffer> writer(buffer);
		d.Accept(writer);
		std::string msg = buffer.GetString();
		{
			std::unique_lock<std::mutex> l(shared_m);
			csws->ws->send(msg.c_str());
		}

		std::this_thread::sleep_for(std::chrono::seconds(1));
//		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	};
}

class MsgHandler {
	public:
		MsgHandler(uWS::WebSocket<uWS::CLIENT> *ws = nullptr) {
			this->ws = ws;
		}

		virtual ~MsgHandler() {

		}

	public:
		void onMessage(char *message, size_t length, bool is_binary) {

		}

	private:
		uWS::WebSocket<uWS::CLIENT> *ws;
};

class WSClient {
	public:
		WSClient(std::string host = "localhost", std::string port = "3000") {
			this->_pings = 0;
			this->_session = new UserSession<uWS::CLIENT>(nullptr);
			this->_host = host;
			this->_port = port;
		}

		virtual ~WSClient() {
			delete this->_session;
		}

	public:

		int init() {
			this->_h.onPing(
					[this](uWS::WebSocket<uWS::CLIENT> *ws, char *message, size_t length) {
						LOGFMTD("PING: %d",_pings++);
					});

			this->_h.onConnection(
					[this](uWS::WebSocket<uWS::CLIENT> *ws, uWS::HttpRequest req) {
						LOGD("onConnection");
						ws->setUserData(_session);
						{
							std::unique_lock<std::mutex> l(shared_m);
							shared_data.ws = ws;
							shared_data.connected = 1;
							shared_cv.notify_all();
						}
					});

			this->_h.onMessage(
					[this](uWS::WebSocket<uWS::CLIENT> *ws, char *message, size_t length, uWS::OpCode opCode) {
						if (opCode == uWS::OpCode::BINARY) {
							TransportData trans(TransportData::TYPE::decode);
							int r = trans.parse(message, length);
							if (r == 0) {
								int cmd = trans.getCmd();
								_borker.dispatch(ws, cmd, trans.getBuffer(), trans.getBufferLength(), true);
							}
							else {
								LOGD("protocol package error.");
							}
						}
						else if (opCode == uWS::OpCode::TEXT) {
							LOGFMTD("received: %s", std::string(message, length).c_str());
						}
						else {

						}
					});

			this->_h.onDisconnection(
					[this](uWS::WebSocket<uWS::CLIENT> *ws, int code, char * message, size_t length) {
						LOGD("onDisconnection");
						switch (code) {
							case 10001:

							break;
							case 10002:

							break;
							default:
							break;
						}
					});

			this->_h.onError(
					[this](void* user) {
						switch ((long) user) {
							case 1:
							LOGD("Client emitted error on invalid URI");
							break;
							case 2:
							LOGD("Client emitted error on resolve failure");
							break;
							case 3:
							LOGD("Client emitted error on connection timeout (non-SSL)");
							break;
							case 5:
							LOGD("Client emitted error on connection timeout (SSL)");
							break;
							case 6:
							LOGD("Client emitted error on HTTP response without upgrade (non-SSL)");
							break;
							case 7:
							LOGD("Client emitted error on HTTP response without upgrade (SSL)");
							break;
							default:
							LOGFMTD("FAILURE: %d should not emit error!", user);
						}
					});

			char addr[512];
			unsigned int rid = rand() % 5 + 5;
			unsigned int uid = rand() % 10000 + 10000;
			std::string name = std::string("test_") + std::to_string(uid);
			std::string upsd = std::string("1234567890") + std::to_string(uid);
			std::string extra = "1234567890";

			sprintf(addr,
					"ws://%s:%s/ws?name=%s&upsd=%s&rid=%d&uid=%d&extra=%s",
					this->_host.c_str(), this->_port.c_str(), name.c_str(),
					upsd.c_str(), rid, uid, extra.c_str());

			this->_session->name = name;
			this->_session->upsd = upsd;
			this->_session->extra = extra;
			this->_session->rid = rid;
			this->_session->uid = uid;

			this->_h.connect(addr);

			return 0;
		}

	public:
		void start() {
			this->_h.run();
		}

	private:
		uWS::Hub _h;
		MsgHandler _handler;
		ClientBorkerMessage _borker;
		UserSession<uWS::CLIENT>* _session;
		int _pings;
		std::string _host;
		std::string _port;
};

int test_wb_client(int argc, char** argv) {

	srand(time(nullptr));

	WSClient cli("localhost", "3000");
	if (cli.init() == 0) {
//		{
//			std::thread t(__run_server_binary__, &shared_data);
//			t.detach();
//		}

		{
			std::thread t(__run_server_text__, &shared_data);
			t.detach();
		}

		cli.start();
	}

	return 0;
}

