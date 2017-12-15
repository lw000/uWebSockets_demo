#include "main.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <atomic>
#include <string>
#include <algorithm>

#include <uWS/uWS.h>

#include "utils.h"

#include "log4z.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "urlparser.h"

#ifdef __cplusplus
}
#endif

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include "ws_chat_protocol.pb.h"

#include "TransportData.h"
#include "ServerBorkerMessage.h"
#include "UserSession.h"

using namespace rapidjson;

static std::stringstream __g_index_html;

class WSServer {
	public:
		WSServer() {

		}

		virtual ~WSServer() {

		}

	public:
		int init() {
			this->_index_html << std::ifstream("index.html").rdbuf();
			if (!this->_index_html.str().length()) {
				LOGD("Failed to load index.html");
				return -1;
			}

			return 0;
		}

	private:
		uWS::Hub h;
		ServerBorkerMessage _borker;
		std::stringstream _index_html;
};

int test_wb_server(int argc, char** argv) {
	uWS::Hub h;
	ServerBorkerMessage borker;

	__g_index_html << std::ifstream("./index.html").rdbuf();
	if (!__g_index_html.str().length()) {
		LOGD("Failed to load index.html");
		return -1;
	}

	h.onPong(
			[](uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length) {
				UserSession<uWS::SERVER>* session = (UserSession<uWS::SERVER>*)ws->getUserData();
				if (session != nullptr) {
					session->pings++;
					LOGFMTD("client[%d] pongs[%d]", session->uid, session->pings);

					//					if (c->pings >= c->pongs) {
					//						printf("close client[%d] \n", c->uid);
					//						ws->close();
					//					}
				}
				else {
					LOGD("session == nullptr");
				}
			});

	h.onConnection(
			[&borker](uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest req) {
				std::string url = req.getUrl().toString();
				LOGFMTD( "url: %s", url.c_str());

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

				LOGFMTD( "name: %s, upsd: %s, rid: %s, uid: %s, extra: %s", t_name.c_str(), t_upsd.c_str(),t_rid.c_str(),t_uid.c_str(),t_extra.c_str());

				unsigned int rid = atoi(t_rid.c_str());
				unsigned int uid = atoi(t_uid.c_str());

				UserSession<uWS::SERVER>* session = new UserSession<uWS::SERVER>(ws);
				session->rid = rid;
				session->uid = uid;
				ws->setUserData(session);

				borker.session_mgr.addSession(session);

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
			{
				Document d;
				d.SetObject();
				Document::AllocatorType& allocator = d.GetAllocator();
				d.AddMember("rid", session->rid, allocator);
				d.AddMember("uid", session->uid, allocator);
				d.AddMember("what", "connected success", allocator);
				StringBuffer buffer;
				Writer<StringBuffer> writer(buffer);
				d.Accept(writer);
				std::string msg = buffer.GetString();
				ws->send(msg.c_str(), msg.length(), uWS::OpCode::TEXT);
			}
#endif
		});

	h.onDisconnection(
			[&borker](uWS::WebSocket<uWS::SERVER> *ws, int code, char * message, size_t length) {
				UserSession<uWS::SERVER>* session = (UserSession<uWS::SERVER>*)ws->getUserData();
				if (session != nullptr) {
					LOGFMTD("onDisconnection. [%d]", session->uid);
					borker.session_mgr.removeSession(session);
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

//					LOGFMTD("rid: %d, uid: %d, received:%s", session->rid, session->uid, std::string(message, length).c_str());

					if (session->rid == 0) {
						h.getDefaultGroup<uWS::SERVER>().broadcast(message, length, uWS::OpCode::TEXT);
					}
					else {
						borker.session_mgr.broadcast(session->rid, [message, length](UserSession<uWS::SERVER>* s) {
									s->getWS()->send(message, length, uWS::OpCode::TEXT);
								});
					}
				}
				else {
					std::string msg("error.");
					ws->send(msg.c_str(), msg.length(), uWS::OpCode::TEXT);
				}
			});

	h.onHttpRequest(
			[&borker](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t length, size_t remainingBytes) {
				std::string url = req.getUrl().toString();
				LOGFMTD("url: %s", url.c_str());
				{
					std::string s1 = req.getHeader("accept-encoding").toString();
					std::string s2 = req.getHeader("accept-language").toString();
					std::string s3 = req.getHeader("cache-control").toString();
					std::string s4 = req.getHeader("connection").toString();
					std::string s5 = req.getHeader("host").toString();
					std::string s6 = req.getHeader("origin").toString();
					std::string s7 = req.getHeader("pragma").toString();
					std::string s8 = req.getHeader("sec-websocket-extensions").toString();
					std::string s9 = req.getHeader("sec-websocket-key").toString();
					std::string s10 = req.getHeader("sec-webSocket-version").toString();
					std::string s11 = req.getHeader("upgrade").toString();
					std::string s12 = req.getHeader("user-agent").toString();

					LOGFMTD("accept-encoding: %s", s1.c_str());
					LOGFMTD("accept-language: %s", s2.c_str());
					LOGFMTD("cache-control: %s", s3.c_str());
					LOGFMTD("connection: %s", s4.c_str());
					LOGFMTD("host: %s", s5.c_str());
					LOGFMTD("origin: %s", s6.c_str());
					LOGFMTD("pragma: %s", s7.c_str());
					LOGFMTD("sec-websocket-extensions: %s", s8.c_str());
					LOGFMTD("sec-websocket-key: %s", s9.c_str());
					LOGFMTD("sec-webSocket-version: %s", s10.c_str());
					LOGFMTD("upgrade: %s", s11.c_str());
					LOGFMTD("user-agent: %s", s12.c_str());
				}

				if (req.getMethod() == uWS::HttpMethod::METHOD_GET) {

				}
				else if (req.getMethod() == uWS::HttpMethod::METHOD_POST) {
					if (data != nullptr) {
						LOGFMTD("length:%lld, data:%s", length, data);
					}
					else {

					}
				}

				std::string path;
				std::string query;

				URL purl;
				int c = parseURL(url.c_str(), &purl);
				if (c == 0) {
					path = readURLField(url.c_str(), purl.path);
					query = readURLField(url.c_str(), purl.query);
				}

				std::transform(path.begin(), path.end(), path.begin(), std::towlower);
				std::transform(query.begin(), query.end(), query.begin(), std::towlower);

				if (!path.empty()) {
					if (path.compare("/") == 0) {
						res->end(__g_index_html.str().data(), __g_index_html.str().length());
					}
					else if (path.compare("room_user_count") == 0) {

					}
					else {

					}
				}
			});

	h.getDefaultGroup<uWS::SERVER>().startAutoPing(30000);

	if (h.listen(3000)) {
		LOGD("Success to listen 3000");

		h.run();

	}
	else {
		LOGD("Failed to listen 3000");
	}

	return 0;
}
