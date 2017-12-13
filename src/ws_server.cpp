#include "main.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <atomic>
#include <string>

#include <uWS/uWS.h>

#include "utils.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include "log4z.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "urlparser.h"

#ifdef __cplusplus
}
#endif

#include "ws_chat_protocol.pb.h"
#include "ws_command.h"

#include "TransportData.h"
#include "ServerBorkerMessage.h"
#include "UserSession.h"

std::stringstream __g_index_html;

int test_wb_server(int argc, char** argv) {

	uWS::Hub h;
	ServerBorkerMessage borker;

	__g_index_html << std::ifstream("index.html").rdbuf();
	if (!__g_index_html.str().length()) {
		LOGD("Failed to load index.html");
		return -1;
	}

	h.onPong(
			[](uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length) {
				UserSession<uWS::SERVER>* session = (UserSession<uWS::SERVER>*)ws->getUserData();
				session->pings++;
				LOGFMTD("client[%d] pongs[%d]\n", session->uid, session->pings);

				//					if (c->pings >= c->pongs) {
				//						printf("close client[%d] \n", c->uid);
				//						ws->close();
				//					}
			});

	h.onConnection(
			[](uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest req) {
				std::string url = req.getUrl().toString();
				LOGFMTD( "url: %s\n", url.c_str());

				std::string path;
				URL purl;

				int c = parseURL(url.c_str(), &purl);
				if (c == 0) {
					path = readURLField(url.c_str(), purl.path);
				}

				if (path.compare("/ws") != 0) {
					ws->close(10002);
					return;
				}

				std::string query;
				if (c == 0) {
					query = readURLField(url.c_str(), purl.query);
				}

				if (query.empty()) {
					ws->close(10002);
					return;
				}

				std::string t_name;
				std::string t_upsd;
				std::string t_rid;
				std::string t_uid;
				std::string t_extra;

				KVQueryUrlArgsValue queryArgs;
				int c1 = queryArgs.parse(query.c_str());
				if (c1 != 0) {
					ws->close(10002);
					return;
				}

				t_name = queryArgs.find("name");
				t_upsd = queryArgs.find("upsd");
				t_rid = queryArgs.find("rid");
				t_uid = queryArgs.find("uid");
				t_extra = queryArgs.find("extra");
				if (t_name.empty()) {
					ws->close(10001, "username error.", strlen("username error."));
					return;
				}

				if (t_upsd.empty()) {
					ws->close(10001, "password error.", strlen("password error."));
					return;
				}

				if (t_rid.empty()) {
					ws->close(10001, "rid error.", strlen("rid error."));
					return;
				}

				if (t_uid.empty()) {
					ws->close(10001, "uid error.", strlen("uid error."));
					return;
				}

				if (t_extra.empty()) {
					ws->close(10001, "extra error.", strlen("extra error."));
					return;
				}

				LOGFMTD( "name: %s, upsd: %s, rid: %s, uid: %s, extra: %s\n", t_name.c_str(), t_upsd.c_str(),t_rid.c_str(),t_uid.c_str(),t_extra.c_str());

				unsigned int rid = atoi(t_rid.c_str());
				unsigned int uid = atoi(t_uid.c_str());

				UserSession<uWS::SERVER>* session = new UserSession<uWS::SERVER>();
				session->ws =ws;
				session->rid = rid;
				session->uid = uid;
				ws->setUserData(session);

#if 0
			{
				ws_protocol::msg_connected connected;
				connected.set_time(time(nullptr));
				connected.set_uid(session->uid);
				connected.set_msg("connected");
				int len = connected.ByteSize();
				char* buff = new char[len];
				bool r = connected.SerializeToArray(buff, len);
				if (r) {
					TransportData trans(TransportData::TYPE::encode);
					int c = trans.build(MESSAGE_CMD_CONNECTED, buff, len);
					if (c == 0) {
						ws->send(trans.getBuffer(), trans.getBufferLength(), uWS::OpCode::BINARY);
					}
				}
				delete buff;
			}
#else
			std::string msg("connected success.");
			ws->send(msg.c_str(), msg.length(), uWS::OpCode::TEXT);
#endif
		});

	h.onDisconnection(
			[](uWS::WebSocket<uWS::SERVER> *ws, int code, char * message, size_t length) {
				UserSession<uWS::SERVER>* session = (UserSession<uWS::SERVER>*)ws->getUserData();
				if (session != nullptr) {
					LOGFMTD("onDisconnection client [%d]", session->uid);
					delete session;
				}
			});

	h.onMessage(
			[&borker, &h](uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode) {
				UserSession<uWS::SERVER>* session = (UserSession<uWS::SERVER>*)ws->getUserData();
				if (opCode == uWS::OpCode::BINARY) {
					TransportData trans(TransportData::TYPE::decode);
					int r = trans.parse(message, length);
					if (r == 0) {
						int cmd = trans.getCmd();
						borker.dispatch(ws, cmd, trans.getBuffer(), trans.getBufferLength(), true);
					}
					else {
						ws_chat_protocol::ws_msg_error merror;
						merror.set_time(time(NULL));
						merror.set_msg("protocol is error!");
						int len = merror.ByteSize();
						char* buff = new char[len];
						bool r = merror.SerializeToArray(buff, len);
						if (r) {
							TransportData trans(TransportData::TYPE::encode);
							int c = trans.build(MESSAGE_CMD_ERROR, buff, len);
							if (c == 0) {
								ws->send(trans.getBuffer(), trans.getBufferLength(), opCode);
							}
						}
						delete buff;
					}
				}
				else if (opCode == uWS::OpCode::TEXT) {

					LOGFMTD("rid: %d, uid: %d, received:%s", session->rid, session->uid, std::string(message, length).c_str());

					h.getDefaultGroup<uWS::SERVER>().broadcast(message, length, uWS::OpCode::TEXT);

				}
				else {
					std::string msg("error.");
					ws->send(msg.c_str(), msg.length(), uWS::OpCode::TEXT);
				}
			});

	h.onHttpRequest(
			[](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t length, size_t remainingBytes) {
				if (req.getMethod() == uWS::HttpMethod::METHOD_GET) {

				}
				else if (req.getMethod() == uWS::HttpMethod::METHOD_POST) {
					if (data != NULL) {
						std::cout << "data: " << data << " length: " << length << std::endl;
					}
					else {

					}
				}

				std::string url = req.getUrl().toString();
				LOGFMTD( "url: %s\n", url.c_str());

				std::string path;
				std::string query;

				URL purl;
				int c = parseURL(url.c_str(), &purl);
				if (c == 0) {
					path = readURLField(url.c_str(), purl.path);
					query = readURLField(url.c_str(), purl.query);
				}

				if (!path.empty()) {
					if (path.compare("/") == 0) {
						res->end(__g_index_html.str().data(), __g_index_html.str().length());
					}
					else if (path.compare("room_user_count") == 0) {

					}
					else if (path.compare("mul") == 0) {

					}
					else {

					}
				}

			});

	h.getDefaultGroup<uWS::SERVER>().startAutoPing(10000);

	if (h.listen(3000)) {
		LOGD("Success to listen 3000");

		h.run();

	}
	else {
		LOGD("Failed to listen 3000");
	}

	return 0;
}
