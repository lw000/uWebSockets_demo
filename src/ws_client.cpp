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

struct THREAD_SHARED_WS {
	uWS::WebSocket<uWS::CLIENT> *ws;
	int connected;
};

static THREAD_SHARED_WS shared_ws = { NULL, 0 };
std::mutex shared_m;
std::condition_variable shared_cv; // 全局条件变量.

void __run_server0__(void* self) {

	THREAD_SHARED_WS* csws = (THREAD_SHARED_WS*) self;

	{
		std::unique_lock<std::mutex> l(shared_m);
		if (csws->connected != 1) {
			shared_cv.wait(l);
		}
	}

	UserSession<uWS::CLIENT>* session =
			(UserSession<uWS::CLIENT>*) csws->ws->getUserData();

	while (1) {
#if 1
		ws_chat_protocol::ws_msg_chat_request chat;
		chat.set_time(time(NULL));
		chat.set_from(session->uid);
		chat.set_to(10000);
		chat.set_msg("hello_from_9999_to_10000");

		int len = chat.ByteSize();
		char* buff = new char[len];
		bool r = chat.SerializeToArray(buff, len);
		if (r) {
			TransportData trans(TransportData::TYPE::build);
			int c = trans.buildData(MESSAGE_CMD_CHAT, buff, len);
			if (c == 0) {
				{
					std::unique_lock<std::mutex> l(shared_m);
					csws->ws->send(trans.getBuffer(), trans.getBufferLength(),
							uWS::OpCode::BINARY);
				}
			}
		}
		delete buff;
#else
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
		csws->ws->send(msg.c_str());
#endif

//		std::this_thread::sleep_for(std::chrono::seconds(1));
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	};
}

void __run_server1__(void* self) {

	THREAD_SHARED_WS* csws = (THREAD_SHARED_WS*) self;

	{
		std::unique_lock<std::mutex> l(shared_m);
		if (csws->connected != 1) {
			shared_cv.wait(l);
		}
	}

	UserSession<uWS::CLIENT>* session =
			(UserSession<uWS::CLIENT>*) csws->ws->getUserData();

	while (1) {
#if 1
		ws_chat_protocol::ws_msg_chat_request chat;
		chat.set_time(time(NULL));
		chat.set_from(9999);
		chat.set_to(10001);
		chat.set_msg("hello_from_9999_to_10001");

		int len = chat.ByteSize();
		char* buff = new char[len];
		bool r = chat.SerializeToArray(buff, len);
		if (r) {
			TransportData trans(TransportData::TYPE::build);
			int c = trans.buildData(MESSAGE_CMD_CHAT, buff, len);
			if (c == 0) {
				{
					std::unique_lock<std::mutex> l(shared_m);
					csws->ws->send(trans.getBuffer(), trans.getBufferLength(),
							uWS::OpCode::BINARY);
				}

			}
		}
		delete buff;
#else
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
#endif

//		std::this_thread::sleep_for(std::chrono::seconds(1));
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	};
}

int test_wb_client(int argc, char** argv) {
	int pings = 0;
	uWS::Hub h;
	ClientBorkerMessage borker;
	UserSession<uWS::CLIENT>* session = new UserSession<uWS::CLIENT>();

	h.onPing(
			[&pings](uWS::WebSocket<uWS::CLIENT> *ws, char *message, size_t length) {
				std::cout << "PING: " << pings++ << std::endl;
			});

	h.onConnection(
			[session](uWS::WebSocket<uWS::CLIENT> *ws, uWS::HttpRequest req) {
				std::cout << "onConnection" << std::endl;
				ws->setUserData(session);
				{
					std::unique_lock<std::mutex> l(shared_m);
					shared_ws.ws = ws;
					shared_ws.connected = 1;
					shared_cv.notify_all();
				}
			});

	h.onMessage(
			[&borker](uWS::WebSocket<uWS::CLIENT> *ws, char *message, size_t length, uWS::OpCode opCode) {
				if (opCode == uWS::OpCode::BINARY) {
					TransportData trans(TransportData::TYPE::parse);
					int r = trans.parseData(message, length);
					if (r == 0) {
						int cmd = trans.getCmd();
						borker.dispatch(ws, cmd, trans.getBuffer(), trans.getBufferLength(), true);
					} else {
						std::cout << "protocol package error." << std::endl;
					}
				} else if (opCode == uWS::OpCode::TEXT) {
					std::cout <<std::string(message, length) << std::endl;
				} else {

				}
			});

	h.onDisconnection(
			[](uWS::WebSocket<uWS::CLIENT> *ws, int code, char * message, size_t length) {
				std::cout << "onDisconnection" << std::endl;
				if (code == 10001) {

				} else if (code == 10002) {

				} else {

				}
			});

	h.onError(
			[](void* user) {
				switch ((long) user) {
					case 1:
					std::cout << "Client emitted error on invalid URI" << std::endl;
					break;
					case 2:
					std::cout << "Client emitted error on resolve failure" << std::endl;
					break;
					case 3:
					std::cout << "Client emitted error on connection timeout (non-SSL)" << std::endl;
					break;
					case 5:
					std::cout << "Client emitted error on connection timeout (SSL)" << std::endl;
					break;
					case 6:
					std::cout << "Client emitted error on HTTP response without upgrade (non-SSL)" << std::endl;
					break;
					case 7:
					std::cout << "Client emitted error on HTTP response without upgrade (SSL)" << std::endl;
					break;
					default:
					std::cout << "FAILURE: " << user << " should not emit error!" << std::endl;
				}
			});

	srand(time(nullptr));

	char addr[512];
	std::string host = "localhost";
	std::string port = "3000";
	unsigned int rid = rand() % 5 + 5;
	unsigned int uid = rand() % 10000 + 10000;
	std::string name = std::string("liwei_") + std::to_string(uid);
	std::string upsd = std::string("1234567890") + std::to_string(uid);
	std::string extra = "1234567890";

	sprintf(addr, "ws://%s:%s/ws?name=%s&upsd=%s&rid=%d&uid=%d&extra=%s",
			host.c_str(), port.c_str(), name.c_str(), upsd.c_str(), rid, uid,
			extra.c_str());

	session->rid = rid;
	session->uid = uid;

	h.connect(addr);

	{
		std::thread t(__run_server0__, &shared_ws);
			t.detach();
	}

	{
		std::thread t(__run_server1__, &shared_ws);
		t.detach();
	}

	h.run();

	return 0;
}

