/*
 * ServerBorkerMessage.cpp
 *
 *  Created on: Sep 20, 2017
 *      Author: root
 */

#include "ServerBorkerMessage.h"

#include "TransportData.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include <log4z/log4z.h>

ServerBorkerMessage::ServerBorkerMessage() {
	this->setOnMessageCallback(
			CC_WS_MSG_CALLBACK_4(ServerBorkerMessage::onMessage, this));
}

ServerBorkerMessage::~ServerBorkerMessage() {
}

int ServerBorkerMessage::onMessage(uWS::WebSocket<uWS::SERVER> *ws, int cmd,
		char *message, size_t length) {
	switch (cmd) {
		case MESSAGE_CMD_CONNECTED:
			onConnected(ws, message, length);
			break;
		case MESSAGE_CMD_CHAT_BROADCAST:
			onBroadCastMessage(ws, message, length);
			break;
		case MESSAGE_CMD_CHAT:
			onChat(ws, message, length);
			break;
		default:
			LOGFMTD("cmd: %d", cmd);
			break;
	}

	return 0;
}

bool ServerBorkerMessage::onConnected(uWS::WebSocket<uWS::SERVER> *ws,
		char *message, size_t length) {
	ws_chat_protocol::ws_msg_connected con;
	bool r = con.ParseFromArray(message, length);
	if (r) {
		LOGFMTD("%ld,%s", con.head().time(), con.msg().c_str());
	}
	return false;
}

bool ServerBorkerMessage::onChat(uWS::WebSocket<uWS::SERVER> *ws, char *message,
		size_t length) {
	ws_chat_protocol::ws_msg_chat_request chat;
	bool r = chat.ParseFromArray(message, length);
	if (r) {
		LOGFMTD("time: %ld, from: %d, to: %d, msg: %s", chat.head().time(),
				chat.head().from(), chat.head().to(), chat.msg().c_str());

		ws_chat_protocol::ws_msg_chat_replay replay;
		replay.set_code(100);
		replay.set_time(time(NULL));
		replay.set_msg("world");
		replay.set_from(chat.head().from());
		int len = replay.ByteSize();
		char* buff = new char[len];
		bool r = replay.SerializeToArray(buff, len);
		if (r) {
			TransportData trans(TransportData::TYPE::encode);
			int c = trans.build(MESSAGE_CMD_CHAT_REPLAY, buff, len);
			if (c == 0) {
				ws->send(trans.getBuffer(), trans.getBufferLength(),
						uWS::OpCode::BINARY);
			}
		}
		delete buff;
	}
	return false;
}

bool ServerBorkerMessage::onBroadCastMessage(uWS::WebSocket<uWS::SERVER> *ws,
		char *message, size_t length) {

	return false;
}
